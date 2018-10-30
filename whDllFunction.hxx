/**
   whDllFunction.hxx 
   明示的リンクを処理するテンプレートクラス
   
   Windows Helper series.
   
 */
#pragma once
#include "whReserved.h"

#ifndef WH_DLLFUNCTION_HXX
#define WH_DLLFUNCTION_HXX

/* Windows 7, Windows Server 2008 R2, Windows Vista, and Windows Server 2008 の KB2533623 適用すると有効になるフラグ */
/* これは SDK version 7.1A には入っていないので、手前で定義をかける */
#ifndef LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR
#define LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR (0x00000100)
#endif /* LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR */

#ifndef LOAD_LIBRARY_SEARCH_APPLICATION_DIR
#define LOAD_LIBRARY_SEARCH_APPLICATION_DIR (0x00000200)
#endif /* LOAD_LIBRARY_SEARCH_APPLICATION_DIR */

#ifndef LOAD_LIBRARY_SEARCH_USER_DIRS
#define LOAD_LIBRARY_SEARCH_USER_DIRS (0x00000400)
#endif /* LOAD_LIBRARY_SEARCH_USER_DIRS */

#ifndef LOAD_LIBRARY_SEARCH_SYSTEM32
#define LOAD_LIBRARY_SEARCH_SYSTEM32 (0x00000800)
#endif /* LOAD_LIBRARY_SEARCH_SYSTEM32 */

#ifndef LOAD_LIBRARY_SEARCH_DEFAULT_DIRS
#define LOAD_LIBRARY_SEARCH_DEFAULT_DIRS (0x00001000)
#endif /* LOAD_LIBRARY_SEARCH_DEFAULT_DIRS */

namespace wh{
  template <typename functionType,DWORD loadExFlag = LOAD_LIBRARY_SEARCH_SYSTEM32,int useLoadLibrary = 1>
  class ScopedDllFunction{
  public:
    typedef functionType *FunctionPtrType;
    enum{
      LOADLIBRARYEX_DWFLAG = loadExFlag,
    };
    enum{
      USELOADLIBRARY = useLoadLibrary,
    };
  private:
    HMODULE module;
    FunctionPtrType function_ptr;
  private:
    template<int loadType> inline HMODULE getHModule( const TCHAR * dll_name ){
      return ::LoadLibraryEx( dll_name , RESERVED_ARG_DWORD , LOADLIBRARYEX_DWFLAG );
    }
    template<int loadType> inline BOOL releaseModule(){
      BOOL retval = ::FreeLibrary( module );
      module = NULL;
      return retval;
    }
    /* テンプレートの特殊化を行い GetModuleHandle と なにもしない を使うようにする */
    template<> inline HMODULE getHModule<0>( const TCHAR *dll_name ){
      return (module = ::GetModuleHandle( dll_name ));
    }
    template<> inline BOOL releaseModule<0>(){
      module = NULL;
      return TRUE;
    }
    
  public:
    // これ本当はコンストラクタで例外飛ばすようにしたほうがいいと思うんだよね。
    ScopedDllFunction( const TCHAR *const dll_name , const char* const function_name):module(NULL),function_ptr( nullptr ){
      module = getHModule<USELOADLIBRARY>(dll_name);
      if( module != NULL ){
        function_ptr = reinterpret_cast<FunctionPtrType>( GetProcAddress( module , function_name ) );
      }
      return;
    }
    
    ~ScopedDllFunction(){
      releaseModule<USELOADLIBRARY>();
      return;
    }
    
    DWORD GetModuleFileName( LPTSTR lpFilename , DWORD nSize ){
      if( module != NULL ){
        return ::GetModuleFileName( module , lpFilename , nSize );
      }
      return 0;
    }
    
    operator FunctionPtrType () const {
      return function_ptr;
    }
  private:
    /* コピーコンストラクタの禁止 */
    ScopedDllFunction( const ScopedDllFunction &);
    /* コピー禁止 */
    const ScopedDllFunction& operator = ( const ScopedDllFunction &);
  };

  /* サーチパスからカレントワーキングディレクトリを排除する */
  inline BOOL LibraryPathExcludeCurrentWorkingDirectory(){
    ScopedDllFunction< decltype(SetDllDirectoryW) , 0 , 0 > setDllDirectoryW( _T("kernel32.dll"), "SetDllDirectoryW");
    if( setDllDirectoryW ){
      if( 0 != setDllDirectoryW(L"") ){ 
        return TRUE;
      }
    }
    return FALSE;
  };

};

#endif /* WH_DLLFUNCTION_HXX */
