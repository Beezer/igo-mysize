#include "stdafx.h"
#include "iGO-MySize-DLL.h"
#include <windows.h>
#include <stdio.h>



// The true proc address that will not be overwriten
FARPROC WINAPI GetTrueProcAddress(HMODULE hMod, LPCSTR lpProcName)
{
	return ::GetProcAddress(hMod, lpProcName);
}


FARPROC WINAPI MyGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
	// Get the original address of the function
	//
	FARPROC pfn;
	pfn = (FARPROC)FindFunction(lpProcName);
	if (pfn == NULL)
		pfn = GetTrueProcAddress(hModule, lpProcName);

	// Return the function
	return pfn;
}

BOOL	WINAPI MyBitBlt(
  HDC hdcDest, // handle to destination DC
  int nXDest,  // x-coord of destination upper-left corner
  int nYDest,  // y-coord of destination upper-left corner
  int nWidth,  // width of destination rectangle
  int nHeight, // height of destination rectangle
  HDC hdcSrc,  // handle to source DC
  int nXSrc,   // x-coordinate of source upper-left corner
  int nYSrc,   // y-coordinate of source upper-left corner
  DWORD dwRop  // raster operation code
)

{ 

	// Find our window so we can retrieve our scaling factor
	if (!hwnd) {
		hwnd = FindWindow("iGO8.class", "iGO8");
	}

	if (hwnd) {
		if (!xd && !yd) {
			LONG userData = GetWindowLong(hwnd, GWL_USERDATA);

			 // Original Sizes
			 xd = LOWORD(userData);
			 yd = HIWORD(userData);
		}

		if (xd > 0 && yd > 0) {

#ifdef FASTRESIZE
			SetStretchBltMode(hdcDest, COLORONCOLOR);
#else
			SetStretchBltMode(hdcDest, HALFTONE);
#endif
			return ::StretchBlt(
					hdcDest, 
					int(((float)nXDest / 100) * xd), 
					int(((float)nYDest / 100) * yd), 
					int(((float)nWidth / 100) * xd), 
					int(((float)nHeight / 100) * yd), 
					hdcSrc, 
					nXSrc, 
					nYSrc, 
					nWidth, 
					nHeight, 
					dwRop
					);
		} else {
			return ::BitBlt(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);
		}


	} else {
		return ::BitBlt(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);

	}
}


LRESULT WINAPI MessageProc(int nCode, WPARAM wParam, LPARAM lParam) 
{ 
	WORD x, y, newY, newX;

	MSG* pData=reinterpret_cast<MSG*>(lParam);
	switch (pData->message)
    { 

		// Capture & manipulate mouse events
		case WM_MOUSEMOVE:
		case WM_LBUTTONUP:
		case WM_LBUTTONDOWN:
				if (xd > 0 && yd > 0) {
					
					// Scale the coords
					x = LOWORD(pData->lParam);
					y = HIWORD(pData->lParam);

					// find what x is 80% of
					newY = int((y * 100.0) / yd);
					newX = int((x * 100.0) / xd);

					pData->lParam = MAKELONG(newX,newY);
				}
			break; 
	}


    return CallNextHookEx(hhook, nCode, 
        wParam, lParam); 
} 
