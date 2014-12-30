/*
 ------------------------------------------------------------------------------------------------
       NNNNNN      RRRRRRR             N               NNN       CCCCCCCCCCC  RRRRRRRRRRRR
     NNNNNNNNNN RRRRRRRRRRRR           NNN             NNN     CCCCCCCCCCCCCC RRRRRRRRRRRRRRR
   NNNNNN   NNNRRRRR    RRRRRRR        NNNN            NNN   CCCC             RRR         RRRRR
  NNNN        RRR          RRRRR       NNNNNN          NNN  CCCC              RRR            RRR
  NNN        CCC   CCC        RRRR     NNN NNN         NNN  CCC               RRR            RRR
 NNN        CCC   CCC   CCC     RRR    NNN  NNNN       NNN CCC                RRR         RRRRRR
 NNN       CCC   CCC   CCC      RRR    NNN    NNN      NNN CCC                RRRRRRRRRRRRRRR
 NN       CCC   CCC   CCC        RR    NNN     NNN     NNN CC                 RRRRRRRRRRRRR
 NNN     CCC   CCC   CCC        RRR    NNN      NNN    NNN CCC                RRR RRR
 NNN    CCC   CCC   CCC         RRR    NNN       NNN   NNN CCC                RRR  RRRR
  NNN        CCC   CCC        RRRR     NNN         NNN NNN  CCC               RRR    RRRR
  NNNN            NNN        RRRR      NNN          NNNNNN  CCCC              RRR      RRRR
   NNNNNN     NNNNNRRR   RRRRRR        NNN            NNNN   CCCC             RRR        RRRR
      NNNNNNNNNNNN RRRRRRRRRR          NNN             NNN     CCCCCCCCCCCCCC RRR          RRRR
        NNNNNNN     RRRRRRR            NNN               N       CCCCCCCCCCC  RRR            RRRR
 ------------------------------------------------------------------------------------------------
*/

#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include <string>
#include <fstream>
#include <locale>
#include <codecvt>


/*********************************************/
/* Various definitions                       */
/*********************************************/
#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

//define used namespace
using namespace std;				

//variables for detecting the bit version OS
typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);		
LPFN_ISWOW64PROCESS fnIsWow64Process;							

// define registry paths for 32bit and 64bit systems
LPCWSTR neco = L"SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
LPCWSTR W32Reg = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
LPCWSTR W64Reg = L"SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall";





/****************************************************************************/
/*                                                                          */
/*    Name:         : ws2s()                                                */
/*                                                                          */
/*    Return:       : sdt::string                                           */
/*                                                                          */
/*                                                                          */
/*    Description   : Converts std::wstring to std::string.                 */
/*                                                                          */
/****************************************************************************/
std::string ws2s(const std::wstring& wstr)
{
	typedef std::codecvt_utf8<wchar_t> convert_typeX;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}


/****************************************************************************/
/*                                                                          */
/*    Name:         : IsWow64()												*/
/*                                                                          */
/*    Return:       : FALSE: 32 bit OS                                      */
/*					  TRUE:	 64 bit OS		                                */
/*                                                                          */
/*    Description   : Checks if the app runs on 32 bit or 64 bit OS			*/
/*                                                                          */
/****************************************************************************/
BOOL IsWow64()
{
	BOOL bIsWow64 = FALSE;

	//IsWow64Process is not available on all supported versions of Windows.
	//Use GetModuleHandle to get a handle to the DLL that contains the function
	//and GetProcAddress to get a pointer to the function if available.

	fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
		GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

	if (NULL != fnIsWow64Process)
	{
		if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64))
		{
			//handle error
			cout << "Bit version detection is not available. Using W32 settings\n";
		}
	}
	return bIsWow64;
}


/****************************************************************************/
/*                                                                          */
/*    Name:         : ReadRegValue()										*/
/*                                                                          */
/*    Return:       : wstring::key value                                    */
/*					        		                                        */
/*                                                                          */
/*    Description   : Returns key value as std::wstring						*/
/*                                                                          */
/****************************************************************************/
wstring ReadRegValue(HKEY root, wstring key, wstring name)
{
	HKEY hKey;
	if (RegOpenKeyEx(root, key.c_str(), 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
		throw "Could not open registry key";

	DWORD type;
	DWORD cbData;

	/*********************************************/
	/* catch an exception in case key is missing */
	/*********************************************/
	try
	{
		if (RegQueryValueEx(hKey, name.c_str(), NULL, &type, NULL, &cbData) != ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			throw "Could not read registry value";
		}

		if (type != REG_SZ)
		{
			RegCloseKey(hKey);
			throw "Incorrect registry value type";
		}

		wstring value(cbData / sizeof(wchar_t), L'\0');
		if (RegQueryValueEx(hKey, name.c_str(), NULL, NULL, reinterpret_cast<LPBYTE>(&value[0]), &cbData) != ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			throw "Could not read registry value";
		}

		RegCloseKey(hKey);

		size_t firstNull = value.find_first_of(L'\0');
		if (firstNull != string::npos)
			value.resize(firstNull);

		return value;
	}
	catch (...)
	{
		/*********************************************/
		/* key is missing - use N/A value			 */
		/*********************************************/
		wstring value;
		value = L"N/A";
		RegCloseKey(hKey);
		return value;
	}

	
}


/********************************************************************************************************************/
/*																													*/
/*													MAIN APPLICATION												*/
/*																													*/
/********************************************************************************************************************/
int main(int argc, char* argv[])
{
	int i;
	wstring ws = L"neco";
	ofstream outputFile;




	//check if arguments are supplied
	if (argc > 1)
	{
		//some arguments are passed, let's break them down by what we recognize
		// check if arguments is equal -v to display the tool version information
		if (string(argv[1]) == "-v")
		{
			cout << "Sanity cross-check verze 0.0.1\n";
			exit(0);
		}
		//check if argument is equal -h to display the help information
		else if (string(argv[1]) == "-h")
		{
			cout << "Display help usage information here\n";
			cout << "Pouziti: scheck [switch] [argument]\n";
			exit(0);
		}
		else
		//there has been no recognized parameter at 1st position
		{
			cout << "Nezname parametry\n";
			exit(0);
		}
	}
	else
	{
		cout << "Nejsou zadany parametry, pro napovedu pouzijte paremetr -h\n";
		cout << "\n";
		cout << "Ctu hodnotu z registru...\n";
		//wcout << ReadRegValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{020B8383-8F4E-4ADD-8D61-5ADEB1EBBC70}", L"DisplayVersion");
		cout << "\n";
		wcout << ReadRegValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Active Setup", L"JITSetupPage");
		cout << "\n";

		if (IsWow64())
		{
			cout << "The process is running under WOW64.\n";
			//cout << W64REG;
		}
		else
		{
			cout << "The process is not running under WOW64.\n";
			//cout << W32REG;
		}
		
		cout << "\n";
		cout << "\n";
		cout << "Jdu cist podregistry klice:\n";
		
		HKEY		hRegAdapters;
		DWORD		retCode,subKeyCode;
		TCHAR		achKey[MAX_KEY_LENGTH];			// buffer for subkey name
		DWORD		cbName;							// size of name string 
		TCHAR		achClass[MAX_PATH] = TEXT("");  // buffer for class name 
		DWORD		cchClassName = MAX_PATH;		// size of class string 
		DWORD		cSubKeys = 0;					// number of subkeys 
		DWORD		cbMaxSubKey;				    // longest subkey size 
		DWORD		cchMaxClass;				    // longest class string 
		DWORD		cValues;						// number of values for key 
		DWORD		cchMaxValue;					// longest value name 
		DWORD		cbMaxValueData;					// longest value data 
		DWORD		cbSecurityDescriptor;			// size of security descriptor 
		FILETIME	ftLastWriteTime;				// last write time 

		wstring		achVerValue,achNameValue;		// registry key values - version | name
		
		//LONG		res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\ASP.NET", 0, KEY_READ, &hRegAdapters);
		
		//open uninstall registry
		LONG		res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, neco, 0, KEY_READ, &hRegAdapters);
		// query subkey counts
		retCode = RegQueryInfoKey(hRegAdapters, achClass, &cchClassName, NULL, &cSubKeys, &cbMaxSubKey, &cchMaxClass, &cValues, &cchMaxValue, &cbMaxValueData, &cbSecurityDescriptor, &ftLastWriteTime);

		cout << "Pocet podklicu je: ";
		cout << cSubKeys;
		cout << "\n";

		//open output file - create new one / overwrite existing
		outputFile.open("scheck.txt");


		//loop through subkeys
		for (i = 0; i<cSubKeys; i++)
		{
			cbName = MAX_KEY_LENGTH;
			retCode = RegEnumKeyEx(hRegAdapters, i,
				achKey,
				&cbName,
				NULL,
				NULL,
				NULL,
				&ftLastWriteTime);
			if (retCode == ERROR_SUCCESS)
			{
				//process keys that have just the UUID {xxxx-xxx}
				if (achKey[0] == '{')
				{
					// kontrola jestli podklic existuje
					achVerValue = ReadRegValue(hRegAdapters, achKey, L"DisplayVersion");
					achNameValue = ReadRegValue(hRegAdapters, achKey, L"DisplayName");
					_tprintf(TEXT("(%d) %s \t"), i + 1, achKey);
					wcout << achVerValue;
					cout << "\t";
					//wcout << achNameValue;
					cout << "\n";
					outputFile << ws2s(achKey);
					outputFile << "\t\t";
					outputFile << ws2s(achVerValue);
					outputFile << "\t\t";
					outputFile << ws2s(achNameValue);
					outputFile << "\n";

				}
			}
		}
		outputFile.close();
		RegCloseKey(hRegAdapters);
		exit(0);
	}
	return 0;
	//tady bude parsovani argumentu
}

