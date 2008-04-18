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

#ifndef _CLMYSQL_H
#define _CLMYSQL_H 1

#include <grace/str.h>
#include <grace/value.h>

#include <mysql.h>
#include <mysqld_error.h>


namespace clmysql
{
	enum userpriv
	{
		priv_read		= 0x00,
		priv_readwrite	= 0x01,
		priv_admin		= 0x02
	};
}


///
/// MySQL Client class with all 
/// module needs in it
///
class clientmysql
{

	public:
				 //  =====================================================
				 /// Constructor
				 //  =====================================================
				 clientmysql (const string &host, const string &user, 
				 			  const string &passwd, unsigned int port)
				 {
					_host		= host;
					_username	= user;
					_password	= passwd;
					_port		= port;
					
					connect ();
				 }
				 
				 //  =====================================================
				 /// Destructor
				 //  =====================================================
				~clientmysql (void)
				 {
				 	close ();
				 }


				 //  =====================================================
				 /// Connect to database
				 //  =====================================================
		void	 connect (void);

		
				 //  =====================================================
				 /// Close database connection
				 //  =====================================================
		void	 close (void);
		
				
				 //  =====================================================
				 /// Create a database
				 /// \param database
				 //  =====================================================
		bool	 createdatabase (const string &);


				 //  =====================================================
				 /// Delete a database
				 /// \param database Database name
				 //  =====================================================
		bool	 removedatabase (const string &);

				 //  =====================================================
				 /// Creates a user on a single database with pre-selected 
				 /// privileges
				 /// \param username name of the user to create
				 /// \param athost hostname where the user connects from
				 /// \param passwd The user's password
				 /// \param database Database to privilege user on
				 /// \param priv Privilege level
				 //  =====================================================
		bool	 createuser (const string &, const string &, const string &,
							 const string &, const string &);


				 //  =====================================================
				 /// Update user data, password / priivileges
				 /// \param username Name of the user to update
				 /// \param athost Host where the user connects from
				 /// \param passwd User's password
				 /// \param database The database to privilege the user
				 /// \param priv Privilege level
				 //  =====================================================
		bool	 updateuser (const string &, const string &, const string &,
							 const string &, const string &);
							 
							 
				 //  =====================================================
				 /// Remove a user from the database
				 /// \param username Users name
				 /// \param athost Host where the user connects from
				 //  =====================================================
		bool	 removeuser (const string &, const string &, const string &);
							 
				 //  =====================================================
				 /// Flush MySQL privilege table
				 //  =====================================================
		bool	 flushprivileges (void);
	
				 //  =====================================================
				 /// Get error number from last method
				 //  =====================================================
		int		 errorno (void)
				 {
				 	return _errno;
				 }
				 
				 //  =====================================================
				 /// Get error string generated by the last method
				 //  =====================================================
		string  &error (void)
				 {
					return _error;
				 }
	
	private:
				 // Execute a mysql query
		value	 query (const string &query);
		
	
		// Mysql connection paramters
		string	 	 _host;
		string	 	 _username;
		string	 	 _password;
		unsigned
		int		 	 _port;
		
		// Error varialbles
		string		 _error;
		int			 _errno;

		// MySQL Variables
		MYSQL		*m_conn;
		MYSQL_RES	*m_res;
		MYSQL_ROW	 m_row;
};


#endif