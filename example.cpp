#include <tchar.h>
#include <Windows.h>
#include <wrl.h>
#include <memory.h>

#include <iostream>
#include <locale>
#include <thread>

#include <cassert>
#include <cstdint>

#include "agano.h"

#pragma comment( lib , "ole32.lib" )
#pragma comment( lib , "User32.lib")

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
  std::locale::global( std::locale{""} );
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
    main_thread.join();
    return mainThreadArgument.result;
  }
}
