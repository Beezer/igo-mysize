// ResizeIgo.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "iGO-MySize.h"
#include <stdio.h>
#include "CCommandLine.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	int newx = 0;
	int newy = 0;
	CCommandLine CmdLine;
	CmdLine.Initiate(lpCmdLine);

	char *temp;
	if ((temp = CmdLine.FindEntryc("-width=")) != NULL) {
		newx = atoi(temp);
	}
	
	if ((temp = CmdLine.FindEntryc("-height=")) != NULL) {
		newy = atoi(temp);
	}

	SetCurrentDirectory(FindWD());

	// Get the handle for the new specified process
	STARTUPINFO startup;
	GetStartupInfo(&startup);
	HANDLE hThread;

	char sName[256];
	char sParam[256];
	strcpy(sName, FindIGO());
	char acParam[1024];
	strcpy(acParam, sName);
	if (sParam != NULL && sParam[0])
	{
		strcat(acParam, " ");
		strcat(acParam, sParam);
		if (!::CreateProcess(sName, acParam, NULL, NULL, TRUE, CREATE_SUSPENDED, 0, 0, &startup, &mxProcess)) return false;
	}
	else
	{
		if (!::CreateProcess(sName, NULL, NULL, NULL, TRUE, CREATE_SUSPENDED, 0, 0, &startup, &mxProcess)) return false;
	}

	// Get the library name
	const char * LIBNAME = FindDLL();
	if (LIBNAME == NULL) return false;

	// Allocate space in the remote process for the pathname to our spying DLL
	char * LibName = (char *)::VirtualAllocEx(mxProcess.hProcess, NULL, strlen(LIBNAME) + 1, MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	LibName = (char *)::VirtualAllocEx(mxProcess.hProcess, LibName, strlen(LIBNAME) + 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (LibName == NULL) return false;

	// Copy the DLL's pathname to the remote process's address space
	if (!::WriteProcessMemory(mxProcess.hProcess, (PVOID)LibName, (PVOID)LIBNAME, strlen(LIBNAME)+1, NULL)) return false;
	
	// Get the real address of LoadLibraryA in Kernel32.dll
	PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle("Kernel32"), "LoadLibraryA");
	if (pfnThreadRtn == NULL) return false;

	// Create a remote thread that calls LoadLibraryA(DLLPathname)
	hThread = ::CreateRemoteThread(mxProcess.hProcess, NULL, 0, pfnThreadRtn, (PVOID)LibName, 0, NULL);
	if (hThread == NULL) return false;

	// Wait for the remote thread to terminate
	::WaitForSingleObject(hThread, INFINITE);

	// Get the thread exit code
	DWORD exit;
	GetExitCodeThread(hThread, &exit);

	// So resume the thread
	if (ResumeThread(mxProcess.hThread) == -1)
		return false;



	Sleep (500);

	if (newx > 0 && newy > 0) {

		HWND hwnd = NULL;
		for (int i = 0; i < 10; i++) {
			hwnd = FindWindow("iGO8.class", "iGO8");
			if (hwnd) {

				// User data is empty in iGO window,
				// so we can use this to store our data

				// Find the window size
				WINDOWINFO pwi;
				if (GetWindowInfo(hwnd, &pwi)) {
					// We've got the window info
					RECT pData = pwi.rcClient;
					WORD x = pData.right - pData.left;
					WORD y = pData.bottom - pData.top;

					// Find ratio difference
					WORD xr = (((float)newx / (float)x) * 100.0);
					WORD yr = (((float)newy / (float)y) * 100.0);
					LONG sizeData = MAKELONG(xr, yr);

					// Store our user data
					SetWindowLong(hwnd, GWL_USERDATA, sizeData);

				}

				SetWindowPos(hwnd, HWND_TOP, 0, 0, newx, newy + 25, SWP_NOMOVE | SWP_FRAMECHANGED | SWP_NOZORDER);
				break;
			}
			Sleep(200);
		}
	}
	
	return 0;
}

/*
* Find our working directory
*/
const char * FindWD()
{
	static char thePath[MAX_PATH];
	GetModuleFileName(GetModuleHandle( NULL ), thePath, MAX_PATH);
	strrchr(thePath, '\\')[0] = 0;

	return thePath;
	return NULL;
}

/*
* Find iGO
*/
const char * FindIGO()
{
	static char thePath[MAX_PATH];
	strcpy(thePath, FindWD());
	strcat(thePath, "\\igo_pc.exe");

	return thePath;
	return NULL;
}

/*
* Find our dll
*/
const char * FindDLL()
{
	static char thePath[MAX_PATH];
	strcpy(thePath, FindWD());
	strcat(thePath, "\\iGO-MySize.dll");

	return thePath;
	return NULL;
}



