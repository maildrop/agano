#include "winapi.h"

#include "debug_window.h"

namespace debug {
  enum{
       RESERVED = 0,
  };
  enum{
       PWM_DEBUG_LOG = (WM_APP + 1),
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
#define TRACEER( log_level , log_message ) do{ \
    debug::log::trace( debug::log::trace_gen( log_level , __FILE__ , __LINE__ , log_message  ) ) ; \
  }while( false )


void bootstrap_debug_console( std::thread& application_thread )
{
  HANDLE const app_thread_handle = application_thread.native_handle();

  (void)( app_thread_handle );
  
  return;
}

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
      WNDCLASSEX wcx =
        {
         sizeof( WNDCLASSEX ),
         CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW ,
         debugWindowProc,
         0,
         0,
         hInstance,
         NULL,
         ::LoadCursor( NULL , IDC_ARROW ) , // Cursor 
         (HBRUSH) GetStockObject( WHITE_BRUSH ),
         NULL,
         TEXT("whDebugConsole"),
         NULL 
        };
      return RegisterClassEx( &wcx );
    }
  };
  
  const ATOM atom;
  const HINSTANCE hInstance;
  WindowClassAtomRAII() = delete;
  WindowClassAtomRAII(WindowClassAtomRAII&) = delete;
  WindowClassAtomRAII(WindowClassAtomRAII&&) = delete;
  explicit WindowClassAtomRAII(HINSTANCE hInstance)
    :atom(Init{}(hInstance)),hInstance( hInstance )
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

static WindowClassAtomRAII debugWindowAtom{ GetModuleHandle( NULL ) };

