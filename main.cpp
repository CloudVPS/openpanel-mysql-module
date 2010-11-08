// This file is part of OpenPanel - The Open Source Control Panel
// OpenPanel is free software: you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation, using version 3 of the License.
//
// Please note that use of the OpenPanel trademark may be subject to additional 
// restrictions. For more information, please visit the Legal Information 
// section of the OpenPanel website on http://www.openpanel.com/

#include <opencore/moduleapp.h>
#include "mysqlmodule.h"

#include <grace/file.h>
#include <grace/filesystem.h>
#include <grace/exception.h>


using namespace moderr;

APPOBJECT(mysqlmodule);


#define pwdfile		"mysql.pwd"

void mysqlmodule::onsendresult (void)
{
	delete mcontrol;
}

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
    	mcontrol = new mysqlControl ("openpanel",sqlpwfile);
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
	
	string command = v["OpenCORE:Command"];
	
	caseselector (v["OpenCORE:Session"]["classid"])
	{
		// database actions
		incaseof ("MySQL:Database") : 
			caseselector (command)	
			{
				incaseof ("create") :
					if (! mcontrol->createDatabase (dbid))
					{
						sendresult (err_module, "Error on CREATE DATABASE");
						return false;
					}
					break;
					
				incaseof ("delete") :
					mcontrol->dropDatabase (dbid);
					break;
					
				defaultcase:
					sendresult (err_command, "Invalid command");
					break;
			}
			break;

		// user external hosts
		incaseof ("MySQL:DBUserhost") :
		
			string idHost = v["MySQL:DBUserhost"]["metaid"];
			string idUser = v["MySQL:DBUser"]["id"];
		
			caseselector (command)	
			{
				incaseof ("create") :
					if (idHost == "localhost")
					{	
						sendresult (err_module, "Localhost permissions "
									"already assumed by default");
						return false;
					}
					
					if (! mcontrol->addUserHost (idUser, idHost))
					{
						sendresult (err_module, "Error adding user host");
						return false;
					}
					break;
					
				incaseof ("delete") :
					mcontrol->deleteUserHost (idUser, idHost);
					break;
					
				defaultcase:
					sendresult (err_command, "Command not supported");
					return false;
			}
			break;
		
		incaseof ("MySQL:DBUser") : 
		{
			const value &U = v["MySQL:DBUser"];
			string idUser = U["id"];
			value perms;
			
			if (U.exists ("permissions"))
			{
				caseselector (U["permissions"])
				{
					incaseof ("admin") :
						perms = mysqlControl::permsAdmin(); break;
					
					incaseof ("read-write") :
						perms = mysqlControl::permsReadWrite(); break;
					
					defaultcase :
						perms = mysqlControl::permsRead(); break;
				}
			}
			
		    caseselector (command)	
			{
				incaseof ("create") :
					if (! mcontrol->addUser (dbid, idUser, U["password"], perms))
					{
						sendresult (err_module, "Error creating user");
						return false;
					}
        			break;
        			
    			incaseof ("delete") :
    				mcontrol->deleteUser (dbid, idUser);
					break;
				
				defaultcase:
					if (! mcontrol->updateUser (dbid, idUser, perms))
					{
						sendresult (err_module, "Error setting permissions");
						return false;
					}
					
					if (U["password"].sval())
					{
						if (! mcontrol->updateUserPassword (idUser, U["password"]))
						{
							sendresult (err_module, "Error setting password");
							return false;
						}
					}
					break;
			}
		}
			
		defaultcase:
			sendresult (err_context, "Unknown class");
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
