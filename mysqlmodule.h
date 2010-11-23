// This file is part of OpenPanel - The Open Source Control Panel
// OpenPanel is free software: you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation, using version 3 of the License.
//
// Please note that use of the OpenPanel trademark may be subject to additional 
// restrictions. For more information, please visit the Legal Information 
// section of the OpenPanel website on http://www.openpanel.com/

#ifndef _mysqlmodule_H
#define _mysqlmodule_H 1

#include <openpanel-core/moduleapp.h>
#include <grace/system.h>
#include <grace/configdb.h>

#include <mysql.h>
#include <mysqld_error.h>

typedef configdb<class mysqlmodule> appconfig;

//  -------------------------------------------------------------------------
/// Main application class.
//  -------------------------------------------------------------------------
class mysqlmodule : public moduleapp
{
public:
		 	 mysqlmodule (void) :
				moduleapp ("openpanel.module.mysql"),
				conf (this)
			 {
			 }
			~mysqlmodule (void)
			 {
			 }

	int		 main (void);


			 // virtual from moduleapp
	void	 onsendresult ();

protected:

	appconfig		 conf;			///< Modules configuration data
	value			 networkconf;	///< Zone configuration
	class mysqlControl	*mcontrol;	///< MySQL controller object.

	
			 //	 =============================================
			 /// Configuration handler 
			 //	 =============================================
	bool     confSystem (config::action act, keypath &path, 
					  	 const value &nval, const value &oval);	
	
			 //	=============================================
			 /// validate the given configuration
			 /// \return true on ok / false on failure
			 //	=============================================
	bool	 checkconfig (value &v);	
	
			 //	=============================================
			 /// Read the current loaded network module
			 /// and sends the network configuration to the 
			 /// standard output or gives an error.
			 /// \return true on ok / false on failure
			 //	=============================================
	bool	 readconfiguration (void);
	
			 //	=============================================
			 /// Writes a network configuration
			 /// \param v given post data
			 /// \return true on ok / false on failure
			 //	=============================================
	bool 	 writeconfiguration (const value &v);
	
			 //	=============================================
			 /// Validate database name
			 /// \param s database name
			 /// \return true on ok / false on failure
			 //	=============================================
	bool	 checkdbname (const string &s);

			 //	=============================================
			 /// Validate host name
			 /// \param s host name
			 /// \return true on ok / false on failure
			 //	=============================================
	bool	 checkhostname (const string &s);

			 //	=============================================
			 /// Crypts a given password and returns the
			 /// hash (mysql_scramble_password())
			 /// \param v given post data
			 /// \return true on ok / false on failure
			 //	=============================================			 
	bool	 cryptpassword (const value &v);
};

class mysqlSocket
{
public:
				 mysqlSocket (const string &_user, const string &_pass);
				~mysqlSocket (void);
	bool		 query (const string &q);

protected:
	MYSQL		*msock;
};

class mysqlControl
{
public:
					 mysqlControl (const string &_user, const string &_pass);
					~mysqlControl (void);

	static value	*permsRead (void);
	static value	*permsReadWrite (void);
	static value	*permsAdmin (void);

	bool			 createDatabase (const string &dbname);
	bool			 dropDatabase (const string &dbname);
	
	bool			 addUser (const string &dbname,
							  const string &username,
							  const string &passwd,
							  const value &perms);
	
	bool			 deleteUser (const string &dbname,
								 const string &username);
							 
	bool			 updateUser (const string &dbname,
								 const string &username,
								 const value &perms);
							 
	bool			 updateUserPassword (const string &username,
										 const string &cryptedpassword);
	
	bool			 addUserHost (const string &username,
								  const string &host);
	
	bool			 deleteUserHost (const string &username,
									 const string &host);
						 
protected:
	string			 user;
	string			 pass;
	value			 permlist;
	mysqlSocket		 sock;
};

#endif
