

#ifndef DOXYGEN
#define CUSTOM_MEMBER_CASES(r, data, i, elem)\
case BOOST_PP_TUPLE_ELEM( 3, 1, elem ):\
   v.accept_member( name, &data::BOOST_PP_TUPLE_ELEM(3, 0, elem), BOOST_PP_STRINGIZE( BOOST_PP_TUPLE_ELEM(3,0,elem) ), BOOST_PP_TUPLE_ELEM(3, 2,elem) );\
   break;

#define CUSTOM_MEMBER_ALL(r, data, i, elem)\
   v.accept_member( name, &data::BOOST_PP_TUPLE_ELEM(3, 0, elem), BOOST_PP_STRINGIZE( BOOST_PP_TUPLE_ELEM(3,0,elem) ), BOOST_PP_TUPLE_ELEM(3, 2,elem) );\

#define MEMBER_CASES(r, data, i, elem)\
case i:\
   v.accept_member( name, &data::elem, BOOST_PP_STRINGIZE( elem ), i ); \
   break;

#define MEMBER_ALL(r, data, i, elem)\
   v.accept_member( name, &data::elem, BOOST_PP_STRINGIZE( elem ), i );

#define INHERITS (baseA)(baseB)

#define ACCEPT_BASE(r, data, i, elem) \
    v.accept_base( *static_cast<elem*>(&name), BOOST_PP_STRINGIZE( elem ), field );


#define BOOST_REFLECT_IMPL( CONST,TYPE, INHERITS, MEMBERS ) \
template<typename Visitor>\
static inline void visit( CONST TYPE& name, Visitor& v, uint32_t field = -1 ) { \
    v.start(name, BOOST_PP_STRINGIZE(TYPE) );\
    BOOST_PP_SEQ_FOR_EACH_I( ACCEPT_BASE, TYPE, INHERITS ) \
    switch( field ) { \
        case -1: \
            BOOST_PP_SEQ_FOR_EACH_I( MEMBER_ALL, TYPE, MEMBERS ) \
            break; \
        BOOST_PP_SEQ_FOR_EACH_I( MEMBER_CASES, TYPE, MEMBERS ) \
        default: \
            v.not_found( name, field );\
    }\
    v.end(name, BOOST_PP_STRINGIZE(TYPE) );\
} \

#define BOOST_REFLECT_CUSTOM_IMPL( CONST,TYPE, INHERITS, MEMBERS ) \
template<typename Visitor>\
static inline void visit( CONST TYPE& name, Visitor& v, uint32_t field = -1 ) { \
    v.start(name, BOOST_PP_STRINGIZE(TYPE) );\
    BOOST_PP_SEQ_FOR_EACH_I( ACCEPT_BASE, TYPE, INHERITS ) \
    switch( field ) { \
        case -1: \
            BOOST_PP_SEQ_FOR_EACH_I( CUSTOM_MEMBER_ALL, TYPE, MEMBERS ) \
            break; \
        BOOST_PP_SEQ_FOR_EACH_I( CUSTOM_MEMBER_CASES, TYPE, MEMBERS ) \
        default: \
            v.not_found( name, field );\
    }\
    v.end(name, BOOST_PP_STRINGIZE(TYPE) );\
} \

#define BOOST_REFLECT_EMPTY

#define VISIT_BASE( r, data, elem ) reflector<elem>::visit( *((vtable<elem,InterfaceDelegate>*)&a), data );
#define VISIT_MEMBER( r, data, elem ) t.template accept<data>( a.elem, BOOST_PP_STRINGIZE(elem) );
#endif // DOXYGEN

/**
 *  @brief Specializes boost::reflect::reflector for TYPE
 *
 *  @param INHERITS - a sequence of base class names (basea)(baseb)(basec)
 *  @param MEMBERS - a sequence of member names.  (field1)(field2)(field3)
 */
#define BOOST_REFLECT( TYPE, INHERITS, MEMBERS ) \
BOOST_REFLECT_TYPEINFO(TYPE) \
namespace boost { namespace reflect { \
template<> struct reflector<TYPE> {\
    BOOST_REFLECT_IMPL( const, TYPE, INHERITS, MEMBERS ) \
    BOOST_REFLECT_IMPL( BOOST_REFLECT_EMPTY, TYPE, INHERITS, MEMBERS ) \
\
    template<typename T, typename InterfaceDelegate> \
    static void visit( boost::reflect::vtable<TYPE,InterfaceDelegate>& a, T& t ) { \
        BOOST_PP_SEQ_FOR_EACH( VISIT_BASE, t, INHERITS ) \
        BOOST_PP_SEQ_FOR_EACH( VISIT_MEMBER, TYPE, MEMBERS ) \
    } \
}; } }


/**
 *   This macro is identical to BOOST_REFLECT, except that it gives you
 *   the ability to customize field numbers and flags.
 *
 *   @param MEMBERS  - a sequence of 3 param tuples.
 *                    ((field_name, NUMBER, FLAGS))((field_name1, NUMBER, FLAGS))
 *
 */
#define BOOST_REFLECT_CUSTOM( TYPE, INHERITS, MEMBERS ) \
    BOOST_REFLECT_TYPEINFO(TYPE) \
namespace boost { namespace reflect { \
template<> struct reflector<TYPE> { \
    BOOST_REFLECT_CUSTOM_IMPL( const, TYPE, INHERITS, MEMBERS ) \
    BOOST_REFLECT_CUSTOM_IMPL( BOOST_REFLECT_EMPTY, TYPE, INHERITS, MEMBERS ) \
} } }
