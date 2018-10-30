#include "winapi.h"

#include <iostream>
#include <sstream>
#include <string>
#include <deque>
#include <utility>
#include <algorithm>

#include <cstdint>
#include <cassert>

#include "agano.h"
#include <iterator>
#include <functional>
#include <type_traits>

namespace agano{
  static_assert( std::is_trivial<wc_t>::value , "wc_t is trivial");
  static_assert( std::is_standard_layout<wc_t>::value , "wc_t is standard layout" );

  /**
     Textfragment を wc_t に分解して、container に詰める
   */
  template< typename allocator_t,
            template<typename,typename>
            class Container_Type>
  static inline Container_Type<wc_t,allocator_t>&
  copy_from( Container_Type<wc_t,allocator_t>& dst ,const TextFragment& fragment )
  {
    using bind_member_t =std::remove_reference<decltype(dst)>::type;
    std::for_each( std::begin( fragment.text ), std::end( fragment.text ) ,
                   std::bind( &(bind_member_t::emplace_back<const wchar_t&,const wc_attribute&>) ,
                              &dst , std::placeholders::_1 , fragment.attr) );
    return dst;
  }

  /**
     コンテナの中の [first,last) のTextFragment を wc_t に分解して、container に詰める
   */
  template< typename allocator_t,
            template<typename,typename> class Container_Type ,
            typename InputIterator >
  static inline Container_Type<wc_t,allocator_t>&
  copy_from( Container_Type<wc_t,allocator_t>&dst , InputIterator first , InputIterator last )
  {
    std::for_each( first , last ,
                   [&]( std::add_const<decltype(*first )>::type &fragment ){
                     copy_from( dst , fragment );
                   } );
    return dst;
  }

  std::deque<wc_t> to_wc_t( const std::deque<TextFragment>& fragments )
  {
    std::deque<wc_t> deque;
    return copy_from( deque , std::begin( fragments ) , std::end( fragments ) );
  }
  
  std::deque<wc_t> to_wc_t( const TextFragment& fragment )
  {
    std::deque<wc_t> deque{};
    return copy_from( deque , fragment );
  }

  std::deque<wc_t> clip_paragraph( std::deque<TextFragment>& fragments )
  {
    std::deque<wc_t> buf;
    auto rbegin = fragments.rbegin();


    if( rbegin != fragments.rend() && (rbegin->terminate) ){
        std::advance( rbegin,1 );
    }
    
    auto ite = std::find_if( rbegin , fragments.rend() ,
                              []( const TextFragment& fragment ){
                                return fragment.terminate;
                              } );
     if( fragments.rend() == ite ){
       copy_from( buf , std::begin( fragments ), std::end( fragments ) );
       fragments.clear();
     }else{
       if( fragments.rbegin() == ite ){
         std::advance( rbegin , -1 );
       }
       copy_from( buf ,
                  std::make_reverse_iterator( ite ),
                  std::make_reverse_iterator( fragments.rbegin() ) );
       {
         auto pos = std::cbegin(fragments);
         std::advance( pos , ( fragments.size() - std::distance( fragments.rbegin(), ite ) ) );
         fragments.erase( pos );
       }
     }
    return buf;
  }

  void do_check()
  {
    ::OutputDebugString( TEXT("do_check()\n") );
    std::deque<TextFragment> deq{};
    {
      auto deq_wc = clip_paragraph( deq );
      std::cout << deq_wc.size() << std::endl;
    }
    {
      {
        TextFragment one = {L"１行目\n", wc_attribute{} };
        one.terminate = true;
        deq.push_back( one );
      }
      {
        TextFragment twoo = {L"２行目前半" , wc_attribute{} };
        deq.push_back( twoo );
      }
      {
        TextFragment two = {L"２行目後半\n", wc_attribute{} };
        two.terminate = true;
        deq.push_back( two );
      }
      for( size_t i = 0; i< 4 ; ++i ){
        auto deq_wc = clip_paragraph( deq );
        std::cout << deq_wc.size() << std::endl;
        std::wstring buf;
        std::for_each( std::begin( deq_wc ), std::end( deq_wc ),
                       [&buf]( const wc_t & c ){
                         buf.push_back( c.c );
                       });
        OutputDebugStringW( buf.c_str() );
      }
      
    }
  }
};

