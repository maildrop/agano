#pragma once
#if !defined( WINAPI_H_HEADER_GUARD_8B594DCD_238B_4DC4_AE71_F322674CF51A )
#define WINAPI_H_HEADER_GUARD_8B594DCD_238B_4DC4_AE71_F322674CF51A 1

/************************************************
  winapi.h - for precompiled header files.


  author TOGURO Mikito , mit@shalab.net
*************************************************/

#ifndef NOMINMAX
#define NOMINMAX 1
#endif /* NOMINMAX */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif /* WIN32_LEAN_AND_MEAN */

#include <tchar.h>
#include <Windows.h>
#include <wrl.h>
#include <commctrl.h>

#if defined( __cplusplus )
#include <cassert>
#else /* defined( __cplusplus ) */
#include <assert.h>
#endif /* defined( __cplusplus ) */

/* linker and manifest  */
#pragma comment( lib , "kernel32.lib" )
#pragma comment( lib , "advapi32.lib" )
#pragma comment( lib , "ole32.lib" )
#pragma comment( lib , "user32.lib" )
#pragma comment( lib , "gdi32.lib" )
#pragma comment( lib , "Comctl32.lib" )
#pragma comment( lib , "DbgHelp.lib" )

#pragma comment( linker , "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0'              \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#if !defined( VERIFY )
#if defined( NDEBUG )
#define VERIFY( exp ) (void)((exp))
#else /* defined( NDEBUG ) */
#define VERIFY( exp ) assert((exp))
#endif /* defined( NDEBUG ) */
#endif /* !defined( VERIFY ) */

#endif /* !defined( WINAPI_H_HEADER_GUARD_8B594DCD_238B_4DC4_AE71_F322674CF51A )*/
