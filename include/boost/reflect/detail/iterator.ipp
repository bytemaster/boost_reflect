#ifndef _BOOST_REFLECT_ITERATOR_IPP_
#define _BOOST_REFLECT_ITERATOR_IPP_
#include <boost/reflect/iterator.hpp>
#include <boost/reflect/detail/iterator_impl.hpp>

namespace boost { namespace reflect {

    std::string iterator::key()const  { return itr->key();  }
    value_ref   iterator::value()const { return itr->value(); }

    // 0 == end
    iterator::iterator( detail::iterator_impl_base* i )
    :itr(i){}

    iterator::iterator( const iterator& i )
    :itr(i.itr->clone()){}

    iterator::iterator( iterator&& i )
    :itr(i.itr) { i.itr = 0; }

    iterator::~iterator(){ delete itr; }

    iterator& iterator::operator=( const iterator& i ) {
      if( this != &i ) {
        delete itr;
        itr = i.itr->clone();
      }
      return *this;
    }
    
    iterator& iterator::operator++(int) { itr->next(); return *this; }
    iterator& iterator::operator++()    { itr->next(); return *this; }

    bool iterator::operator == ( const iterator& i )const {
      if( itr == i.itr ) return true;
      if( itr == 0 ) return i.itr->equals(itr);
      return itr->equals(i.itr); 
    }


    /**
     *
     *
     *    const_iterator 
     *
     *
     */
    inline const_iterator::const_iterator( const const_iterator& i )
    :itr(i.itr->const_clone()){}
    inline const_iterator::const_iterator( const iterator& i )
    :itr(i.itr->clone()){}

    inline const_iterator::const_iterator( const_iterator&& i )
    :itr(i.itr) { i.itr = 0; }
    inline const_iterator::const_iterator( detail::const_iterator_impl_base* b )
    :itr( b ){}

    inline const_iterator::~const_iterator(){ delete itr; }
    inline std::string const_iterator::key()const  { return itr->key();  }
    inline value_cref  const_iterator::value()const { return itr->const_value(); }
    
    inline const_iterator& const_iterator::operator++(int) { itr->next(); return *this; }
    inline const_iterator& const_iterator::operator++()    { itr->next(); return *this; }

    inline bool const_iterator::operator == ( const const_iterator& i )const {
      if( itr == i.itr ) return true;
      if( itr == 0 ) return i.itr->equals(itr);
      return itr->equals(i.itr); 
    }
    inline bool const_iterator::operator != ( const const_iterator& i )const {
      return !(*this == i);
    }

} }
#endif // _BOOST_REFLECT_ITERATOR_IPP_
