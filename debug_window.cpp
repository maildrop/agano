#include "winapi.h"

#include "debug_window.h"
#include <array>

namespace debug {
  enum{
       RESERVED = 0,
  };
  
  namespace implement{
    static std::atomic<HWND>& getDebugWindowHandleImpl();
  };
  
  struct DebugLog_t{
    std::wstring log_message;
    HANDLE module_handle;
    const wchar_t *source_file;
    int32_t line;
  };
  
  namespace log {

    DebugLog_t* trace_gen( HMODULE module_handle,
                           const wchar_t* source_file , int32_t line ,
                           const std::wstring& log_message ){
      DebugLog_t* log = new DebugLog_t{};
      log->module_handle = module_handle;
      log->log_message = log_message;
      log->source_file = source_file;
      log->line = line;
      return log;
    }
    
    void trace( uint32_t log_level, DebugLog_t* debuglog ){
      HWND hWnd = getDebugWindowHandle() ;
      if( hWnd ){
        PostMessage( hWnd , PWM_DEBUG_LOG ,
                     MAKELONG( log_level , RESERVED ),
                     reinterpret_cast<LPARAM>( debuglog ));
      }
      delete debuglog;
    }
  };
  
};
#define TRACEER( log_level , log_message ) do{                          \
    debug::log::trace( debug::log::trace_gen( log_level , __FILE__ , __LINE__ , log_message  ) ) ; \
  }while( false )



/**
   デバッグウィンドウの参照を返す
*/
static std::atomic<HWND>& debug::implement::getDebugWindowHandleImpl()
{
  static std::atomic<HWND> debugWindowHandle{nullptr};
  return debugWindowHandle;
}

HWND getDebugWindowHandle()
{
  HWND hWnd = debug::implement::getDebugWindowHandleImpl().load();
  return hWnd;
}

static LRESULT debugWindowProc( HWND hWnd , UINT msg , WPARAM wParam , LPARAM lParam )
{
  return ::DefWindowProc( hWnd , msg , wParam , lParam );
}

struct WindowClassAtomRAII{
  struct Init{
    ATOM operator()(HINSTANCE hInstance) const {
      const WNDCLASSEX wcx =
        { sizeof( WNDCLASSEX ),                   // cbSize 
          CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW ,  // style
          debugWindowProc,                        // lpfnWndProc
          0,                                      // cbClsExtra
          0,                                      // cbWndExtra
          hInstance,                              // hInstance
          NULL,                                   // hIcon
          ::LoadCursor( NULL , IDC_ARROW ) ,      // hCursor
          (HBRUSH) GetStockObject( WHITE_BRUSH ), // hbrBackground
          NULL,                                   // lpszMenuName
          TEXT("whDebugConsole" ),                // lpszClassName 
          NULL                                    // hIconSm 
        };
      return ::RegisterClassEx( &wcx );
    }
  };
  
  const ATOM atom;
  const HINSTANCE hInstance;
  WindowClassAtomRAII() = delete;
  WindowClassAtomRAII(WindowClassAtomRAII&) = delete;
  WindowClassAtomRAII(WindowClassAtomRAII&&) = delete;
  explicit WindowClassAtomRAII(HINSTANCE hInstance)
    :atom( Init{}(hInstance) ), hInstance( hInstance )
  {}

  WindowClassAtomRAII& operator=( WindowClassAtomRAII& ) = delete;
  WindowClassAtomRAII&& operator=( WindowClassAtomRAII&& ) = delete;

  explicit operator ATOM() const {
    return atom;
  }

  explicit operator LPCTSTR() const {
    return reinterpret_cast<LPCTSTR>( atom ) ;
  }
  
  ~WindowClassAtomRAII(){
    UnregisterClass( (LPCWSTR)(atom) , hInstance);
  }
};

void bootstrap_debug_console( std::thread& application_thread )
{
  HINSTANCE hInstance = GetModuleHandle( NULL );
  WindowClassAtomRAII debugWindowAtom{ hInstance};

  static_cast<ATOM>( debugWindowAtom );
  HWND debugWindowHandle =
    CreateWindowEx( WS_EX_OVERLAPPEDWINDOW,
                    static_cast<LPCTSTR>( debugWindowAtom ),
                    TEXT("DebugConsole") ,
                    WS_VISIBLE | WS_OVERLAPPEDWINDOW ,
                    CW_USEDEFAULT , CW_USEDEFAULT ,
                    CW_USEDEFAULT , CW_USEDEFAULT ,
                    NULL ,
                    NULL,
                    hInstance ,
                    nullptr );
  VERIFY( NULL != debugWindowHandle );

  if( debugWindowHandle ){
    debug::implement::getDebugWindowHandleImpl().store( debugWindowHandle );
    HANDLE const app_thread_handle = application_thread.native_handle();
    for( ;; ){
      std::array<HANDLE ,1 > handles = { app_thread_handle };
      static_assert( (std::tuple_size<decltype(handles)>::value) < (MAXIMUM_WAIT_OBJECTS -1 ),
                     "(std::tuple_size<decltype(handles)::value) < (MAXIMUM_WAIT_OBJECTS -1 )" );
      DWORD const dw = MsgWaitForMultipleObjects( std::tuple_size<decltype( handles )>::value , handles.data() ,
                                                  FALSE , INFINITE , QS_ALLINPUT );
      switch( dw ){
      case WAIT_OBJECT_0:
        {
          HWND debugWindow = getDebugWindowHandle();
          if( debugWindow ){
            VERIFY( ::DestroyWindow( debugWindow ) );
            for(;;){
              MSG msg = {0};
              switch( GetMessage( &msg ,NULL , 0 ,0  ) ){
              case -1:
              case 0:
                goto end_of_message_loop;
              default:
                VERIFY(TranslateMessage( &msg ));
                VERIFY(DispatchMessage( &msg ));
                break;
              }
            }
            goto end_of_message_loop;
          }
          goto end_of_message_loop;
        }
        continue;
      case (WAIT_OBJECT_0+std::tuple_size<decltype( handles ) >::value):
        for( MSG msg = {0}; PeekMessage(&msg , NULL , 0 ,0 , PM_REMOVE ) ; msg = {0} ){
          if( WM_QUIT == msg.message ){
            goto end_of_message_loop;
          }
          VERIFY(TranslateMessage( &msg ));
          VERIFY(DispatchMessage( &msg ));
        }
        continue;
      default:
        continue;
      }
    }
  end_of_message_loop:
    debug::implement::getDebugWindowHandleImpl().store( NULL );
  }
  return;
}

