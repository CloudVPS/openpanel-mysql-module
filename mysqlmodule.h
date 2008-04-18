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

#ifndef _mysqlmodule_H
#define _mysqlmodule_H 1

#include <moduleapp.h>
#include <grace/system.h>
#include <grace/configdb.h>

#include "clmysql.h"

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
	void	 onsendresult ()
			 {
			 	delete clmysql;
			 }

	
private:

			 //	 =============================================
			 /// Delete user and it's userhosts below
			 //	 =============================================
	bool	 deleteuser (const value &);
	
protected:

	appconfig		 conf;			///< Modules configuration data
	value			 networkconf;	///< Zone configuration
	clientmysql	 	*clmysql;		///< Mysql Client class

	
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

#endif