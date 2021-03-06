// This file is part of OpenPanel - The Open Source Control Panel
// OpenPanel is free software: you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation, using version 3 of the License.
//
// Please note that use of the OpenPanel trademark may be subject to additional 
// restrictions. For more information, please visit the Legal Information 
// section of the OpenPanel website on http://www.openpanel.com/

#include <grace/exception.h>

$exception (mysqlSocketInitException, "Error initializing mysql socket");
$exception (mysqlConnectException, "Error connecting to mysqld");

#include "mysqlmodule.h"

// ==========================================================================
// CONSTRUCTOR mysqlSocket
// ==========================================================================
mysqlSocket::mysqlSocket (const string &user, const string &pass)
{
	value ini;
	string hostname = "localhost";
	if (fs.exists ("/etc/openpanel/mod-mysql.conf"))
	{
		ini.loadini ("/etc/openpanel/mod-mysql.conf");
		hostname = ini["hostname"];
	}
	msock = mysql_init (NULL);
	if (! msock) throw (mysqlSocketInitException());
	
	if (! mysql_real_connect (msock, hostname.str(), user.str(),
							  pass.str(), NULL, 3306, NULL, 0))
	{
		throw (mysqlConnectException());
	}
}
	
// ==========================================================================
// DESTRUCTOR mysqlSocket
// ==========================================================================
mysqlSocket::~mysqlSocket (void)
{
	if (msock) mysql_close (msock);
}
	
// ==========================================================================
// METHOD mysqlSocket::query
// ==========================================================================
bool mysqlSocket::query (const string &q)
{
	if (mysql_query (msock, q.str()))
	{
		return false;
	}
	return true;
}

// ==========================================================================
// CONSTRUCTOR mysqlControl
// ==========================================================================
mysqlControl::mysqlControl (const string &_user, const string &_pass)
	: user (_user), pass (_pass), sock (user, pass)
{
	permlist =
		$("Select_priv","N") ->
		$("Insert_priv","N") ->
		$("Update_priv","N") ->
		$("Delete_priv","N") ->
		$("Create_priv","N") ->
		$("Drop_priv","N") ->
		$("Grant_priv","N") ->
		$("References_priv","N") ->
		$("Index_priv","N") ->
		$("Alter_priv","N") ->
		$("Create_tmp_table_priv","N") ->
		$("Lock_tables_priv","N");
}

// ==========================================================================
// DESTRUCTOR mysqlControl
// ==========================================================================
mysqlControl::~mysqlControl (void)
{
}

// ==========================================================================
// METHOD mysqlControl::permsRead
// ==========================================================================
value *mysqlControl::permsRead (void)
{
	return $("Select_priv","Y");
}

value *mysqlControl::permsReadWrite (void)
{
	return $("Select_priv","Y") ->
		   $("Insert_priv","Y") ->
		   $("Update_priv","Y") ->
		   $("Delete_priv","Y");
}

// ==========================================================================
// METHOD mysqlControl::permsAdmin
// ==========================================================================
value *mysqlControl::permsAdmin (void)
{
	return $("Select_priv","Y") ->
		   $("Insert_priv","Y") ->
		   $("Update_priv","Y") ->
		   $("Delete_priv","Y") ->
		   $("Create_priv","Y") ->
		   $("Drop_priv","Y") ->
		   $("Grant_priv","Y") ->
		   $("References_priv","Y") ->
		   $("Index_priv","Y") ->
		   $("Alter_priv","Y") ->
		   $("Create_tmp_table_priv","Y") ->
		   $("Lock_tables_priv","Y");
}

// ==========================================================================
// METHOD mysqlControl::escapeUnderscore
// ==========================================================================
string *mysqlControl::escapeUnderscore (const string &from)
{
	returnclass (string) res retain;
	res = from;
	res.replace ($("_","\\_"));
	return &res;
}

// ==========================================================================
// METHOD mysqlControl::createDatabase
// ==========================================================================
bool mysqlControl::createDatabase (const string &dbname)
{
	return sock.query ("CREATE DATABASE `%S`" %format (dbname));
}

// ==========================================================================
// METHOD mysqlControl::dropDatabase
// ==========================================================================
bool mysqlControl::dropDatabase (const string &dbname)
{
	sock.query ("DELETE FROM mysql.user WHERE User in (SELECT User FROM "
				"mysql.db WHERE Db=%M) AND User not in (SELECT User FROM "
				"mysql.db WHERE Db!=%M)" %format (dbname,dbname));
	sock.query ("DELETE FROM mysql.db where Db=%M" %format (dbname));
	sock.query ("FLUSH PRIVILEGES");
	return sock.query ("DROP DATABASE `%S`" %format (dbname));
}

// ==========================================================================
// METHOD mysqlControl::addUser
// ==========================================================================
bool mysqlControl::addUser (const string &_dbname, const string &username,
							const string &passwd, const value &perms)
{
	string dbname = escapeUnderscore (_dbname);
	value P = permlist;
	P << perms;
	
	string qry = "INSERT INTO mysql.user (Host,User,Password) "
				 "VALUES ('localhost',%Q,%Q)"
				 %format (username, passwd);
	
	if (! sock.query (qry)) return false;
	
	qry = "INSERT INTO mysql.db (Host,Db,User";
	foreach (p, P)
	{
		qry += ",";
		qry += p.id();
	}
	qry += ") VALUES ('%%',%Q,%Q" %format (dbname, username);
	foreach (p, P)
	{
		qry += ",%Q" %format (p);
	}
	qry += ")";
	
	if (! sock.query (qry))
	{
		qry = "DELETE FROM mysql.user WHERE "
			  "User=%Q and Host='localhost'" %format (username);
		sock.query (qry);
		sock.query ("FLUSH PRIVILEGES");
		return false;
	}
	
	sock.query ("FLUSH PRIVILEGES");
	return true;
}

// ==========================================================================
// METHOD mysqlControl::deleteUser
// ==========================================================================
bool mysqlControl::deleteUser (const string &_dbname, const string &username)
{
	string dbname = escapeUnderscore (_dbname);
	sock.query ("DELETE FROM mysql.db WHERE User=%Q AND"
				" Db=%Q" %format (username, dbname));
	
	sock.query ("DELETE FROM mysql.user WHERE User=%Q" %format (username));
	sock.query ("FLUSH PRIVILEGES");
	return true;
}

// ==========================================================================
// METHOD mysqlControl::updateUser
// ==========================================================================
bool mysqlControl::updateUser (const string &_dbname, const string &username,
							   const value &perms)
{
	string dbname = escapeUnderscore (_dbname);

	value P = permlist;
	P << perms;
	
	string qry = "UPDATE mysql.db SET ";
	bool first = true;
	
	foreach (p, P)
	{
		if (first) first = false;
		else qry.strcat (',');
		
		qry += "%s=%Q" %format (p.id(), p);
	}
	
	qry += " WHERE User=%Q AND Db=%Q" %format (username,dbname);
	
	if (! sock.query (qry)) return false;
	sock.query ("FLUSH PRIVILEGES");
	return true;
}

// ==========================================================================
// METHOD mysqlControl::updateUserPassword
// ==========================================================================
bool mysqlControl::updateUserPassword (const string &username,
									   const string &cryptedpasswd)
{
	string qry = "UPDATE mysql.user SET Password=%Q WHERE "
				 "USER=%Q" %format (cryptedpasswd, username);
	
	if (! sock.query (qry)) return false;
	sock.query ("FLUSH PRIVILEGES");
	return true;
}

// ==========================================================================
// METHOD mysqlControl::addUserHost
// ==========================================================================
bool mysqlControl::addUserHost (const string &username, const string &host)
{
	if (! sock.query ("INSERT INTO mysql.user(Host,User,Password) "
					  "SELECT %Q,User,Password FROM mysql.user "
					  "WHERE User=%Q AND Host='localhost'"
					  %format (host,username)))
	{
		return false;
	}
	
	sock.query ("FLUSH PRIVILEGES");
	return true;
}

// ==========================================================================
// METHOD mysqlControl::deleteUserHost
// ==========================================================================
bool mysqlControl::deleteUserHost (const string &username, const string &host)
{
	if (! sock.query ("DELETE FROM mysql.user WHERE User=%Q AND "
					  "Host=%Q" %format (username, host)))
	{
		return false;
	}
	
	sock.query ("FLUSH PRIVILEGES");
	return true;
}
