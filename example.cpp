#include <tchar.h>
#include <Windows.h>
#include <commctrl.h>
#include <wrl.h>
#include <memory.h>

#include <iostream>
#include <locale>
#include <array>
#include <thread>

#include <cassert>
#include <cstdint>

#include "agano.h"
#include "whReserved.h"
#include "whDllFunction.hxx"

#pragma comment( lib , "kernel32.lib" )
#pragma comment( lib , "advapi32.lib" )
#pragma comment( lib , "ole32.lib" )
#pragma comment( lib , "user32.lib" )
#pragma comment( lib , "gdi32.lib" )
#pragma comment( lib , "Comctl32.lib" )
#pragma comment( lib , "DbgHelp.lib" )

#pragma comment( linker , "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0'    \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#if !defined( VERIFY )
#if defined( NDEBUG )
#define VERIFY( exp ) (void)((exp))
#else /* defined( NDEBUG ) */
#define VERIFY( exp ) assert((exp))
#endif /* defined( NDEBUG ) */
#endif /* !defined( VERIFY ) */

struct MainThreadArgument{
  int result;
};

static int mainThread(MainThreadArgument *ptr){
  assert( ptr );
  using agano::EditRegion;

  EditRegion el{};
  el << L'あ' << L'い' << L'う' << L'え' << L'お';
  el.previous();
  el.previous();
  el << L'1' << L'2';
  el << L'\n' ;
  el.previous();
  el << L"メッセージ本文" ;
  el.following();
  el << L"これは" ;
  while( el.previous() );
  el << L"先頭\n" ;

  agano::do_check();
  
  MessageBox( NULL,static_cast<std::wstring>( el ).c_str()  , TEXT("メッセージキャプション" ), MB_OK );
  return (ptr->result = EXIT_SUCCESS);
}

int main(int,char*[])
{
  /**********************************
   // まず、ヒープ破壊の検出で直ぐ死ぬように設定
   ***********************************/
  // HeapSetInformation() は Kernel32.dll 
  { // プロセスヒープの破壊をチェック Kernel32.dll依存だが、これは KnownDLL
    HANDLE processHeapHandle = ::GetProcessHeap(); // こちらの関数は、HANDLE が戻り値
    VERIFY( processHeapHandle != NULL );
    if( processHeapHandle ){
      VERIFY( HeapSetInformation(processHeapHandle, HeapEnableTerminationOnCorruption, NULL, 0) );
    }
#if defined(_MSC_VER)
   // MSVCRT のヒープに対しても、破壊をチェックするように設定
    // MSVCRT の _get_heap_handle() は intptr_t を返す
    HANDLE crtHeapHandle = reinterpret_cast<HANDLE>( _get_heap_handle()); 
    VERIFY( crtHeapHandle != NULL );
    if( crtHeapHandle && processHeapHandle != crtHeapHandle ){
      VERIFY( HeapSetInformation(crtHeapHandle, HeapEnableTerminationOnCorruption, NULL, 0) );

      // 追加でLow-Fragmentation-Heap を使用するように指示する。
      ULONG heapInformaiton(2L); // Low Fragmention Heap を指示
      VERIFY( HeapSetInformation(crtHeapHandle, HeapCompatibilityInformation, 
                                 &heapInformaiton, sizeof( heapInformaiton)));

    }
#endif /* defined( _MSC_VER ) */
  }

  /***************************************************
  DLL の読み込み先から、カレントワーキングディレクトリの排除 
  ***************************************************/
  if(! wh::LibraryPathExcludeCurrentWorkingDirectory() ){
    MessageBox(NULL,_T("SetDllDirectory(\"\") に失敗しました。\n") 
               _T("カレントディレクトリがDLLのサーチパスに含まれるセキュリティ上のリスクがあります。\n")
               _T("KB2533623 の適用を考慮してください。" ),
               _T("警告"), MB_OK );
  }
  
  std::locale::global( std::locale{""} );
  // ここまできたら、CRT の設定開始
  std::locale::global( std::locale("") ); /* ローケルの設定 */
  VERIFY( -1 != _set_error_mode(_OUT_TO_MSGBOX) ); 
  
  HRESULT const hr = CoInitializeEx( NULL , COINIT_MULTITHREADED );
  assert( S_OK == hr );
  if( S_OK != hr ){
    return 3;
  }else{
    struct CoUninit{
      ~CoUninit(){
        CoUninitialize();
      }
    };
    CoUninit coUninit{};

    { // コモンコントロールの DLL 群を読み込む 
      INITCOMMONCONTROLSEX init_commonctrls =
        { sizeof( INITCOMMONCONTROLSEX ),
          ICC_WIN95_CLASSES };
      if( ! InitCommonControlsEx(&init_commonctrls) ){
        return 3;
      }
    }

    HWND debugWindow{ NULL }; // debug コンソールウィンドウ
    (void)(debugWindow);
    
    if( debugWindow ){
      MainThreadArgument mainThreadArgument{};
      std::thread main_thread{ [](MainThreadArgument* arg)->int{
                                 HRESULT const hr = CoInitializeEx( NULL , COINIT_APARTMENTTHREADED );
                                 assert( S_OK == hr );
                                 if( S_OK == hr ){
                                   CoUninit coUninit{};
                                   return mainThread(arg);
                                 }
                                 return 3;
                               } , &mainThreadArgument };
      HANDLE main_thread_handle{ main_thread.native_handle() };
      if( main_thread_handle ){
        for(;;){
          std::array<HANDLE,1> handles = { main_thread_handle };
          
          static_assert( ((std::tuple_size<decltype(handles)>::value) < (MAXIMUM_WAIT_OBJECTS-1)),
                         "((std::tuple_size<decltype(handles)>::value) < (MAXIMUM_WAIT_OBJECTS-1))" );
          
          DWORD const dw = MsgWaitForMultipleObjects( std::tuple_size<decltype(handles)>::value ,  handles.data(),
                                                      FALSE , INFINITE , QS_ALLINPUT );
          switch( dw ){
          case WAIT_OBJECT_0:
            main_thread.join();
            goto end_of_message_loop;
          case (WAIT_OBJECT_0 + std::tuple_size<decltype( handles )>::value) :
            // TODO : message pump;
            continue;
          case (WAIT_TIMEOUT):
            continue;
          case 0xFFFFFFFF: // error 
            goto end_of_message_loop;
          default:
            goto end_of_message_loop;
          }
        }
      }
    end_of_message_loop:
      return mainThreadArgument.result;
    }
    return 3;
  }
}
