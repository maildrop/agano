#include <tchar.h>
#include <Windows.h>
#include <thread>
#include "debug_window.h"

namespace implement{
  static std::atomic<HWND>& getDebugWindowHandleImpl();
};

void bootstrap_debug_console( std::thread& application_thread )
{
  HANDLE const app_thread_handle = application_thread.native_handle();

  (void)( app_thread_handle );
  
  return;
}

static std::atomic<HWND>& implement::getDebugWindowHandleImpl()
{
  static std::atomic<HWND> debugWindowHandle{nullptr};
  return debugWindowHandle;
}

HWND getDebugWindowHandle()
{
  HWND hWnd = implement::getDebugWindowHandleImpl().load();
  return hWnd;
}
