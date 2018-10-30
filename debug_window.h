#pragma once
#if !defined( DEBUG_WINDOW_H_HEADER_GUARD30fb1f5c_3d07_498c_bcbd_13bc46b7886b )
#define DEBUG_WINDOW_H_HEADER_GUARD30fb1f5c_3d07_498c_bcbd_13bc46b7886b 1

#include <atomic>

#if defined( __cplusplus )
#define EXTERN_C extern "C" 
#else /* defined( __cplusplus ) */
#define EXTERN_C 
#endif /* defined( __cplusplus ) */

/**
   デバッグウィンドウは、アプリケーション内で一つだけである。
   デバッグウィンドウのハンドルを取得する。デバッグウィンドウが無い場合はNULL が戻る
 */
EXTERN_C HWND getDebugWindowHandle();

#if defined( __cplusplus )
void bootstrap_debug_console( std::thread& application_thread );
#endif /* defined( __cplusplus )  */

#endif /* DEBUG_WINDOW_H_HEADER_GUARD30fb1f5c_3d07_498c_bcbd_13bc46b7886b */
