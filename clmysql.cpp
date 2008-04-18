// --------------------------------------------------------------------------
// OpenPanel - The Open Source Control Panel
// Copyright (c) 2006-2007 PanelSix
//
// This software and its source code are subject to version 2 of the
// GNU General Public License. Please be aware that use of the OpenPanel
// and PanelSix trademarks and the IconBase.com iconset may be subject
// to additional restrictions. For more information on these restrictions
// and for a copy of version 2 of the GNU General Public License, please
// visit the Legal and Privacy Information section of the OpenPanel
// website on http://www.openpanel.com/
// --------------------------------------------------------------------------

#include "clmysql.h"
#include <grace/exception.h>

using namespace  clmysql;

THROWS_EXCEPTION (mysqlInitException, 0x8f93facd, "Error in mysql_init()");
THROWS_EXCEPTION (mysqlConnectException, 0xd60603e5, "Error in mysql_real_connect()");

//	===========================================================================
///	clientmysql::connect
//	===========================================================================
void clientmysql::connect (void)
{
	string err;

	m_conn = mysql_init (NULL);
	if (m_conn == NULL)
		throw (mysqlInitException());
	
	if (mysql_real_connect (
			m_conn,
			_host.str(),
			_username.str(),
			_password.str(),
			NULL,
			_port,
			NULL,
			0)
		== NULL)
	{
		string err;
		err.printf ("mysql_real_connect() failed:\nError %u (%s)\n",
				 mysql_errno (m_conn), mysql_error (m_conn));
				 
		throw (mysqlConnectException (err.str()));
	}
}


//	===========================================================================
/// clientmysql::close
//	===========================================================================
void clientmysql::close (void)
{
	// Close mysql connection
	mysql_close (m_conn);
}

		
//	===========================================================================
/// clientmysql::createdatabase
//	===========================================================================
bool clientmysql::createdatabase (const string &database)
{
	string query;
	
	query.printf ("CREATE DATABASE %S", database.str());

	if (mysql_query (m_conn, query.str()))
	{
		// Error executing command, or something else
		// err.printf ("mysql_query() <CREATE DATABASE> failed:\nError %u (%s)\n",
		//		 mysql_errno (m_conn), mysql_error (m_conn));
				 
		_errno = mysql_errno (m_conn);
		_error = mysql_error (m_conn);
	
		return false;
	}
	
	return true;
}


//	===========================================================================
/// clientmysql::removedatabase
//	===========================================================================
bool clientmysql::removedatabase (const string &database)
{
	string query;
	
	query.printf ("DROP DATABASE %S", database.str());

	if (mysql_query (m_conn, query.str()))
	{
		// Error executing command, or something else
		// err.printf ("mysql_query() <DROP DATABASE>  failed:\nError %u (%s)\n",
		//		 mysql_errno (m_conn), mysql_error (m_conn));
				 
		_errno = mysql_errno (m_conn);
		_error = mysql_error (m_conn);
	
		return false;
	}
	
	return true;
}


//	===========================================================================
/// clientmysql::createuser
//	===========================================================================
bool clientmysql::createuser (const string &username, const string &athost,
							  const string &passwd,
					 		  const string &database, const string &priv)
{
	string query;
	query = "GRANT USAGE ON %S.* TO %M@%M IDENTIFIED BY "
			"PASSWORD %M" %format (database,username,
			athost?athost:"%", passwd);

	// Execute query
	if (mysql_query (m_conn, query.str()))
	{
		_errno = mysql_errno (m_conn);
		_error = mysql_error (m_conn);
	
		return false;		
	}
	
	if (! updateuser (username, athost, passwd, database, priv))
		return false;
		
	return true;
}


//	===========================================================================
/// clientmysql::updateuser
//	===========================================================================
bool clientmysql::updateuser (const string &username, const string &athost,
							  const string &passwd,
					 		  const string &database, const string &priv)
{
	string query;
	
	
	// Set privileges
	caseselector(priv)
	{
		incaseof("read"):
			query = "GRANT SELECT ON %S.* TO %M@%M IDENTIFIED BY "
					"PASSWORD %M" %format (database,username,
					athost ? athost : "%", passwd);
			break;
			
		incaseof("read-write"):
			query = "GRANT SELECT,INSERT,UPDATE,DELETE ON %S.* TO "
					"%M@%M IDENTIFIED BY PASSWORD %M" %format (database,
					username, athost ? athost : "%", passwd);
			break;
		
		incaseof("admin"):
			query = "GRANT ALL PRIVILEGES ON %S.* TO %M@%M IDENTIFIED "
					"BY PASSWORD %M WITH GRANT OPTION" %format (database,
					username, athost ? athost : "%", passwd);
			break;
			
		defaultcase:
			_error = "Unknown privilege level (%s)" %format(priv);
			_errno = 1;
			return false;
	}

	// Execute query
	if (mysql_query (m_conn, query.str()))
	{
		_errno = mysql_errno (m_conn);
		_error = mysql_error (m_conn);
	
		return false;
	}


	return true;
}
					 

//	===========================================================================
/// clientmysql::removeuser
//	===========================================================================
bool clientmysql::removeuser (const string &username, const string &athost, const string &db)
{
	string query;
	query = "REVOKE ALL PRIVILEGES ON %S.* FROM %M@%M" %format (db,username,
			 athost);
				  
	// Execute query
	if (mysql_query (m_conn, query.str()))
	{
	    _errno = mysql_errno (m_conn);
		_error = mysql_error (m_conn);
	
		if (_errno != ER_NONEXISTING_GRANT) return false;		
	}

	query = "REVOKE GRANT OPTION ON %S.* FROM %M@%M" %format (db,username,athost);
				  
	// Execute query
	if (mysql_query (m_conn, query.str()))
	{
		_errno = mysql_errno (m_conn);
		_error = mysql_error (m_conn);
	
		if (_errno != ER_NONEXISTING_GRANT) return false;		
	}

	query = "DROP USER %M@%M" %format (username, athost);	
	
	if (mysql_query (m_conn, query.str()))
	{
		_errno = mysql_errno (m_conn);
		_error = mysql_error (m_conn);
	
		return false;
	}
	
	return true;
}


//	===========================================================================
/// clientmysql::flushprivileges
//	===========================================================================
bool clientmysql::flushprivileges (void)
{
	// If no errors have been occured, flush privileges
	// Execute query
	if (mysql_query (m_conn, "FLUSH PRIVILEGES"))
	{
		// Error executing command, or something else
		// err.printf ("mysql_query() <FLUSH PRIVILEGES>  failed:\nError %u (%s)\n",
		//		 mysql_errno (m_conn), mysql_error (m_conn));
				 
		_errno = mysql_errno (m_conn);
		_error = mysql_error (m_conn);
	
		return false;
	}
	

	return true;
}

