// ThreadSpy.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "iGO-MySize-DLL.h"
#include <imagehlp.h>
#include <Tlhelp32.h>
#include <stdio.h>
#include <windows.h>

HookStruct HookedFunctions[] =
{
	{"Gdi32.dll"   , "BitBlt"      , MyBitBlt      , NULL, NotHookedYet},
};

// Simple number of element in the array
#define HookedFunctionSize (sizeof(HookedFunctions) / sizeof(HookedFunctions[0])) 

// The main protection against multi access
CRITICAL_SECTION	mhSection;
HANDLE				hCurrentHandle;

HINSTANCE hInst;
UINT UWM_MOUSEMOVE;
HHOOK hook;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	char msg[256];
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			//MessageBox(NULL, "Spying DLL is started", "None", 0);
			InitializeCriticalSection(&mhSection);
			hCurrentHandle = hModule;
			// Now hook the functions
			HookFunctions(HookedFunctions, HookedFunctionSize);
			// And replicate in all modules
			HookAllModules(HookedFunctions, HookedFunctionSize);
			//MessageBox(NULL, "Functions are hooked", "None", 0);


			break;
		case DLL_THREAD_ATTACH:

			// Hook each thread that attaches
			// hhook is unique within each thread context
			hhook = SetWindowsHookEx( 
                            WH_GETMESSAGE, 
                            MessageProc, 
                            (HINSTANCE) NULL, GetCurrentThreadId()); 
			break;

		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			//MessageBox(NULL, "Spying DLL is leaving", "None", 0);
			UnhookFunctions(HookedFunctions, HookedFunctionSize);

			DeleteCriticalSection(&mhSection);
			break;
    }
    return TRUE;
}


// Returns the HMODULE that contains the specified memory address
static HMODULE ModuleFromAddress(PVOID pv) 
{
	MEMORY_BASIC_INFORMATION mbi;

	return ((::VirtualQuery(pv, &mbi, sizeof(mbi)) != 0) 
	        ? (HMODULE) mbi.AllocationBase : NULL);
}

static HMODULE Kernel32Module()
{
	return GetModuleHandle("kernel32.dll"); 
}


bool Replace(HookStruct * pStruct, BOOL bHook, HMODULE hMod)
{
	ULONG    ulSize;
	bool	 bResult = false;

	// Get the address of the module's import section
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc;
	pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(hMod, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &ulSize);

	// Does this module has import section ?
	if (pImportDesc == NULL) return false;
__try
{
	// Loop through all descriptors and
	// find the import descriptor containing references to callee's functions
	while (pImportDesc->Name)
	{
		const char* szModName = (const char *)((PBYTE) hMod + pImportDesc->Name);
		if (stricmp(szModName, pStruct->szDLLName) == 0) 
				break;   // Found
		pImportDesc++;
	}

	// Does this module import any functions from this callee ?
	if (pImportDesc->Name == 0) return false;

	// Get caller's IAT 
	PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)( (PBYTE) hMod + pImportDesc->FirstThunk );

	// Replace current function address with new one
	while (pThunk->u1.Function)
	{
		// Get the address of the function address
		PROC* ppfn = (PROC*) &pThunk->u1.Function;
		// Is this the function we're looking for?
		BOOL bFound = (*ppfn == (bHook ? pStruct->pPrevFunc : pStruct->pNewFunc));
		if (bFound) 
		{
			MEMORY_BASIC_INFORMATION mbi;
			::VirtualQuery(ppfn, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
			// In order to provide writable access to this part of the 
			// memory we need to change the memory protection
			if (::VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_READWRITE, &mbi.Protect) == FALSE) return false;

			// Hook the function.
            *ppfn = (PROC)(bHook ? pStruct->pNewFunc : pStruct->pPrevFunc);
			bResult = true;
			// Restore the protection back
            DWORD dwOldProtect;
			::VirtualProtect(mbi.BaseAddress, mbi.RegionSize, mbi.Protect, &dwOldProtect);
			break;
		} // if
		pThunk++;
	} // while
}
__finally
{
//	Debug1("Some exception occured : %d", 0);
}


	return bResult;
}


bool ReplaceFunc(HMODULE hMod, HookStruct * pStruct)
{
	IMAGE_DOS_HEADER * dosheader = (IMAGE_DOS_HEADER *)hMod;
	IMAGE_OPTIONAL_HEADER * opthdr = (IMAGE_OPTIONAL_HEADER *) ((BYTE*)hMod+dosheader->e_lfanew+24);
	IMAGE_IMPORT_DESCRIPTOR *descriptor = (IMAGE_IMPORT_DESCRIPTOR*)((BYTE*)dosheader+opthdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	HANDLE hand=GetCurrentProcess();
	HMODULE ker=GetModuleHandle("kernel32.dll");

	while(descriptor->FirstThunk)
	{
		 char* dllname=(char*)((BYTE*)hMod+descriptor->Name);
		 if (stricmp(dllname, pStruct->szDLLName))
		 {
			 descriptor++;
			 continue;
		 }
		 
		 IMAGE_THUNK_DATA* thunk=(IMAGE_THUNK_DATA*)((BYTE*)dosheader+descriptor->OriginalFirstThunk);
		 
		 int x=0;
		 while(thunk->u1.Function)
		 {
			char * functionname = (char*)((BYTE*)dosheader+(unsigned)thunk->u1.AddressOfData+2);
			DWORD* IATentryaddress = (DWORD*)((BYTE*)dosheader+descriptor->FirstThunk)+x;

			if (stricmp(functionname, pStruct->szFuncName) == 0)
			{
				DWORD addr=(DWORD)pStruct->pNewFunc;
				DWORD byteswritten;    
				WriteProcessMemory(hand, IATentryaddress, &addr, 4, &byteswritten);
			}
			x++;
			thunk++;
		}

		descriptor++;

	}

	CloseHandle(hand);
	
	return true;
}
 
// Now hook the function inside any module from now
void HookFunctions(HookStruct array[], const int size)
{
	int i = 0;
	HMODULE  hMod = ModuleFromAddress(fnThreadSpy);

	for (; i < size; i++)
	{
		if (array[i].flags == NotHookedYet)
		{
		
			EnterCriticalSection(&mhSection);

			// Now hook the function
			array[i].pPrevFunc = GetTrueProcAddress(::GetModuleHandleA(array[i].szDLLName), array[i].szFuncName);
			
			// Maybe the dll is not loaded yet, so load it
			if (array[i].pPrevFunc == NULL)
			{
				HMODULE hMod = ::LoadLibraryA(array[i].szDLLName);
				if (hMod != NULL)
					array[i].pPrevFunc = GetTrueProcAddress(::GetModuleHandleA(array[i].szDLLName), array[i].szFuncName);
			} 		

			// Found the previous function pointer, so prepare the next function pointer
			if (array[i].pPrevFunc != NULL)
			{
				hMod = GetModuleHandle(array[i].szDLLName);
				if (Replace(&array[i], TRUE, hMod) == TRUE)
				{
					array[i].flags = Hooked;
				}
			}
			LeaveCriticalSection(&mhSection);
			void * pPtr = ::TextOutA;
		}
	}

}

void HookAllModules(HookStruct array[], const int size)
{
	// Now replace in all the module of the current process
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,GetCurrentProcessId());
	MODULEENTRY32 mod;
	int i;
	mod.dwSize = sizeof(MODULEENTRY32);

	Module32First(snap,&mod);
	
	HMODULE first = mod.hModule;
	for (i = 0; i < size; i++)
		Replace(&array[i], TRUE, first);
	
	while(Module32Next(snap,&mod))
	{
		HMODULE next = mod.hModule;
		if (next == hCurrentHandle) continue;
		for (i = 0; i < size; i++)
			Replace(&array[i], TRUE, next);
	}
}

void UnhookFunctions(HookStruct array[], const int size)
{
	int i = 0;
	HMODULE  hMod = ModuleFromAddress(fnThreadSpy);
	for (; i < size; i++)
	{
		if (array[i].flags == Hooked)
		{
			EnterCriticalSection(&mhSection);
			// Found the previous function pointer, so prepare the next function pointer
			if (array[i].pPrevFunc != NULL)
			{
				if (Replace(&array[i], FALSE, hMod) == TRUE)
				{
					array[i].flags = NotHookedYet;
				}
			}
			LeaveCriticalSection(&mhSection);
		}
	}
}

// This is an ugly function
void * FindFunction(LPCSTR lpName)
{
	EnterCriticalSection(&mhSection);
	for (int i = 0; i < HookedFunctionSize; i++)
	{
		if (strcmp(lpName, HookedFunctions[i].szFuncName) == 0)
		{
			LeaveCriticalSection(&mhSection);
			return HookedFunctions[i].pNewFunc;
		}
	}
	LeaveCriticalSection(&mhSection);
	return NULL;
}

// This is an example of an exported function.
THREADSPY_API int fnThreadSpy(void)
{
	return 42;
}

