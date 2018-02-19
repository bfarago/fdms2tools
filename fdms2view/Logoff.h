#pragma once

// Class definition...
class CLogoff{

public:

   // Constructor...
   CLogoff ( void );
   // Destructor...
   ~CLogoff( void );

   // Public functions...
   void Logoff( void );
   void Poweroff( void );
   void Restart( void );
   void Shutdown( void );

private:

   // Data members...
   BOOL m_isWinNT;

   // Private functions...
   void EnableShutdownPrivileges( void );
   void DisableShutdownPrivileges( void );

};//end CLogoff
