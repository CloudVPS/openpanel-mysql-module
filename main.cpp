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

#include <moduleapp.h>
#include "mysqlmodule.h"

#include <grace/file.h>
#include <grace/filesystem.h>
#include <grace/exception.h>


using namespace moderr;
using namespace clmysql;

APPOBJECT(mysqlmodule);


#define pwdfile		"mysql.pwd"


//  =========================================================================
/// Main method.
//  =========================================================================
int mysqlmodule::main (void)
{
	string conferr;

	// Add configuration watcher
	conf.addwatcher ("config", &mysqlmodule::confSystem);


   // Load will fail if watchers did not valiate.
    if (! conf.load ("openpanel.module.mysql", conferr))
    {   
        ferr.printf ("%% Error loading configuration: %s\n", conferr.str());
        return 1;
    } 		
    
    // Setup a mysql database connection
    // This is for testing purposes now
    try
    {
    	string sqlpwfile;
    	sqlpwfile = authd.getfiledata (pwdfile);
		sqlpwfile = sqlpwfile.stripchar ('\n');    
    
    	// Try to setup connection
    	clmysql = new clientmysql ("localhost", "openpanel", sqlpwfile, 0);
   	}
   	catch (exception e)
   	{	
   		// External error from mysql, undefined 
   		// inside the module's context
   		sendresult (err_unknown, e.description);
   	}
 		
	caseselector (data["OpenCORE:Command"])
	{
		// Crypt password or try to,
		// exit program afterwards, cryptpassword
		// will generate its own error or output
		incaseof ("crypt") :
			cryptpassword (data);
			return 0;
			
// TODO:
//	Remove the commented checkconfig
//	lines, these are set cause an incomplete 
// 	checkconfig function
//
//
		incaseof ("create") : 
			if (! checkconfig (data))
				return 0;

			if(! writeconfiguration (data))
				return false;
			break;
		incaseof ("update") :
			if (! checkconfig (data))
				return 0;

			if (! writeconfiguration (data))
				return 0;
			break;
		incaseof ("delete") :
			if (! writeconfiguration (data))
				return 0;
			break;
		 	
		incaseof ("validate") : ;
			
		defaultcase:
			sendresult (err_command, 
						"Unsupported command");
			return 0;
	}

	sendresult (moderr::ok, "");
				
	return 0;
}


//  =========================================================================
//	METHOD: mysqlmodule::cryptpassword
//  =========================================================================
bool mysqlmodule::cryptpassword (const value &v)
{

	if (v["OpenCORE:Session"]["classid"] == "MySQL:DBUser")
	{
		if (v.exists ("password"))
		{		
			char 	p[128];
			value 	returndata;
			
			make_scrambled_password (p, v["password"].cval());
		
			returndata["password"] = p;
			sendresult (ok, "", returndata);
			
			return true;
		}
		else
		{
			sendresult (err_value, 
						"only the password field has crypt support");
						
			return false;
		}
	}
	else
	{
		sendresult (err_context, 
					"classid out of crypt context");
		
		return false;
	}
}

//  =========================================================================
//	METHOD: mysqlmodule::readconfiguration
//  =========================================================================
bool mysqlmodule::readconfiguration (void)
{

	return true;
}

//  =========================================================================
//	METHOD: mysqlmodule::writeconfiguration
//  =========================================================================
bool mysqlmodule::writeconfiguration (const value &v)
{
	string dbid = v["MySQL:Database"]["metaid"];
	
	if (! dbid.validate ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
						 "0123456789-"))
	{
		string error = "Invalid characters in database name";
		sendresult (err_module, error);
		return false;
	}
	
	caseselector (v["OpenCORE:Session"]["classid"])
	{
		// database actions
		incaseof ("MySQL:Database") : 
			caseselector (v["OpenCORE:Command"])	
			{
				incaseof ("create") :
					// Create a new database
					if (! clmysql->createdatabase (dbid))
					{
						string error;
						
						error.printf ("Error creating database\n");
						error.printf ("Code: %i\n", clmysql->errorno());
						error.printf ("Msg: %s\n", clmysql->error().str());
						
						// Send error
						sendresult (err_module, error);
						return false;
					}
					break;
					
				incaseof ("delete") :
					// Delete database
                    clmysql->removedatabase (dbid);
					break;
					
				defaultcase:
					// Invalid command
					sendresult (err_command, 
								"MySQL:Database only supports: create, delete");
					break;
			}
			break;

		// user external hosts
		incaseof ("MySQL:DBUserhost") : 
			caseselector (v["OpenCORE:Command"])	
			{
				incaseof ("create") :
					// Add a new external host for this user
					statstring objid;
					objid = v["MySQL:DBUserhost"]["metaid"];
					if (objid == "localhost")
					{	
					    string error;
						error.printf ("Do not add localhost - localhost always has access\n");
						
						// Send error
						sendresult (err_module, error);
						return false;
					    
					}
					if (! clmysql->createuser (v["MySQL:DBUser"]["id"], objid,
								v["MySQL:DBUser"]["password"], dbid,
                            	v["MySQL:DBUser"]["permissions"])
					   )
					{
						string error;
						error.printf ("Error creating user external host\n");
						error.printf ("Code: %i\n", clmysql->errorno());
						error.printf ("Msg: %s\n", clmysql->error().str());
						
						// Send error
						sendresult (err_module, error);
						return false;
					}					
					break;
					
				incaseof ("delete") :
					// Delete an external host for this user
                    clmysql->removeuser (v["MySQL:DBUser"]["id"],
					    				 v["MySQL:DBUserhost"]["metaid"],
					    				 dbid);
					break;
					
				defaultcase:
					// Invalid command
					sendresult (err_command, 
								"MySQL:DBUserhost only supports: create, delete");
					return false;
			}
			break;
		
		incaseof ("MySQL:DBUser") : 
		{
		    caseselector (v["OpenCORE:Command"])	
			{
				incaseof ("create") :
        			if (! clmysql->createuser (v["MySQL:DBUser"]["id"],
        						"localhost",
        						v["MySQL:DBUser"]["password"],
        						dbid,
                            	v["MySQL:DBUser"]["permissions"])
        			   )
        			{
        				string error;
        				error.printf ("Error creating user@localhost\n");
        				error.printf ("Code: %i\n", clmysql->errorno());
        				error.printf ("Msg: %s\n", clmysql->error().str());
				
        				// Send error
        				sendresult (err_module, error);
        				return false;
        			}
        			break;
    			incaseof ("delete") :
					// Delete an external host for this user
                    clmysql->removeuser (v["MySQL:DBUser"]["id"],
					    				 "localhost", dbid);
					break;
				
				defaultcase:
					// Invalid command
					sendresult (err_command, 
								"MySQL:DBUser only supports: create, delete");
					return false;
			}
		}
			
		defaultcase:
			sendresult 
				(
				err_context, 
				"Supported classes are: MySQL:(Database, DBUser, DBUserhost)"
				);
			return false;
	}


	return true;
}

//  =========================================================================
/// METHOD ::checkdatabasename
//  =========================================================================
bool mysqlmodule::checkdbname (const string &s)
{
	static string DBNameChars ("abcdefghijklmnopqrstuvwxyz"
							   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
							   "0123456789-");
	if(! s.validate(DBNameChars))
	{
		sendresult (err_command, "Invalid character in database name");
		return false;
	}
	
	return true;
}

//  =========================================================================
/// METHOD ::checkhostname
//  =========================================================================
bool mysqlmodule::checkhostname (const string &s)
{
	static string HostNameChars ("abcdefghijklmnopqrstuvwxyz"
					 		     "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
							     "0123456789-.");
	if(! s.validate(HostNameChars))
	{
		sendresult (err_command, "Invalid character in hostname");
		return false;
	}
	
	return true;
}

//  =========================================================================
/// METHOD ::checkconfig
//  =========================================================================
bool mysqlmodule::checkconfig (value &v)
{
	// Check if the given command is supported
	// in combination with the given classid
	
	if (v["OpenCORE:Session"]["classid"] == "MySQL:Database")
	{
		// Check if the given command are correct
		if ((v["OpenCORE:Command"] == "create") ||
			(v["OpenCORE:Command"] == "delete") ||
			(v["OpenCORE:Command"] == "validate"))
		{
			if (! v.exists("MySQL:Database"))
			{
				// Given classid not found
				sendresult (err_notfound, "Class not found by given id");
				return false;
			}
		}
		else
		{
			// Unsupported command
			sendresult (err_command, "Given command not supported");
			return false;
		}
		if(!checkdbname(v["MySQL:Database"]["id"]))
			return false;
		
	}
	else if (v["OpenCORE:Session"]["classid"] == "MySQL:DBUser")
	{
		// Check if the given command are correct
		if ((v["OpenCORE:Command"] == "create") ||
			(v["OpenCORE:Command"] == "delete") ||
			(v["OpenCORE:Command"] == "update") ||
			(v["OpenCORE:Command"] == "crypt")  ||
			(v["OpenCORE:Command"] == "validate"))
		{
			if (v.exists("MySQL:DBUser"))
			{
				// Check if all fields exists
				if (! v["MySQL:DBUser"].exists("password"))
				{
					sendresult 
						(
						err_notfound, 
						"No password hash found in MySQL:DBUser"
						);
					return false;					
				}
				else if (! v["MySQL:DBUser"].exists("permissions"))
				{
					sendresult 
						(
						err_notfound, 
						"No permission level found in MySQL:DBUser"
						);
					return false;					
				}
			}
			else
			{
				// Given classid not found
				sendresult (err_notfound, "Class not found by given id");
				return false;
			}			
		}
		else
		{
			// Unsupported command
			sendresult (err_command, "Given command not supported");
			return false;
		}
	}
	else if (v["OpenCORE:Session"]["classid"] == "MySQL:DBUserhost")
	{
		// Check if the given command are correct
		if ((v["OpenCORE:Command"] == "create") ||
			(v["OpenCORE:Command"] == "delete") ||
			(v["OpenCORE:Command"] == "validate"))
		{
			// that's ok
		}
		else
		{
			// Unsupported command
			sendresult (err_command, "Given command not supported");
			return false;
		}
		if(!checkhostname(v["MySQL:DBUserhost"]["id"]))
			return false;
	}
	else
	{
		sendresult 
			(
			err_context, 
			"Supported classes are: MySQL:(Database, DBUser, DBUserhost)"
			);		
			
		return false;
	}
	
	
	// No errors during validation
	return true;
}


//  =========================================================================
/// Recursive user delete
//  =========================================================================
bool mysqlmodule::deleteuser (const value &v)
{
	//
	// Remove the DBUserhosts if they are available
	//
	if (v["MySQL:DBUser"].exists("MySQL:DBUserhost"))
	{
		foreach (h, v["MySQL:DBUser"]["MySQL:DBUserhost"])
		{
			//
			// Delete user with external host `DBUserhost`
			//
			clmysql->removeuser (v["MySQL:DBUser"]["id"].sval(),
							     h["id"].sval(),
                                 v["MySQL:Database"]["metaid"]);
		}
	}

	return true;
}

//  =========================================================================
/// Configuration watcher for the event log.
//  =========================================================================
bool mysqlmodule::confSystem (config::action act, keypath &kp,
                const value &nval, const value &oval)
{
	switch (act)
	{
		case config::isvalid:
			return true;

		case config::create:
			return true;		
	}

	return false;
}
