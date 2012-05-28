#ifndef _MACE_REFLECT_ITERATOR_HPP_
#define _MACE_REFLECT_ITERATOR_HPP_
#include <mace/reflect/value_ref.hpp>

namespace mace { namespace reflect {

namespace detail { 
  class const_iterator_impl_base;
  class iterator_impl_base;
} 
class value_ref;
class value_cref;

class iterator {
  public:
    std::string key()const;
    value_ref   value()const;

    // 0 == end
    iterator( const iterator& i );
    iterator( iterator&& i );
    iterator( detail::iterator_impl_base* p = 0 );
    ~iterator();

    iterator& operator=( const iterator& i );
    
    iterator& operator++(int);
    iterator& operator++();

    bool operator == ( const iterator& i )const;
    bool operator != ( const iterator& i )const;

  private:
    friend class const_iterator;
    detail::iterator_impl_base* itr;
};

class const_iterator {
  public:
    std::string key()const;
    value_cref  value()const;

    const_iterator( detail::const_iterator_impl_base* b = 0 );
    const_iterator( const iterator& i );
    const_iterator( const const_iterator& i );
    const_iterator( const_iterator&& i );
    ~const_iterator();
    
    const_iterator& operator++(int);
    const_iterator& operator++();

    bool operator == ( const const_iterator& i )const;
    bool operator != ( const const_iterator& i )const; 

  private:
    friend class iterator;
    detail::const_iterator_impl_base* itr;
};

} } //mace::reflect

#endif
