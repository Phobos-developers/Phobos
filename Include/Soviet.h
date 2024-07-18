
#if !defined(_SOVIET_SUPP_H_00568000_INCLUDED_)
#define _SOVIET_SUPP_H_00568000_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <windows.h>

#pragma pack(push)
#pragma pack(1)

__declspec(align(16)) struct stSoviet
{
	UCHAR Header[0x1000];
	UCHAR Text[0xA9000];
	UCHAR Rdata[0x26000];
	UCHAR Data[0x8000];
	UCHAR Syhks00[0x4000];
	UCHAR Patch[0x1000];
	UCHAR Rsrc[0x484000];
	UCHAR Reloc[0x7000];
};

#pragma pack(pop)

__declspec(align(16)) extern stSoviet Soviet;

/* Data for initialization. */
extern UCHAR Soviet_InitData[0xD13E];

extern HMODULE g_hSoviet;

void* __stdcall Soviet_RVA(DWORD rvaAddr);

#define Soviet_VA(vaAddr) Soviet_RVA((vaAddr) - 0x10000000)


#pragma once
#include "targetver.h"
#include <stdio.h>
#include <tchar.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit
#include <atlbase.h>
#include <atlstr.h>
#ifndef ASSERT
#include <assert.h>
#define ASSERT(x) assert(x)
#endif

#pragma once
#ifndef WINVER                          // Specifies that the minimum required platform is Windows Vista.
#define WINVER 0x0600           // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT            // Specifies that the minimum required platform is Windows Vista.
#define _WIN32_WINNT 0x0600     // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINDOWS          // Specifies that the minimum required platform is Windows 98.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE                       // Specifies that the minimum required platform is Internet Explorer 7.0.
#define _WIN32_IE 0x0700        // Change this to the appropriate value to target other versions of IE.
#endif


#endif // !defined(_SOVIET_SUPP_H_00568000_INCLUDED_)