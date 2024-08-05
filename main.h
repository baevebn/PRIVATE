#pragma once
#include <Windows.h>
#include <cstdio>
#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <algorithm>
#include <Psapi.h>
#include <cstdlib>
#include <tlhelp32.h>
#include <locale>
#include <filesystem>
#include <ctime>
#include <chrono>
#include <stdexcept>
#include <codecvt>
#include <unordered_set>
#include <WtsApi32.h>
#include <unordered_map>
#include <sstream>
#include <Shlwapi.h>
#include <vector>
#include <mmsystem.h>
#include <stdarg.h>
#include <cstdarg>
#include <WbemIdl.h>
#include <thread>
#include <wlanapi.h>
#include <wininet.h>

#pragma comment(lib,"Winmm.lib")

#pragma comment(lib,"Wininet.lib")
#pragma comment(lib,"Wlanapi.lib")

#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"wbemuuid.lib")

#pragma warning(disable:4996)
#pragma warning(disable:4129)

#pragma warning(disable:4101) 
#pragma warning(disable:4018)

using namespace std;
using namespace chrono;

namespace fs = filesystem;
using namespace filesystem;

inline string convertWstringToString(wstring&str)
{
	wstring_convert<codecvt_utf8_utf16<wchar_t>, wchar_t>converter;
	return converter.to_bytes(str);
}

inline vector<wstring> readExtensionsFromFile(const char* filename, const string&section) 
{
	vector<wstring> NEXT;
	ifstream file(filename);

	file.imbue(std::locale(file.getloc(), new std::codecvt_utf8_utf16<wchar_t>));
	wstring_convert<codecvt_utf8_utf16<wchar_t>, wchar_t>converter;

	if (file.is_open())
	{
		string line;
		bool inNameSection = false;

		while (getline(file, line)) 
		{
			if (line.find('[' + section + ']') != string::npos)
			{
				inNameSection = true;
			}
			else if (inNameSection && line != "") 
			{
				line.erase(0, line.find_first_not_of(" "));
				line.erase(line.find_last_not_of(" ") + 1);

				NEXT.push_back(converter.from_bytes(line));
			}
		}

		file.close();
	}

	return NEXT;
}

inline char* IniRead(const char* filename, const char* section, const char* key)
{
	char* out = new char[MAX_PATH];
	GetPrivateProfileString((LPCSTR)section, (LPCSTR)key, NULL, out,MAX_PATH, (LPCSTR)filename);
	return out;
}

inline string HackExeName()
{
	const size_t len = 260;
	LPSTR buffer = new TCHAR[len];

	GetModuleFileName(NULL, buffer, len);
	char* szHackName = PathFindFileName(buffer);

	return szHackName;
}

inline string GetPathToHard()
{
	char buffer[MAX_PATH];
	GetSystemDirectory(buffer, sizeof(buffer));

	string windowsDir(buffer);
	size_t pos = windowsDir.find('\\', 3);

	string diskPath = windowsDir.substr(0, pos - 7);

	char cmd[256];
	sprintf(cmd, "%s", diskPath.c_str());

	return cmd;
}

inline string GetPathToFull()
{
	DWORD drives = GetLogicalDrives();
	string path;

	for (char i = 'A'; i <= 'Z'; i++)
	{
		if (drives & 1)
		{
			wstring drivePath = (wstring(1, i) + L":\\");
			path = convertWstringToString(drivePath);
		}

		drives >>= 1;
	}

	return path;
}

inline string GetPathToUSB(const char* str)
{
	char szLogicalDrivers[64];
	char USB[MAX_PATH];

	DWORD dwResult = GetLogicalDriveStringsA(sizeof(szLogicalDrivers), szLogicalDrivers);
	string szPath = str;

	if (dwResult > 0 && dwResult <= sizeof(szLogicalDrivers))
	{
		char* szSingleDriver = szLogicalDrivers;

		while (*szSingleDriver)
		{
			UINT driveType = GetDriveTypeA(szSingleDriver);

			if (driveType == DRIVE_REMOVABLE || 
				driveType == DRIVE_FIXED)
			{
				sprintf(USB, "%s", szSingleDriver);
				
			}

			szSingleDriver += strlen(szSingleDriver) + 1;
		}
	}

	return (USB + szPath);
}

inline bool bPath(const char* pszName)
{
	return fs::exists(pszName);
}

inline string szDirHack(const char* szName)
{
	TCHAR buffer[MAX_PATH];
	GetCurrentDirectory(sizeof(buffer), buffer);

	char str[MAX_PATH];
	sprintf(str, "%s\\", buffer);

	string pDir = str;
	return (pDir + szName);
}

inline string pszExtHack(const char* Path, const char* ExtensionName)
{
	string pszStr;

	for (const auto& entry : directory_iterator(Path))
	{
		if (is_regular_file(entry))
		{
			string fileName = entry.path().filename().string();

			if (fileName.find(ExtensionName) != string::npos)
			{
				pszStr = fileName.substr(0, fileName.find_last_of(".")).c_str();
			}
		}
	}

	return pszStr;
}

inline bool bRelease(const wstring& filename)
{
	return (filename.find(L"$Recycle.Bin") != wstring::npos);
}

inline void scanFiles(const char* Path)
{
	DWORD dwBytes = 0;
	char buffer[MAX_PACKAGE_NAME];

	HANDLE hDir = CreateFile(
		Path,
		FILE_LIST_DIRECTORY,
		FILE_SHARE_READ |
		FILE_SHARE_WRITE |
		FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS,
		NULL
	);

	if (hDir == INVALID_HANDLE_VALUE)
	{
		return;
	}

	unordered_set<wstring>modifiedFiles;

	while (true)
	{
		ReadDirectoryChangesW(
			hDir,
			buffer,
			MAX_PACKAGE_NAME,
			TRUE,
			FILE_NOTIFY_CHANGE_FILE_NAME |
			FILE_NOTIFY_CHANGE_DIR_NAME |
			FILE_NOTIFY_CHANGE_LAST_WRITE,
			&dwBytes,
			NULL,
			NULL
		);

		FILE_NOTIFY_INFORMATION* pNotify = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);

		while (pNotify)
		{
			static bool bFile = false;
			char* fileRead = new char[MAX_PATH];

			sprintf(fileRead, "%s.ini", pszExtHack(szDirHack("").c_str(), ".ini").c_str());
			vector<wstring> forbiddenExtensions = readExtensionsFromFile(fileRead, "BlockList");

			wstring pFile(pNotify->FileName, pNotify->FileName + pNotify->FileNameLength / 2);
			wstring wideFileName(pFile);

			wstring::size_type pos = wideFileName.find_last_of(L"\\");

			if (pos != wstring::npos)
			{
				wideFileName = wideFileName.substr(pos + 1);
			}

			for (const auto& extension : forbiddenExtensions)
			{
				if (wideFileName.find(extension) != wstring::npos)
				{
					bFile = true;
					break;
				}
			}

			if (bFile)
			{
				if (!bRelease(pFile))
				{
					if (pNotify->Action == FILE_ACTION_ADDED)
					{
						printf("[ADDED] %s%ls\n", Path, pFile.c_str());
					}

					if (pNotify->Action == FILE_ACTION_MODIFIED)
					{
						if (modifiedFiles.find(pFile) == modifiedFiles.end())
						{
							printf("[MODIFIED] %s%ls\n", Path, pFile.c_str());
							modifiedFiles.insert(pFile);
						}
					}

					if (pNotify->Action == FILE_ACTION_RENAMED_NEW_NAME)
					{
						printf("[REN-NEW] %s%ls\n", Path, pFile.c_str());
					}

					if (pNotify->Action == FILE_ACTION_RENAMED_OLD_NAME)
					{
						printf("[REN-OLD] %s%ls\n", Path, pFile.c_str());
					}
				}

				bFile = false;
			}

			if (pNotify->NextEntryOffset == 0)
			{
				break;
			}

			char* pcNext = reinterpret_cast<char*>(pNotify);
			pcNext += pNotify->NextEntryOffset;
			pNotify = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(pcNext);
		}
	}

	CloseHandle(hDir);
}

inline bool checkInternetConnection()
{
	bool bFlags = false;
	DWORD dwFlags = (INTERNET_CONNECTION_MODEM | INTERNET_CONNECTION_LAN | INTERNET_CONNECTION_PROXY);

	if (InternetGetConnectedState(&dwFlags, 0)) { bFlags = true; }
	else { bFlags = false;  }

	return bFlags;
}