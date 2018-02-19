#include "StdAfx.h"
#include "Logoff.h"

CLogoff::CLogoff(void)
{
   TCHAR buffer[ 100 ];

   // Find out if the OS is Windows NT/2000/XP...
   GetEnvironmentVariable( L"OS", buffer, 100 );
   if( wcscmp( L"Windows_NT", buffer ) == 0 )
      m_isWinNT = TRUE;
   else
      m_isWinNT = FALSE;

}

CLogoff::~CLogoff(void)
{
}
void CLogoff::EnableShutdownPrivileges( void ){

   // Variables...
   HANDLE token;
   TOKEN_PRIVILEGES privileges;

   // Get the current process token handle...
   if( !OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES
                         | TOKEN_QUERY, &token ))
      AfxMessageBox( TEXT( "ERROR: Unable to open process token." ),
                     NULL, MB_OK | MB_ICONERROR );

   // Get the LUID for shutdown privilege...
   LookupPrivilegeValue( NULL, SE_SHUTDOWN_NAME, &privileges.Privileges[ 0 ].Luid );

   // Set parameters for AdjustTokenPrivileges...
   privileges.PrivilegeCount = 1;
   privileges.Privileges[ 0 ].Attributes = SE_PRIVILEGE_ENABLED;

   // Enable shutdown privilege...
   AdjustTokenPrivileges( token, FALSE, &privileges, 0, (PTOKEN_PRIVILEGES)NULL, 0 );
   if( GetLastError() != ERROR_SUCCESS )
      AfxMessageBox( TEXT( "ERROR: Uanble to adjust token privileges." ),
                     NULL, MB_OK | MB_ICONERROR );

}//end function EnableShutdownPrivileges()
void CLogoff::DisableShutdownPrivileges( void ){

   // Variables...
   HANDLE token;
   TOKEN_PRIVILEGES privileges;

   // Get the current process token handle...
   if( !OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES
                         | TOKEN_QUERY, &token ))
      AfxMessageBox( TEXT( "ERROR: Unable to open process token." ),
                     NULL, MB_OK | MB_ICONERROR );

   // Get the LUID for shutdown privilege...
   LookupPrivilegeValue( NULL, SE_SHUTDOWN_NAME, &privileges.Privileges[ 0 ].Luid );

   // Set parameters for AdjustTokenPrivileges...
   privileges.PrivilegeCount = 1;
   privileges.Privileges[ 0 ].Attributes = 0;

   // Disable shutdown privilege...
   AdjustTokenPrivileges( token, FALSE, &privileges, 0, (PTOKEN_PRIVILEGES)NULL, 0 );
   if( GetLastError() != ERROR_SUCCESS )
      AfxMessageBox( TEXT( "ERROR: Uanble to adjust token privileges." ),
                     NULL, MB_OK | MB_ICONERROR );

}//end function DisableShutdownPrivileges()
void CLogoff::Logoff( void ){
   ExitWindowsEx( EWX_LOGOFF, 0 );
}//end function Logoff()

void CLogoff::Poweroff( void ){
   if( m_isWinNT ) EnableShutdownPrivileges();
   ExitWindowsEx( EWX_POWEROFF, 0 );
   if( m_isWinNT ) DisableShutdownPrivileges();
}//end function Poweroff()

void CLogoff::Restart( void ){
   if( m_isWinNT ) EnableShutdownPrivileges();
   ExitWindowsEx( EWX_REBOOT, 0 );
   if( m_isWinNT ) DisableShutdownPrivileges();
}//end function Restart()

void CLogoff::Shutdown( void ){
   if( m_isWinNT ) EnableShutdownPrivileges();
   ExitWindowsEx( EWX_SHUTDOWN, 0 );
   if( m_isWinNT ) DisableShutdownPrivileges();
}//end function Shutdown()
