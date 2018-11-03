#pragma once
#if !defined( WH_UUID_H_HEADER_GUARD_6d45b261_d0b1_4a57_a092_6c897af672cf ) 
#define WH_UUID_H_HEADER_GUARD_6d45b261_d0b1_4a57_a092_6c897af672cf 1

#include <rpc.h>

#include <utility>
#include <string>
#include <cassert>

namespace wh{
  class uuidgen{

    template< typename allocator >
    class uuidtostring{
    private :

      template< typename allocator_t ,typename char_t ,typename char_traits_t>
      struct impl{
        inline int operator()( std::basic_string<char_t,char_traits_t,allocator_t> &dst ,
                                UUID& uuid ) const
        {
          RPC_CSTR string_uuid{nullptr};
          {
            struct RPC_CSTR_RAII{
              RPC_CSTR *ptr;
              ~RPC_CSTR_RAII(){ RpcStringFreeA( ptr ); }
            } raii = { &string_uuid };
            if( RPC_S_OK == ::UuidToStringA( &uuid , &string_uuid ) ){
              dst = std::move( std::string{reinterpret_cast<char*>(string_uuid)} ) ;
            }else{
              return 1;
            }
          }
          assert( nullptr == string_uuid );
          return 0;
        }
      };

      template< typename allocator_t >
      struct impl<allocator_t, wchar_t ,std::char_traits<wchar_t>  >{
        inline int operator()( std::basic_string<wchar_t,std::char_traits<wchar_t>,allocator_t> &dst ,
                                UUID& uuid ) const
        {
          RPC_WSTR string_uuid{nullptr};
          {
            struct RPC_WSTR_RAII{
              RPC_WSTR *ptr;
              ~RPC_WSTR_RAII(){ RpcStringFreeW( ptr ); }
            } raii = { &string_uuid };
            if( RPC_S_OK == ::UuidToStringW( &uuid , &string_uuid ) ){
              dst = std::move( std::wstring{reinterpret_cast<wchar_t*>(string_uuid)} ) ;
            }else{
              return 1;
            }
          }
          assert( nullptr == string_uuid );
          return 0;
        }
      };
    public:
      template< typename char_t >
      inline int operator()( std::basic_string<char_t,std::char_traits<char_t>,allocator> &dst , UUID& uuid) const
      {
        return impl<allocator,char_t,std::char_traits<char_t>>{}( dst , uuid );
      }
    };

  public:
    template< typename char_t , typename char_traits , typename allocator >
    inline int operator()( std::basic_string<char_t,char_traits,allocator> &dst ) const 
    {
      UUID uuid = {0};
      switch( ::UuidCreate( &uuid ) ){
      case RPC_S_OK :
      case RPC_S_UUID_LOCAL_ONLY:
        return uuidtostring<allocator>{}( dst , uuid );
      default:
        return 1;
      }
    }
  };
};

#pragma comment( lib , "Rpcrt4.lib" )

#endif /* !defined( WH_UUID_H_HEADER_GUARD_6d45b261_d0b1_4a57_a092_6c897af672cf )  */
