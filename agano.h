#pragma once

#include <deque>
#include <vector>
#include <mutex>
#include <algorithm>

namespace agano{

  struct wc_attribute{
  };

  inline bool operator==( const wc_attribute& lhv , const wc_attribute& rhv )
  {
    (void)(lhv);
    (void)(rhv);
    return true;
  }
  inline bool operator!=( const wc_attribute& lhv , const wc_attribute& rhv )
  {
    return !( lhv == rhv ) ;
  }
  
  struct wc_t{
    wchar_t c;
    wc_attribute attr;
    wc_t() = default;
    explicit wc_t( const wc_t& ) = default;
    explicit wc_t( wc_t&& ) = default;
    explicit wc_t(const wchar_t &c )
      : c(c) , attr()
    {}
    wc_t(const wchar_t &c , const wc_attribute& attr )
      : c( c ), attr( attr )
    {}
    wc_t& operator=( const wc_t & ) = default;
  };

  struct TextFragment{
    std::wstring text;
    bool terminate;
    wc_attribute attr;

    TextFragment()
      :text{},terminate{false},attr{}
    {
    }
    
    TextFragment(const std::wstring& text , const wc_attribute& attr )
      :text{text},terminate{false},attr{attr}
    {
    }
    
    explicit TextFragment( const TextFragment& rhv )
      : text( rhv.text ), terminate( rhv.terminate ),attr( rhv.attr )
    {
    }
    
    explicit TextFragment( TextFragment&& rhv )
      : text( std::move( rhv.text ) ) ,terminate(rhv.terminate), attr( std::move( rhv.attr ) )
    {
    }
    
    inline TextFragment& operator =( TextFragment&& rhv )
    {
      if( this != &rhv ){
        std::swap( text , rhv.text );
        std::swap( terminate , rhv.terminate );
        std::swap( attr , rhv.attr );
      }
      return *this;
    }
    
    inline TextFragment& operator=(const TextFragment& rhv )
    {
      if( this != &rhv ){
        text = rhv.text;
        terminate = rhv.terminate;
        attr = rhv.attr;
      }
      return *this;
    }
    
    inline operator bool() const
    {
      return !(text.empty());
    }
  };

  struct EditRegion{
    std::mutex mutex;
    std::deque<wc_t> upper_half;
    std::deque<wc_t> lower_half;
    using lock_t = std::unique_lock<decltype( mutex )>;

    inline std::size_t pos() const {
      return upper_half.size();
    }

    inline bool following() {
      lock_t lock{ mutex };
      if( lower_half.empty() ){
        return false;
      }
      upper_half.push_back( std::move( lower_half.front() ) );
      lower_half.pop_front();
      return true;
    }

    inline bool previous(){
      lock_t lock{ mutex };
      if( upper_half.empty() ){
        return false;
      }
      lower_half.push_front( std::move( upper_half.back() ));
      upper_half.pop_back();
      return true;
    }
    
    inline void put( const wchar_t c )
    {
      lock_t lock{ mutex };
      if( upper_half.empty() ){
        upper_half.emplace_back( c );
      }else{
        upper_half.emplace_back( c , upper_half.back().attr );
      }
      return;
    }

    inline void put( const wchar_t *str )
    {
      if( str ){
        lock_t lock{ mutex };
        for( ; L'\0' != *str ; ++str ){
          if( upper_half.empty() ){
            upper_half.emplace_back( *str );
          }else{
            upper_half.emplace_back( *str , upper_half.back().attr );
          }
        }
      }
    }
    inline EditRegion& operator << ( const wchar_t c )
    {
      put(c);
      return *this;
    }
    
    inline EditRegion& operator << ( const wchar_t* str )
    {
      put(str);
      return *this;
    }
    // デバッグ用です
    explicit inline operator std::wstring();
  };

  std::deque<wc_t> to_wc_t( const std::deque<TextFragment>& fragments );
  std::deque<wc_t> to_wc_t( const TextFragment& fragment );
  
  // 後でagano.cpp へ移動させる。
  inline std::vector<TextFragment> pack(EditRegion& edit)
  {
    EditRegion::lock_t lock{ edit.mutex };
    std::vector<TextFragment> packed{};

    {
      TextFragment fragment{};
      auto predicate =
        [&]( const wc_t &wc ){
          if( fragment.attr != wc.attr ){
            if( static_cast<bool>( fragment ) ){
              packed.emplace_back( std::move(fragment) );
              fragment = TextFragment{std::wstring{},wc.attr};
            }
          }
          fragment.text.push_back( wc.c );
          if( L'\n' == wc.c ){
            if( static_cast<bool>( fragment ) ){
              fragment.terminate = true;
              packed.emplace_back( std::move(fragment) );
              fragment = TextFragment{std::wstring{},wc.attr};
            }
          }
        };
      std::for_each( std::begin( edit.upper_half ) , std::end( edit.upper_half ), predicate );
      std::for_each( std::begin( edit.lower_half ) , std::end( edit.lower_half ) ,predicate );
      if( static_cast<bool>( fragment ) ){
        packed.push_back( fragment );
      }
    }
    return packed;
  };

  EditRegion::operator std::wstring()
  {
    std::vector<TextFragment> fragments = pack( *this );
    std::wstring buf{};
    for_each( std::begin( fragments ) , std::end( fragments ) ,
              [&]( const TextFragment& frag ){
                buf.append( frag.text );
              });
    return buf;
  }
  
  struct TextBuffer{
    std::deque<TextFragment> low_harf;
    std::deque<TextFragment> high_harf;
    EditRegion current;
    inline void put(const wchar_t c )
    {
      current.put(c);
    }
    inline void put(const wchar_t *str )
    {
      assert( str );
      if( str ){
        for( ; L'\0' != *str; ++str ){
          put( *str );
        }
      }
    }
  };

  // for debug 
  void do_check();
  
};
