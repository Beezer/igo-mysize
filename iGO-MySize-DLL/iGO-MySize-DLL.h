#include "mmsystem.h"

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the THREADSPY_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// THREADSPY_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef THREADSPY_EXPORTS
#define THREADSPY_API __declspec(dllexport)
#else
#define THREADSPY_API __declspec(dllimport)
#endif

// Declare the signature for our new functions
WINBASEAPI FARPROC WINAPI MyGetProcAddress(HMODULE hModule, LPCSTR lpProcName);
WINGDIAPI  BOOL    WINAPI MyBitBlt(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, HDC hdcSrc, int nXSrc,  int nYSrc,  DWORD dwRop );
LRESULT WINAPI MessageProc(int nCode, WPARAM wParam, LPARAM lParam);

typedef enum
{
	NotHookedYet = 0,
	Hooked		 = 1,
} Flags;

// This class is exported from the ThreadSpy.dll
typedef struct
{
	char		szDLLName[MAX_PATH];	// The DLL name
	char		szFuncName[MAX_PATH];	// The function name
	void *		pNewFunc;				// The new function pointer
	void *		pPrevFunc;				// The previous function pointer
	Flags		flags;					// The flags (hooked or not, etc...)
} HookStruct;

FARPROC WINAPI GetTrueProcAddress(HMODULE hMod, LPCSTR lpProcName);

void HookAllModules(HookStruct array[], const int size);
void HookFunctions(HookStruct array[], const int size);
void UnhookFunctions(HookStruct array[], const int size);

void * FindFunction(LPCSTR lpName);

THREADSPY_API int fnThreadSpy(void);

static HHOOK hhook;
static HWND hwnd;
static HWND fehwnd;
static float xd = 0;
static float yd = 0;



