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


#if defined( _WIN32 )

namespace wh{

  /** ナロー文字用のメッセージフォーマット関数 */
  template<DWORD primaryLanguage = LANG_NEUTRAL ,
           DWORD subLanguage = SUBLANG_DEFAULT>
  inline std::string format_messageA( DWORD const lastError ){
    LPVOID lpMsgBuf{nullptr};
    DWORD formatMessageResult = 
      ::FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL,
                        lastError ,
                        MAKELANGID(primaryLanguage, subLanguage ), // 既定の言語
                        (LPSTR) &lpMsgBuf ,
                        0 ,
                        NULL );
    if( formatMessageResult ){
      std::string formatedMessage{ static_cast<const char*>((lpMsgBuf) ? lpMsgBuf : ""),
                                   static_cast<size_t>(formatMessageResult) };
      if( ::LocalFree( lpMsgBuf ) ){
        // LocalFree failed! it maybe heap was broken. check check check!
        assert( !"LocalFree failed." );
      }
      return formatedMessage;
    }else{
      return std::to_string( lastError );
    }
  }

  /** ワイド文字用のメッセージメッセージフォーマット関数 */
  template<DWORD primaryLanguage = LANG_NEUTRAL ,
           DWORD subLanguage = SUBLANG_DEFAULT>
  inline std::string format_messageW( DWORD const lastError ){
    LPVOID lpMsgBuf{nullptr};
    DWORD formatMessageResult = 
      ::FormatMessageW( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL,
                        lastError ,
                        MAKELANGID(primaryLanguage, subLanguage ), // 既定の言語
                        (LPSTR) &lpMsgBuf ,
                        0 ,
                        NULL );
    if(  formatMessageResult ){
      std::wstring formatedMessage{ static_cast<const char*>((lpMsgBuf) ? lpMsgBuf : ""),
                                    static_cast<size_t>(formatMessageResult) };
      if( ::LocalFree( lpMsgBuf )  ){
        // LocalFree failed! it maybe heap was broken. check check check!
        assert( !"LocalFree failed." );
      }
      return formatedMessage;
    }else{
      return std::to_wxstring( lastError );
    }
  }
};

#endif /* defined( _WIN32 ) */


#endif /* DEBUG_WINDOW_H_HEADER_GUARD30fb1f5c_3d07_498c_bcbd_13bc46b7886b */
