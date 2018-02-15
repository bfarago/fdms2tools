using System;
using System.Runtime.InteropServices;

/*
 Change mono config before run !
  $monodir\etc\mono\config:
  <dllmap dll="fdms2" target="fdms2.dll" />
  or
  <dllmap dll="fdms2" target="libfdms2.so" />
*/

class Fdms2Lib {
    [DllImport("fdms2.dll", EntryPoint="getVersionMajor")]
	static extern int getVersionMajor();
    [DllImport("fdms2.dll")]
	static extern int getVersionMinor();	
public  int m_iMajor;
public  int m_iMinor;

public  void init(){
	try {
	 m_iMajor = getVersionMajor();
	 m_iMinor = getVersionMinor();
	 Console.WriteLine(m_iMajor.ToString());
	 Console.WriteLine(m_iMinor.ToString());
	}
	catch (DllNotFoundException e) {
	    Console.WriteLine(e.ToString());
	}
	catch (EntryPointNotFoundException e) {
	    Console.WriteLine(e.ToString());
	}
        }
public static void Main(){
         Fdms2Lib l= new Fdms2Lib();
         l.init();
       }
};