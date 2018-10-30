/**
   WindowsHelperLibrary:whReserved.h
   予約定義ヘッダファイル

   Windows Helper series.
 */
#pragma once
#ifndef WH_RESERVED_H
#define WH_RESERVED_H 1

/* Windows API で NULL を指定されている引数  */
#ifndef RESERVED_ARG_PTR
#if defined(__cplusplus)
#define RESERVED_ARG_PTR (nullptr)
#else /* defined(__cplusplus) */
#define RESERVED_ARG_PTR (NULL)
#endif /* defined(__cplusplus) */
#endif /* RESERVED_ARG_PTR */

/* Windows API で 0 を指定されている引数 */
#ifndef RESERVED_ARG_DWORD 
#if defined(__cplusplus)
#define RESERVED_ARG_DWORD (static_cast<DWORD>(0))
#else /* defined(__cplusplus) */
#define RESERVED_ARG_DWORD (0)
#endif /* defined(__cplusplus) */
#endif /* RESERVED_ARG_DWORD */

#endif /* WH_RESERVED_H */
