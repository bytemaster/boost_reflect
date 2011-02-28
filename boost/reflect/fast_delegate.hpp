/**
 * @file fast_delegate.hpp
 *
 * This class is used in place of boost::function<> because boost::function depends upon
 * RTTI
 *
 */


//						fast_delegate.hpp
//	Efficient delegates in C++ that generate only two lines of asm code!
//  Documentation is found at http://www.codeproject.com/cpp/fast_delegate.asp
//
//						- Don Clugston, Mar 2004.
//		Major contributions were made by Jody Hagins.
// History:
// 24-Apr-04 1.0  * Submitted to CodeProject. 
// 28-Apr-04 1.1  * Prevent most unsafe uses of evil static function hack.
//				  * Improved syntax for horrible_cast (thanks Paul Bludov).
//				  * Tested on Metrowerks MWCC and Intel ICL (IA32)
//				  * Compiled, but not run, on Comeau C++ and Intel Itanium ICL.
//	27-Jun-04 1.2 * Now works on Borland C++ Builder 5.5
//				  * Now works on /clr "managed C++" code on VC7, VC7.1
//				  * Comeau C++ now compiles without warnings.
//				  * Prevent the virtual inheritance case from being used on 
//					  VC6 and earlier, which generate incorrect code.
//				  * Improved warning and error messages. Non-standard hacks
//					 now have compile-time checks to make them safer.
//				  * implicit_cast used instead of static_cast in many cases.
//				  * If calling a const member function, a const class pointer can be used.
//				  * make_fast_delegate() global helper function added to simplify pass-by-value.
//				  * Added fastdelegate.clear()
// 16-Jul-04 1.2.1* Workaround for gcc bug (const member function pointers in templates)
// 30-Oct-04 1.3  * Support for (non-void) return values.
//				  * No more workarounds in client code!
//					 MSVC and Intel now use a clever hack invented by John Dlugosz:
//				     - The FASTDELEGATEDECLARE workaround is no longer necessary.
//					 - No more warning messages for VC6
//				  * Less use of macros. Error messages should be more comprehensible.
//				  * Added include guards
//				  * Added fast_delegate::empty() to test if invocation is safe (Thanks Neville Franks).
//				  * Now tested on VS 2005 Express Beta, PGI C++
// 24-Dec-04 1.4  * Added delegate_memento, to allow collections of disparate delegates.
//                * <,>,<=,>= comparison operators to allow storage in ordered containers.
//				  * Substantial reduction of code size, especially the 'Closure' class.
//				  * Standardised all the compiler-specific workarounds.
//                * MFP conversion now works for CodePlay (but not yet supported in the full code).
//                * Now compiles without warnings on _any_ supported compiler, including BCC 5.5.1
//				  * New syntax: fast_delegate< int (char *, double) >. 
// 14-Feb-05 1.4.1* Now treats =0 as equivalent to .clear(), ==0 as equivalent to .empty(). (Thanks elfric).
//				  * Now tested on Intel ICL for AMD64, VS2005 Beta for AMD64 and Itanium.
// 30-Mar-05 1.5  * Safebool idiom: "if (dg)" is now equivalent to "if (!dg.empty())"
//				  * Fully supported by CodePlay VectorC
//                * Bugfix for Metrowerks: empty() was buggy because a valid MFP can be 0 on MWCC!
//                * More optimal assignment,== and != operators for static function pointers.

#ifndef _REFLECT_FASTDELEGATE_H
#define _REFLECT_FASTDELEGATE_H

#include <string.h> // to allow <,> comparisons
namespace boost { namespace reflect {

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include <memory.h> // to allow <,> comparisons

////////////////////////////////////////////////////////////////////////////////
//						Configuration options
//
////////////////////////////////////////////////////////////////////////////////

// Uncomment the following #define for optimally-sized delegates.
// In this case, the generated asm code is almost identical to the code you'd get
// if the compiler had native support for delegates.
// It will not work on systems where sizeof(dataptr) < sizeof(codeptr). 
// Thus, it will not work for DOS compilers using the medium model.
// It will also probably fail on some DSP systems.
#define FASTDELEGATE_USESTATICFUNCTIONHACK

// Uncomment the next line to allow function declarator syntax.
// It is automatically enabled for those compilers where it is known to work.
#define FASTDELEGATE_ALLOW_FUNCTION_TYPE_SYNTAX

////////////////////////////////////////////////////////////////////////////////
//						Compiler identification for workarounds
//
////////////////////////////////////////////////////////////////////////////////

// Compiler identification. It's not easy to identify Visual C++ because
// many vendors fraudulently define Microsoft's identifiers.
#if defined(_MSC_VER) && !defined(__MWERKS__) && !defined(__VECTOR_C) && !defined(__ICL) && !defined(__BORLANDC__)
#define FASTDLGT_ISMSVC

#if (_MSC_VER <1300) // Many workarounds are required for VC6.
#define FASTDLGT_VC6
#pragma warning(disable:4786) // disable this ridiculous warning
#endif

#endif

// Does the compiler uses Microsoft's member function pointer structure?
// If so, it needs special treatment.
// Metrowerks CodeWarrior, Intel, and CodePlay fraudulently define Microsoft's 
// identifier, _MSC_VER. We need to filter Metrowerks out.
#if defined(_MSC_VER) && !defined(__MWERKS__)
#define FASTDLGT_MICROSOFT_MFP

#if !defined(__VECTOR_C)
// CodePlay doesn't have the __single/multi/virtual_inheritance keywords
#define FASTDLGT_HASINHERITANCE_KEYWORDS
#endif
#endif

// Does it allow function declarator syntax? The following compilers are known to work:
#if defined(FASTDLGT_ISMSVC) && (_MSC_VER >=1310) // VC 7.1
#define FASTDELEGATE_ALLOW_FUNCTION_TYPE_SYNTAX
#endif

// Gcc(2.95+), and versions of Digital Mars, Intel and Comeau in common use.
#if defined (__DMC__) || defined(__GNUC__) || defined(__ICL) || defined(__COMO__)
#define FASTDELEGATE_ALLOW_FUNCTION_TYPE_SYNTAX
#endif

// It works on Metrowerks MWCC 3.2.2. From boost.Config it should work on earlier ones too.
#if defined (__MWERKS__)
#define FASTDELEGATE_ALLOW_FUNCTION_TYPE_SYNTAX
#endif

#ifdef __GNUC__ // Workaround GCC bug #8271 
	// At present, GCC doesn't recognize constness of MFPs in templates
#define FASTDELEGATE_GCC_BUG_8271
#endif



////////////////////////////////////////////////////////////////////////////////
//						General tricks used in this code
//
// (a) Error messages are generated by typdefing an array of negative size to
//     generate compile-time errors.
// (b) Warning messages on MSVC are generated by declaring unused variables, and
//	    enabling the "variable XXX is never used" warning.
// (c) Unions are used in a few compiler-specific cases to perform illegal casts.
// (d) For Microsoft and Intel, when adjusting the 'this' pointer, it's cast to
//     (char *) first to ensure that the correct number of *bytes* are added.
//
////////////////////////////////////////////////////////////////////////////////
//						Helper templates
//
////////////////////////////////////////////////////////////////////////////////

/// @cond FASTDELEGATE
namespace detail {	// we'll hide the implementation details in a nested namespace.

//		implicit_cast< >
// I believe this was originally going to be in the C++ standard but 
// was left out by accident. It's even milder than static_cast.
// I use it instead of static_cast<> to emphasize that I'm not doing
// anything nasty. 
// Usage is identical to static_cast<>
/**
    @note I added
*/
template <class OutputClass, class InputClass>
inline OutputClass implicit_cast(InputClass input){
	return static_cast<OutputClass>(input); // changed by Dan Larimer
	//return input; // original line
}

//		horrible_cast< >
// This is truly evil. It completely subverts C++'s type system, allowing you 
// to cast from any class to any other class. Technically, using a union 
// to perform the cast is undefined behaviour (even in C). But we can see if
// it is OK by checking that the union is the same size as each of its members.
// horrible_cast<> should only be used for compiler-specific workarounds. 
// Usage is identical to reinterpret_cast<>.

// This union is declared outside the horrible_cast because BCC 5.5.1
// can't inline a function with a nested class, and gives a warning.
template <class OutputClass, class InputClass>
union horrible_union{
	OutputClass out;
	InputClass in;
};

template <class OutputClass, class InputClass>
inline OutputClass horrible_cast(const InputClass input){
	horrible_union<OutputClass, InputClass> u;
	// Cause a compile-time error if in, out and u are not the same size.
	// If the compile fails here, it means the compiler has peculiar
	// unions which would prevent the cast from working.
	typedef int ERROR_CantUseHorrible_cast[sizeof(InputClass)==sizeof(u) 
		&& sizeof(InputClass)==sizeof(OutputClass) ? 1 : -1];
	u.in = input;
	return u.out;
}

////////////////////////////////////////////////////////////////////////////////
//						Workarounds
//
////////////////////////////////////////////////////////////////////////////////

// Backwards compatibility: This macro used to be necessary in the virtual inheritance
// case for Intel and Microsoft. Now it just forward-declares the class.
#define FASTDELEGATEDECLARE(CLASSNAME)	class CLASSNAME;

// Prevent use of the static function hack with the DOS medium model.
#ifdef __MEDIUM__
#undef FASTDELEGATE_USESTATICFUNCTIONHACK
#endif

//			DefaultVoid - a workaround for 'void' templates in VC6.
//
//  (1) VC6 and earlier do not allow 'void' as a default template argument.
//  (2) They also doesn't allow you to return 'void' from a function.
//
// Workaround for (1): Declare a dummy type 'DefaultVoid' which we use
//   when we'd like to use 'void'. We convert it into 'void' and back
//   using the templates default_void_to_void<> and void_to_default_void<>.
// Workaround for (2): On VC6, the code for calling a void function is
//   identical to the code for calling a non-void function in which the
//   return value is never used, provided the return value is returned
//   in the EAX register, rather than on the stack. 
//   This is true for most fundamental types such as int, enum, void *.
//   Const void * is the safest option since it doesn't participate 
//   in any automatic conversions. But on a 16-bit compiler it might
//   cause extra code to be generated, so we disable it for all compilers
//   except for VC6 (and VC5).
#ifdef FASTDLGT_VC6
// VC6 workaround
typedef const void * DefaultVoid;
#else
// On any other compiler, just use a normal void.
typedef void DefaultVoid;
#endif

// Translate from 'DefaultVoid' to 'void'.
// Everything else is unchanged
template <class T>
struct default_void_to_void { typedef T type; };

template <>
struct default_void_to_void<DefaultVoid> {	typedef void type; };

// Translate from 'void' into 'DefaultVoid'
// Everything else is unchanged
template <class T>
struct void_to_default_void { typedef T type; };

template <>
struct void_to_default_void<void> { typedef DefaultVoid type; };



////////////////////////////////////////////////////////////////////////////////
//						Fast Delegates, part 1:
//
//		Conversion of member function pointer to a standard form
//
////////////////////////////////////////////////////////////////////////////////

// GenericClass is a fake class, ONLY used to provide a type.
// It is vitally important that it is never defined, so that the compiler doesn't
// think it can optimize the invocation. For example, Borland generates simpler
// code if it knows the class only uses single inheritance.

// Compilers using Microsoft's structure need to be treated as a special case.
#ifdef  FASTDLGT_MICROSOFT_MFP

#ifdef FASTDLGT_HASINHERITANCE_KEYWORDS
	// For Microsoft and Intel, we want to ensure that it's the most efficient type of MFP 
	// (4 bytes), even when the /vmg option is used. Declaring an empty class 
	// would give 16 byte pointers in this case....
	class __single_inheritance GenericClass;
#endif
	// ...but for Codeplay, an empty class *always* gives 4 byte pointers.
	// If compiled with the /clr option ("managed C++"), the JIT compiler thinks
	// it needs to load GenericClass before it can call any of its functions,
	// (compiles OK but crashes at runtime!), so we need to declare an 
	// empty class to make it happy.
	// Codeplay and VC4 can't cope with the unknown_inheritance case either.
	class GenericClass {};
#else
	class GenericClass {};
#endif

// The size of a single inheritance member function pointer.
const int SINGLE_MEMFUNCPTR_SIZE = sizeof(void (GenericClass::*)());
const int SINGLE_MEMDATA_SIZE = sizeof(int GenericClass::*);

//						SimplifyMemFunc< >::Convert()
//
//	A template function that converts an arbitrary member function pointer into the 
//	simplest possible form of member function pointer, using a supplied 'this' pointer.
//  According to the standard, this can be done legally with reinterpret_cast<>.
//	For (non-standard) compilers which use member function pointers which vary in size 
//  depending on the class, we need to use	knowledge of the internal structure of a 
//  member function pointer, as used by the compiler. Template specialization is used
//  to distinguish between the sizes. Because some compilers don't support partial 
//	template specialisation, I use full specialisation of a wrapper struct.

// general case -- don't know how to convert it. Force a compile failure
template <int N>
struct SimplifyMemFunc {
	template <class X, class XFuncType, class GenericMemFuncType>
	inline static GenericClass *Convert(X *pthis, XFuncType function_to_bind, 
		GenericMemFuncType &bound_func) { 
		// Unsupported member function type -- force a compile failure.
	    // (it's illegal to have a array with negative size).
		typedef char ERROR_Unsupported_member_function_pointer_on_this_compiler[N-100];
		return 0; 
	}
};

// For compilers where all member func ptrs are the same size, everything goes here.
// For non-standard compilers, only single_inheritance classes go here.
template <>
struct SimplifyMemFunc<SINGLE_MEMFUNCPTR_SIZE>  {	
	template <class X, class XFuncType, class GenericMemFuncType>
	inline static GenericClass *Convert(X *pthis, XFuncType function_to_bind, 
			GenericMemFuncType &bound_func) {
#if defined __DMC__  
		// Digital Mars doesn't allow you to cast between abitrary PMF's, 
		// even though the standard says you can. The 32-bit compiler lets you
		// static_cast through an int, but the DOS compiler doesn't.
		bound_func = horrible_cast<GenericMemFuncType>(function_to_bind);
#else 
        bound_func = reinterpret_cast<GenericMemFuncType>(function_to_bind);
#endif
        return reinterpret_cast<GenericClass *>(pthis);
	}
};

////////////////////////////////////////////////////////////////////////////////
//						Fast Delegates, part 1b:
//
//					Workarounds for Microsoft and Intel
//
////////////////////////////////////////////////////////////////////////////////


// Compilers with member function pointers which violate the standard (MSVC, Intel, Codeplay),
// need to be treated as a special case.
#ifdef FASTDLGT_MICROSOFT_MFP

// We use unions to perform horrible_casts. I would like to use #pragma pack(push, 1)
// at the start of each function for extra safety, but VC6 seems to ICE
// intermittently if you do this inside a template.

// __multiple_inheritance classes go here
// Nasty hack for Microsoft and Intel (IA32 and Itanium)
template<>
struct SimplifyMemFunc< SINGLE_MEMFUNCPTR_SIZE + sizeof(int) >  {
	template <class X, class XFuncType, class GenericMemFuncType>
	inline static GenericClass *Convert(X *pthis, XFuncType function_to_bind, 
		GenericMemFuncType &bound_func) { 
		// We need to use a horrible_cast to do this conversion.
		// In MSVC, a multiple inheritance member pointer is internally defined as:
        union {
			XFuncType func;
			struct {	 
				GenericMemFuncType funcaddress; // points to the actual member function
				int delta;	     // #BYTES to be added to the 'this' pointer
			}s;
        } u;
		// Check that the horrible_cast will work
		typedef int ERROR_CantUsehorrible_cast[sizeof(function_to_bind)==sizeof(u.s)? 1 : -1];
        u.func = function_to_bind;
		bound_func = u.s.funcaddress;
		return reinterpret_cast<GenericClass *>(reinterpret_cast<char *>(pthis) + u.s.delta); 
	}
};

// virtual inheritance is a real nuisance. It's inefficient and complicated.
// On MSVC and Intel, there isn't enough information in the pointer itself to
// enable conversion to a closure pointer. Earlier versions of this code didn't
// work for all cases, and generated a compile-time error instead.
// But a very clever hack invented by John M. Dlugosz solves this problem.
// My code is somewhat different to his: I have no asm code, and I make no 
// assumptions about the calling convention that is used.

// In VC++ and ICL, a virtual_inheritance member pointer 
// is internally defined as:
struct MicrosoftVirtualMFP {
	void (GenericClass::*codeptr)(); // points to the actual member function
	int delta;		// #bytes to be added to the 'this' pointer
	int vtable_index; // or 0 if no virtual inheritance
};
// The CRUCIAL feature of Microsoft/Intel MFPs which we exploit is that the
// m_codeptr member is *always* called, regardless of the values of the other
// members. (This is *not* true for other compilers, eg GCC, which obtain the
// function address from the vtable if a virtual function is being called).
// Dlugosz's trick is to make the codeptr point to a probe function which
// returns the 'this' pointer that was used.

// Define a generic class that uses virtual inheritance.
// It has a trival member function that returns the value of the 'this' pointer.
struct GenericVirtualClass : virtual public GenericClass
{
	typedef GenericVirtualClass * (GenericVirtualClass::*ProbePtrType)();
	GenericVirtualClass * GetThis() { return this; }
};

// __virtual_inheritance classes go here
template <>
struct SimplifyMemFunc<SINGLE_MEMFUNCPTR_SIZE + 2*sizeof(int) >
{

	template <class X, class XFuncType, class GenericMemFuncType>
	inline static GenericClass *Convert(X *pthis, XFuncType function_to_bind, 
		GenericMemFuncType &bound_func) {
		union {
			XFuncType func;
			GenericClass* (X::*ProbeFunc)();
			MicrosoftVirtualMFP s;
		} u;
		u.func = function_to_bind;
		bound_func = reinterpret_cast<GenericMemFuncType>(u.s.codeptr);
		union {
			GenericVirtualClass::ProbePtrType virtfunc;
			MicrosoftVirtualMFP s;
		} u2;
		// Check that the horrible_cast<>s will work
		typedef int ERROR_CantUsehorrible_cast[sizeof(function_to_bind)==sizeof(u.s)
			&& sizeof(function_to_bind)==sizeof(u.ProbeFunc)
			&& sizeof(u2.virtfunc)==sizeof(u2.s) ? 1 : -1];
   // Unfortunately, taking the address of a MF prevents it from being inlined, so 
   // this next line can't be completely optimised away by the compiler.
		u2.virtfunc = &GenericVirtualClass::GetThis;
		u.s.codeptr = u2.s.codeptr;
		return (pthis->*u.ProbeFunc)();
	}
};

#if (_MSC_VER <1300)

// Nasty hack for Microsoft Visual C++ 6.0
// unknown_inheritance classes go here
// There is a compiler bug in MSVC6 which generates incorrect code in this case!!
template <>
struct SimplifyMemFunc<SINGLE_MEMFUNCPTR_SIZE + 3*sizeof(int) >
{
	template <class X, class XFuncType, class GenericMemFuncType>
	inline static GenericClass *Convert(X *pthis, XFuncType function_to_bind, 
		GenericMemFuncType &bound_func) {
		// There is an apalling but obscure compiler bug in MSVC6 and earlier:
		// vtable_index and 'vtordisp' are always set to 0 in the 
		// unknown_inheritance case!
		// This means that an incorrect function could be called!!!
		// Compiling with the /vmg option leads to potentially incorrect code.
		// This is probably the reason that the IDE has a user interface for specifying
		// the /vmg option, but it is disabled -  you can only specify /vmg on 
		// the command line. In VC1.5 and earlier, the compiler would ICE if it ever
		// encountered this situation.
		// It is OK to use the /vmg option if /vmm or /vms is specified.

		// Fortunately, the wrong function is only called in very obscure cases.
		// It only occurs when a derived class overrides a virtual function declared 
		// in a virtual base class, and the member function 
		// points to the *Derived* version of that function. The problem can be
		// completely averted in 100% of cases by using the *Base class* for the 
		// member fpointer. Ie, if you use the base class as an interface, you'll
		// stay out of trouble.
		// Occasionally, you might want to point directly to a derived class function
		// that isn't an override of a base class. In this case, both vtable_index 
		// and 'vtordisp' are zero, but a virtual_inheritance pointer will be generated.
		// We can generate correct code in this case. To prevent an incorrect call from
		// ever being made, on MSVC6 we generate a warning, and call a function to 
		// make the program crash instantly. 
		typedef char ERROR_VC6CompilerBug[-100];
		return 0; 
	}
};


#else 

// Nasty hack for Microsoft and Intel (IA32 and Itanium)
// unknown_inheritance classes go here 
// This is probably the ugliest bit of code I've ever written. Look at the casts!
// There is a compiler bug in MSVC6 which prevents it from using this code.
template <>
struct SimplifyMemFunc<SINGLE_MEMFUNCPTR_SIZE + 3*sizeof(int) >
{
	template <class X, class XFuncType, class GenericMemFuncType>
	inline static GenericClass *Convert(X *pthis, XFuncType function_to_bind, 
			GenericMemFuncType &bound_func) {
		// The member function pointer is 16 bytes long. We can't use a normal cast, but
		// we can use a union to do the conversion.
		union {
			XFuncType func;
			// In VC++ and ICL, an unknown_inheritance member pointer 
			// is internally defined as:
			struct {
				GenericMemFuncType m_funcaddress; // points to the actual member function
				int delta;		// #bytes to be added to the 'this' pointer
				int vtordisp;		// #bytes to add to 'this' to find the vtable
				int vtable_index; // or 0 if no virtual inheritance
			} s;
		} u;
		// Check that the horrible_cast will work
		typedef int ERROR_CantUsehorrible_cast[sizeof(XFuncType)==sizeof(u.s)? 1 : -1];
		u.func = function_to_bind;
		bound_func = u.s.funcaddress;
		int virtual_delta = 0;
		if (u.s.vtable_index) { // Virtual inheritance is used
			// First, get to the vtable. 
			// It is 'vtordisp' bytes from the start of the class.
			const int * vtable = *reinterpret_cast<const int *const*>(
				reinterpret_cast<const char *>(pthis) + u.s.vtordisp );

			// 'vtable_index' tells us where in the table we should be looking.
			virtual_delta = u.s.vtordisp + *reinterpret_cast<const int *>( 
				reinterpret_cast<const char *>(vtable) + u.s.vtable_index);
		}
		// The int at 'virtual_delta' gives us the amount to add to 'this'.
        // Finally we can add the three components together. Phew!
        return reinterpret_cast<GenericClass *>(
			reinterpret_cast<char *>(pthis) + u.s.delta + virtual_delta);
	};
};
#endif // MSVC 7 and greater

#endif // MS/Intel hacks

}  // namespace detail


////////////////////////////////////////////////////////////////////////////////
//						Fast Delegates, part 2:
//
//	Define the delegate storage, and cope with static functions
//
////////////////////////////////////////////////////////////////////////////////

// delegate_memento -- an opaque structure which can hold an arbitary delegate.
// It knows nothing about the calling convention or number of arguments used by
// the function pointed to.
// It supplies comparison operators so that it can be stored in STL collections.
// It cannot be set to anything other than null, nor invoked directly: 
//   it must be converted to a specific delegate.

// Implementation:
// There are two possible implementations: the Safe method and the Evil method.
//				delegate_memento - Safe version
//
// This implementation is standard-compliant, but a bit tricky.
// A static function pointer is stored inside the class. 
// Here are the valid values:
// +-- Static pointer --+--pThis --+-- pMemFunc-+-- Meaning------+
// |   0				|  0       |   0        | Empty          |
// |   !=0              |(dontcare)|  Invoker   | Static function|
// |   0                |  !=0     |  !=0*      | Method call    |
// +--------------------+----------+------------+----------------+
//  * For Metrowerks, this can be 0. (first virtual function in a 
//       single_inheritance class).
// When stored stored inside a specific delegate, the 'dontcare' entries are replaced
// with a reference to the delegate itself. This complicates the = and == operators
// for the delegate class.

//				delegate_memento - Evil version
//
// For compilers where data pointers are at least as big as code pointers, it is 
// possible to store the function pointer in the this pointer, using another 
// horrible_cast. In this case the delegate_memento implementation is simple:
// +--pThis --+-- pMemFunc-+-- Meaning---------------------+
// |    0     |  0         | Empty                         |
// |  !=0     |  !=0*      | Static function or method call|
// +----------+------------+-------------------------------+
//  * For Metrowerks, this can be 0. (first virtual function in a 
//       single_inheritance class).
// Note that the Sun C++ and MSVC documentation explicitly state that they 
// support static_cast between void * and function pointers.

class delegate_memento {
protected: 
	// the data is protected, not private, because many
	// compilers have problems with template friends.
	typedef void (detail::GenericClass::*GenericMemFuncType)(); // arbitrary MFP.
	GenericMemFuncType m_pFunction;
	detail::GenericClass *m_pthis;

#if !defined(FASTDELEGATE_USESTATICFUNCTIONHACK)
	typedef void (*GenericFuncPtr)(); // arbitrary code pointer
	GenericFuncPtr m_pStaticFunction;
#endif

public:
#if !defined(FASTDELEGATE_USESTATICFUNCTIONHACK)
	delegate_memento() : m_pthis(0), m_pFunction(0), m_pStaticFunction(0) {};
	void clear() {
		m_pthis=0; m_pFunction=0; m_pStaticFunction=0;
	}
#else
	delegate_memento() : m_pFunction(0), m_pthis(0) {};
	void clear() {	m_pthis=0; m_pFunction=0;	}
#endif
public:
#if !defined(FASTDELEGATE_USESTATICFUNCTIONHACK)
	inline bool IsEqual (const delegate_memento &x) const{
	    // We have to cope with the static function pointers as a special case
		if (m_pFunction!=x.m_pFunction) return false;
		// the static function ptrs must either both be equal, or both be 0.
		if (m_pStaticFunction!=x.m_pStaticFunction) return false;
		if (m_pStaticFunction!=0) return m_pthis==x.m_pthis;
		else return true;
	}
#else // Evil Method
	inline bool IsEqual (const delegate_memento &x) const{
		return m_pthis==x.m_pthis && m_pFunction==x.m_pFunction;
	}
#endif
	// Provide a strict weak ordering for delegate_mementos.
	inline bool IsLess(const delegate_memento &right) const {
		// deal with static function pointers first
#if !defined(FASTDELEGATE_USESTATICFUNCTIONHACK)
		if (m_pStaticFunction !=0 || right.m_pStaticFunction!=0) 
				return m_pStaticFunction < right.m_pStaticFunction;
#endif
		if (m_pthis !=right.m_pthis) return m_pthis < right.m_pthis;
	// There are no ordering operators for member function pointers, 
	// but we can fake one by comparing each byte. The resulting ordering is
	// arbitrary (and compiler-dependent), but it permits storage in ordered STL containers.
		return memcmp(&m_pFunction, &right.m_pFunction, sizeof(m_pFunction)) < 0;

	}
	// BUGFIX (Mar 2005):
	// We can't just compare m_pFunction because on Metrowerks,
	// m_pFunction can be zero even if the delegate is not empty!
	inline bool operator ! () const		// Is it bound to anything?
	{ return m_pthis==0 && m_pFunction==0; }
	inline bool empty() const		// Is it bound to anything?
	{ return m_pthis==0 && m_pFunction==0; }
public:
	delegate_memento & operator = (const delegate_memento &right)  {
		set_mementoFrom(right); 
		return *this;
	}
	inline bool operator <(const delegate_memento &right) {
		return IsLess(right);
	}
	inline bool operator >(const delegate_memento &right) {
		return right.IsLess(*this);
	}
	delegate_memento (const delegate_memento &right)  : 
		m_pFunction(right.m_pFunction), m_pthis(right.m_pthis)
#if !defined(FASTDELEGATE_USESTATICFUNCTIONHACK)
		, m_pStaticFunction (right.m_pStaticFunction)
#endif
		{}
protected:
	void set_mementoFrom(const delegate_memento &right)  {
		m_pFunction = right.m_pFunction;
		m_pthis = right.m_pthis;
#if !defined(FASTDELEGATE_USESTATICFUNCTIONHACK)
		m_pStaticFunction = right.m_pStaticFunction;
#endif
	}
};


//						ClosurePtr<>
//
// A private wrapper class that adds function signatures to delegate_memento.
// It's the class that does most of the actual work.
// The signatures are specified by:
// GenericMemFunc: must be a type of GenericClass member function pointer. 
// StaticFuncPtr:  must be a type of function pointer with the same signature 
//                 as GenericMemFunc.
// UnvoidStaticFuncPtr: is the same as StaticFuncPtr, except on VC6
//                 where it never returns void (returns DefaultVoid instead).

// An outer class, fast_delegateN<>, handles the invoking and creates the
// necessary typedefs.
// This class does everything else.

namespace detail {

template < class GenericMemFunc, class StaticFuncPtr, class UnvoidStaticFuncPtr>
class ClosurePtr : public delegate_memento {
public:
	// These functions are for setting the delegate to a member function.

	// Here's the clever bit: we convert an arbitrary member function into a 
	// standard form. XMemFunc should be a member function of class X, but I can't 
	// enforce that here. It needs to be enforced by the wrapper class.
	template < class X, class XMemFunc >
	inline void bindmemfunc(X *pthis, XMemFunc function_to_bind ) {
		m_pthis = SimplifyMemFunc< sizeof(function_to_bind) >
			::Convert(pthis, function_to_bind, m_pFunction);
#if !defined(FASTDELEGATE_USESTATICFUNCTIONHACK)
		m_pStaticFunction = 0;
#endif
	}
	// For const member functions, we only need a const class pointer.
	// Since we know that the member function is const, it's safe to 
	// remove the const qualifier from the 'this' pointer with a const_cast.
	// VC6 has problems if we just overload 'bindmemfunc', so we give it a different name.
	template < class X, class XMemFunc>
	inline void bindconstmemfunc(const X *pthis, XMemFunc function_to_bind) {
		m_pthis= SimplifyMemFunc< sizeof(function_to_bind) >
			::Convert(const_cast<X*>(pthis), function_to_bind, m_pFunction);
#if !defined(FASTDELEGATE_USESTATICFUNCTIONHACK)
		m_pStaticFunction = 0;
#endif
	}
#ifdef FASTDELEGATE_GCC_BUG_8271	// At present, GCC doesn't recognize constness of MFPs in templates
	template < class X, class XMemFunc>
	inline void bindmemfunc(const X *pthis, XMemFunc function_to_bind) {
		bindconstmemfunc(pthis, function_to_bind);
#if !defined(FASTDELEGATE_USESTATICFUNCTIONHACK)
		m_pStaticFunction = 0;
#endif
	}
#endif
	// These functions are required for invoking the stored function
	inline GenericClass *GetClosureThis() const { return m_pthis; }
	inline GenericMemFunc GetClosureMemPtr() const { return reinterpret_cast<GenericMemFunc>(m_pFunction); }

// There are a few ways of dealing with static function pointers.
// There's a standard-compliant, but tricky method.
// There's also a straightforward hack, that won't work on DOS compilers using the
// medium memory model. It's so evil that I can't recommend it, but I've
// implemented it anyway because it produces very nice asm code.

#if !defined(FASTDELEGATE_USESTATICFUNCTIONHACK)

//				ClosurePtr<> - Safe version
//
// This implementation is standard-compliant, but a bit tricky.
// I store the function pointer inside the class, and the delegate then
// points to itself. Whenever the delegate is copied, these self-references
// must be transformed, and this complicates the = and == operators.
public:
	// The next two functions are for operator ==, =, and the copy constructor.
	// We may need to convert the m_pthis pointers, so that
	// they remain as self-references.
	template< class DerivedClass >
	inline void CopyFrom (DerivedClass *pParent, const delegate_memento &x) {
		set_mementoFrom(x);
		if (m_pStaticFunction!=0) {
			// transform self references...
			m_pthis=reinterpret_cast<GenericClass *>(pParent);
		}
	}
	// For static functions, the 'static_function_invoker' class in the parent 
	// will be called. The parent then needs to call GetStaticFunction() to find out 
	// the actual function to invoke.
	template < class DerivedClass, class ParentInvokerSig >
	inline void bindstaticfunc(DerivedClass *pParent, ParentInvokerSig static_function_invoker, 
				StaticFuncPtr function_to_bind ) {
		if (function_to_bind==0) { // cope with assignment to 0
			m_pFunction=0;
		} else { 
			bindmemfunc(pParent, static_function_invoker);
        }
		m_pStaticFunction=reinterpret_cast<GenericFuncPtr>(function_to_bind);
	}
	inline UnvoidStaticFuncPtr GetStaticFunction() const { 
		return reinterpret_cast<UnvoidStaticFuncPtr>(m_pStaticFunction); 
	}
#else

//				ClosurePtr<> - Evil version
//
// For compilers where data pointers are at least as big as code pointers, it is 
// possible to store the function pointer in the this pointer, using another 
// horrible_cast. Invocation isn't any faster, but it saves 4 bytes, and
// speeds up comparison and assignment. If C++ provided direct language support
// for delegates, they would produce asm code that was almost identical to this.
// Note that the Sun C++ and MSVC documentation explicitly state that they 
// support static_cast between void * and function pointers.

	template< class DerivedClass >
	inline void CopyFrom (DerivedClass /* *pParent*/, const delegate_memento &right) {
		set_mementoFrom(right);
	}
	// For static functions, the 'static_function_invoker' class in the parent 
	// will be called. The parent then needs to call GetStaticFunction() to find out 
	// the actual function to invoke.
	// ******** EVIL, EVIL CODE! *******
	template < 	class DerivedClass, class ParentInvokerSig>
	inline void bindstaticfunc(DerivedClass *pParent, ParentInvokerSig static_function_invoker, 
				StaticFuncPtr function_to_bind) {
		if (function_to_bind==0) { // cope with assignment to 0
			m_pFunction=0;
		} else { 
		   // We'll be ignoring the 'this' pointer, but we need to make sure we pass
		   // a valid value to bindmemfunc().
			bindmemfunc(pParent, static_function_invoker);
        }

		// WARNING! Evil hack. We store the function in the 'this' pointer!
		// Ensure that there's a compilation failure if function pointers 
		// and data pointers have different sizes.
		// If you get this error, you need to #undef FASTDELEGATE_USESTATICFUNCTIONHACK.
		typedef int ERROR_CantUseEvilMethod[sizeof(GenericClass *)==sizeof(function_to_bind) ? 1 : -1];
		m_pthis = horrible_cast<GenericClass *>(function_to_bind);
		// MSVC, SunC++ and DMC accept the following (non-standard) code:
//		m_pthis = static_cast<GenericClass *>(static_cast<void *>(function_to_bind));
		// BCC32, Comeau and DMC accept this method. MSVC7.1 needs __int64 instead of long
//		m_pthis = reinterpret_cast<GenericClass *>(reinterpret_cast<long>(function_to_bind));
	}
	// ******** EVIL, EVIL CODE! *******
	// This function will be called with an invalid 'this' pointer!!
	// We're just returning the 'this' pointer, converted into
	// a function pointer!
	inline UnvoidStaticFuncPtr GetStaticFunction() const {
		// Ensure that there's a compilation failure if function pointers 
		// and data pointers have different sizes.
		// If you get this error, you need to #undef FASTDELEGATE_USESTATICFUNCTIONHACK.
		typedef int ERROR_CantUseEvilMethod[sizeof(UnvoidStaticFuncPtr)==sizeof(this) ? 1 : -1];
		return horrible_cast<UnvoidStaticFuncPtr>(this);
	}
#endif // !defined(FASTDELEGATE_USESTATICFUNCTIONHACK)

	// Does the closure contain this static function?
	inline bool IsEqualToStaticFuncPtr(StaticFuncPtr funcptr){
		if (funcptr==0) return empty(); 
	// For the Evil method, if it doesn't actually contain a static function, this will return an arbitrary
	// value that is not equal to any valid function pointer.
		else return funcptr==reinterpret_cast<StaticFuncPtr>(GetStaticFunction());
	}
};


} // namespace detail

////////////////////////////////////////////////////////////////////////////////
//						Fast Delegates, part 3:
//
//				Wrapper classes to ensure type safety
//
////////////////////////////////////////////////////////////////////////////////


// Once we have the member function conversion templates, it's easy to make the
// wrapper classes. So that they will work with as many compilers as possible, 
// the classes are of the form
//   fast_delegate3<int, char *, double>
// They can cope with any combination of parameters. The max number of parameters
// allowed is 8, but it is trivial to increase this limit.
// Note that we need to treat const member functions seperately.
// All this class does is to enforce type safety, and invoke the delegate with
// the correct list of parameters.

// Because of the weird rule about the class of derived member function pointers,
// you sometimes need to apply a downcast to the 'this' pointer.
// This is the reason for the use of "implicit_cast<X*>(pthis)" in the code below. 
// If CDerivedClass is derived from CBaseClass, but doesn't override SimpleVirtualFunction,
// without this trick you'd need to write:
//		MyDelegate(static_cast<CBaseClass *>(&d), &CDerivedClass::SimpleVirtualFunction);
// but with the trick you can write
//		MyDelegate(&d, &CDerivedClass::SimpleVirtualFunction);

// RetType is the type the compiler uses in compiling the template. For VC6,
// it cannot be void. result_type is the real type which is returned from
// all of the functions. It can be void.

// Implicit conversion to "bool" is achieved using the safe_bool idiom,
// using member data pointers (MDP). This allows "if (dg)..." syntax
// Because some compilers (eg codeplay) don't have a unique value for a zero
// MDP, an extra padding member is added to the SafeBool struct.
// Some compilers (eg VC6) won't implicitly convert from 0 to an MDP, so
// in that case the static function constructor is not made explicit; this
// allows "if (dg==0) ..." to compile.

//N=0
template<class RetType=detail::DefaultVoid>
class fast_delegate0 {
public:
	typedef typename boost::reflect::detail::default_void_to_void<RetType>::type result_type;
private:
	typedef result_type (*StaticFunctionPtr)();
	typedef RetType (*UnvoidStaticFunctionPtr)();
	typedef RetType (boost::reflect::detail::GenericClass::*GenericMemFn)();
	typedef boost::reflect::detail::ClosurePtr<GenericMemFn, StaticFunctionPtr, UnvoidStaticFunctionPtr> ClosureType;
	ClosureType m_Closure;
public:
	// Typedefs to aid generic programming
	typedef fast_delegate0 type;

	// Construction and comparison functions
	fast_delegate0() { clear(); }
	fast_delegate0(const fast_delegate0 &x) {
		m_Closure.CopyFrom(this, x.m_Closure); }
	void operator = (const fast_delegate0 &x)  {
		m_Closure.CopyFrom(this, x.m_Closure); }
	bool operator ==(const fast_delegate0 &x) const {
		return m_Closure.IsEqual(x.m_Closure);	}
	bool operator !=(const fast_delegate0 &x) const {
		return !m_Closure.IsEqual(x.m_Closure); }
	bool operator <(const fast_delegate0 &x) const {
		return m_Closure.IsLess(x.m_Closure);	}
	bool operator >(const fast_delegate0 &x) const {
		return x.m_Closure.IsLess(m_Closure);	}
	// Binding to non-const member functions
	template < class X, class Y >
	fast_delegate0(Y *pthis, result_type (X::* function_to_bind)() ) {
		m_Closure.bindmemfunc(boost::reflect::detail::implicit_cast<X*>(pthis), function_to_bind); }
	template < class X, class Y >
	inline void bind(Y *pthis, result_type (X::* function_to_bind)()) {
		m_Closure.bindmemfunc(boost::reflect::detail::implicit_cast<X*>(pthis), function_to_bind);	}
	// Binding to const member functions.
	template < class X, class Y >
	fast_delegate0(const Y *pthis, result_type (X::* function_to_bind)() const) {
		m_Closure.bindconstmemfunc(boost::reflect::detail::implicit_cast<const X*>(pthis), function_to_bind);	}
	template < class X, class Y >
	inline void bind(const Y *pthis, result_type (X::* function_to_bind)() const) {
		m_Closure.bindconstmemfunc(boost::reflect::detail::implicit_cast<const X *>(pthis), function_to_bind);	}
	// Static functions. We convert them into a member function call.
	// This constructor also provides implicit conversion
	fast_delegate0(result_type (*function_to_bind)() ) {
		bind(function_to_bind);	}
	// for efficiency, prevent creation of a temporary
	void operator = (result_type (*function_to_bind)() ) {
		bind(function_to_bind);	}
	inline void bind(result_type (*function_to_bind)()) {
		m_Closure.bindstaticfunc(this, &fast_delegate0::InvokeStaticFunction, 
			function_to_bind); }
	// Invoke the delegate
	RetType operator() () const {
	return (m_Closure.GetClosureThis()->*(m_Closure.GetClosureMemPtr()))(); }
	// Implicit conversion to "bool" using the safe_bool idiom
private:
	typedef struct SafeBoolStruct {
		int a_data_pointer_to_this_is_0_on_buggy_compilers;
		StaticFunctionPtr m_nonzero;
	} UselessTypedef;
    typedef StaticFunctionPtr SafeBoolStruct::*unspecified_bool_type;
public:
	operator unspecified_bool_type() const {
        return empty()? 0: &SafeBoolStruct::m_nonzero;
    }
	// necessary to allow ==0 to work despite the safe_bool idiom
	inline bool operator==(StaticFunctionPtr funcptr) {
		return m_Closure.IsEqualToStaticFuncPtr(funcptr);	}
	inline bool operator!=(StaticFunctionPtr funcptr) { 
		return !m_Closure.IsEqualToStaticFuncPtr(funcptr);    }
	inline bool operator ! () const	{	// Is it bound to anything?
			return !m_Closure; }
	inline bool empty() const	{
			return !m_Closure; }
	void clear() { m_Closure.clear();}
	// Conversion to and from the delegate_memento storage class
	const delegate_memento & get_memento()const { return m_Closure; }
	void set_memento(const delegate_memento &any) { m_Closure.CopyFrom(this, any); }

private:	// Invoker for static functions
	RetType InvokeStaticFunction() const {
	return (*(m_Closure.GetStaticFunction()))(); }
};

//N=1
template<class Param1, class RetType=detail::DefaultVoid>
class fast_delegate1 {
public:
	typedef typename boost::reflect::detail::default_void_to_void<RetType>::type result_type;
private:
	typedef result_type (*StaticFunctionPtr)(Param1 p1);
	typedef RetType (*UnvoidStaticFunctionPtr)(Param1 p1);
	typedef RetType (boost::reflect::detail::GenericClass::*GenericMemFn)(Param1 p1);
	typedef boost::reflect::detail::ClosurePtr<GenericMemFn, StaticFunctionPtr, UnvoidStaticFunctionPtr> ClosureType;
	ClosureType m_Closure;
public:
	// Typedefs to aid generic programming
	typedef fast_delegate1 type;

	// Construction and comparison functions
	fast_delegate1() { clear(); }
	fast_delegate1(const fast_delegate1 &x) {
		m_Closure.CopyFrom(this, x.m_Closure); }
	void operator = (const fast_delegate1 &x)  {
		m_Closure.CopyFrom(this, x.m_Closure); }
	bool operator ==(const fast_delegate1 &x) const {
		return m_Closure.IsEqual(x.m_Closure);	}
	bool operator !=(const fast_delegate1 &x) const {
		return !m_Closure.IsEqual(x.m_Closure); }
	bool operator <(const fast_delegate1 &x) const {
		return m_Closure.IsLess(x.m_Closure);	}
	bool operator >(const fast_delegate1 &x) const {
		return x.m_Closure.IsLess(m_Closure);	}
	// Binding to non-const member functions
	template < class X, class Y >
	fast_delegate1(Y *pthis, result_type (X::* function_to_bind)(Param1 p1) ) {
		m_Closure.bindmemfunc(boost::reflect::detail::implicit_cast<X*>(pthis), function_to_bind); }
	template < class X, class Y >
	inline void bind(Y *pthis, result_type (X::* function_to_bind)(Param1 p1)) {
		m_Closure.bindmemfunc(boost::reflect::detail::implicit_cast<X*>(pthis), function_to_bind);	}
	// Binding to const member functions.
	template < class X, class Y >
	fast_delegate1(const Y *pthis, result_type (X::* function_to_bind)(Param1 p1) const) {
		m_Closure.bindconstmemfunc(boost::reflect::detail::implicit_cast<const X*>(pthis), function_to_bind);	}
	template < class X, class Y >
	inline void bind(const Y *pthis, result_type (X::* function_to_bind)(Param1 p1) const) {
		m_Closure.bindconstmemfunc(boost::reflect::detail::implicit_cast<const X *>(pthis), function_to_bind);	}
	// Static functions. We convert them into a member function call.
	// This constructor also provides implicit conversion
	fast_delegate1(result_type (*function_to_bind)(Param1 p1) ) {
		bind(function_to_bind);	}
	// for efficiency, prevent creation of a temporary
	void operator = (result_type (*function_to_bind)(Param1 p1) ) {
		bind(function_to_bind);	}
	inline void bind(result_type (*function_to_bind)(Param1 p1)) {
		m_Closure.bindstaticfunc(this, &fast_delegate1::InvokeStaticFunction, 
			function_to_bind); }
	// Invoke the delegate
	RetType operator() (Param1 p1) const {
	return (m_Closure.GetClosureThis()->*(m_Closure.GetClosureMemPtr()))(p1); }
	// Implicit conversion to "bool" using the safe_bool idiom
private:
	typedef struct SafeBoolStruct {
		int a_data_pointer_to_this_is_0_on_buggy_compilers;
		StaticFunctionPtr m_nonzero;
	} UselessTypedef;
    typedef StaticFunctionPtr SafeBoolStruct::*unspecified_bool_type;
public:
	operator unspecified_bool_type() const {
        return empty()? 0: &SafeBoolStruct::m_nonzero;
    }
	// necessary to allow ==0 to work despite the safe_bool idiom
	inline bool operator==(StaticFunctionPtr funcptr) {
		return m_Closure.IsEqualToStaticFuncPtr(funcptr);	}
	inline bool operator!=(StaticFunctionPtr funcptr) { 
		return !m_Closure.IsEqualToStaticFuncPtr(funcptr);    }
	inline bool operator ! () const	{	// Is it bound to anything?
			return !m_Closure; }
	inline bool empty() const	{
			return !m_Closure; }
	void clear() { m_Closure.clear();}
	// Conversion to and from the delegate_memento storage class
	const delegate_memento & get_memento()const { return m_Closure; }
	void set_memento(const delegate_memento &any) { m_Closure.CopyFrom(this, any); }

private:	// Invoker for static functions
	RetType InvokeStaticFunction(Param1 p1) const {
	return (*(m_Closure.GetStaticFunction()))(p1); }
};

//N=2
template<class Param1, class Param2, class RetType=detail::DefaultVoid>
class fast_delegate2 {
public:
	typedef typename boost::reflect::detail::default_void_to_void<RetType>::type result_type;
private:
	typedef result_type (*StaticFunctionPtr)(Param1 p1, Param2 p2);
	typedef RetType (*UnvoidStaticFunctionPtr)(Param1 p1, Param2 p2);
	typedef RetType (boost::reflect::detail::GenericClass::*GenericMemFn)(Param1 p1, Param2 p2);
	typedef boost::reflect::detail::ClosurePtr<GenericMemFn, StaticFunctionPtr, UnvoidStaticFunctionPtr> ClosureType;
	ClosureType m_Closure;
public:
	// Typedefs to aid generic programming
	typedef fast_delegate2 type;

	// Construction and comparison functions
	fast_delegate2() { clear(); }
	fast_delegate2(const fast_delegate2 &x) {
		m_Closure.CopyFrom(this, x.m_Closure); }
	void operator = (const fast_delegate2 &x)  {
		m_Closure.CopyFrom(this, x.m_Closure); }
	bool operator ==(const fast_delegate2 &x) const {
		return m_Closure.IsEqual(x.m_Closure);	}
	bool operator !=(const fast_delegate2 &x) const {
		return !m_Closure.IsEqual(x.m_Closure); }
	bool operator <(const fast_delegate2 &x) const {
		return m_Closure.IsLess(x.m_Closure);	}
	bool operator >(const fast_delegate2 &x) const {
		return x.m_Closure.IsLess(m_Closure);	}
	// Binding to non-const member functions
	template < class X, class Y >
	fast_delegate2(Y *pthis, result_type (X::* function_to_bind)(Param1 p1, Param2 p2) ) {
		m_Closure.bindmemfunc(boost::reflect::detail::implicit_cast<X*>(pthis), function_to_bind); }
	template < class X, class Y >
	inline void bind(Y *pthis, result_type (X::* function_to_bind)(Param1 p1, Param2 p2)) {
		m_Closure.bindmemfunc(boost::reflect::detail::implicit_cast<X*>(pthis), function_to_bind);	}
	// Binding to const member functions.
	template < class X, class Y >
	fast_delegate2(const Y *pthis, result_type (X::* function_to_bind)(Param1 p1, Param2 p2) const) {
		m_Closure.bindconstmemfunc(boost::reflect::detail::implicit_cast<const X*>(pthis), function_to_bind);	}
	template < class X, class Y >
	inline void bind(const Y *pthis, result_type (X::* function_to_bind)(Param1 p1, Param2 p2) const) {
		m_Closure.bindconstmemfunc(boost::reflect::detail::implicit_cast<const X *>(pthis), function_to_bind);	}
	// Static functions. We convert them into a member function call.
	// This constructor also provides implicit conversion
	fast_delegate2(result_type (*function_to_bind)(Param1 p1, Param2 p2) ) {
		bind(function_to_bind);	}
	// for efficiency, prevent creation of a temporary
	void operator = (result_type (*function_to_bind)(Param1 p1, Param2 p2) ) {
		bind(function_to_bind);	}
	inline void bind(result_type (*function_to_bind)(Param1 p1, Param2 p2)) {
		m_Closure.bindstaticfunc(this, &fast_delegate2::InvokeStaticFunction, 
			function_to_bind); }
	// Invoke the delegate
	RetType operator() (Param1 p1, Param2 p2) const {
	return (m_Closure.GetClosureThis()->*(m_Closure.GetClosureMemPtr()))(p1, p2); }
	// Implicit conversion to "bool" using the safe_bool idiom
private:
	typedef struct SafeBoolStruct {
		int a_data_pointer_to_this_is_0_on_buggy_compilers;
		StaticFunctionPtr m_nonzero;
	} UselessTypedef;
    typedef StaticFunctionPtr SafeBoolStruct::*unspecified_bool_type;
public:
	operator unspecified_bool_type() const {
        return empty()? 0: &SafeBoolStruct::m_nonzero;
    }
	// necessary to allow ==0 to work despite the safe_bool idiom
	inline bool operator==(StaticFunctionPtr funcptr) {
		return m_Closure.IsEqualToStaticFuncPtr(funcptr);	}
	inline bool operator!=(StaticFunctionPtr funcptr) { 
		return !m_Closure.IsEqualToStaticFuncPtr(funcptr);    }
	inline bool operator ! () const	{	// Is it bound to anything?
			return !m_Closure; }
	inline bool empty() const	{
			return !m_Closure; }
	void clear() { m_Closure.clear();}
	// Conversion to and from the delegate_memento storage class
	const delegate_memento & get_memento()const { return m_Closure; }
	void set_memento(const delegate_memento &any) { m_Closure.CopyFrom(this, any); }

private:	// Invoker for static functions
	RetType InvokeStaticFunction(Param1 p1, Param2 p2) const {
	return (*(m_Closure.GetStaticFunction()))(p1, p2); }
};

//N=3
template<class Param1, class Param2, class Param3, class RetType=detail::DefaultVoid>
class fast_delegate3 {
public:
	typedef typename boost::reflect::detail::default_void_to_void<RetType>::type result_type;
private:
	typedef result_type (*StaticFunctionPtr)(Param1 p1, Param2 p2, Param3 p3);
	typedef RetType (*UnvoidStaticFunctionPtr)(Param1 p1, Param2 p2, Param3 p3);
	typedef RetType (boost::reflect::detail::GenericClass::*GenericMemFn)(Param1 p1, Param2 p2, Param3 p3);
	typedef boost::reflect::detail::ClosurePtr<GenericMemFn, StaticFunctionPtr, UnvoidStaticFunctionPtr> ClosureType;
	ClosureType m_Closure;
public:
	// Typedefs to aid generic programming
	typedef fast_delegate3 type;

	// Construction and comparison functions
	fast_delegate3() { clear(); }
	fast_delegate3(const fast_delegate3 &x) {
		m_Closure.CopyFrom(this, x.m_Closure); }
	void operator = (const fast_delegate3 &x)  {
		m_Closure.CopyFrom(this, x.m_Closure); }
	bool operator ==(const fast_delegate3 &x) const {
		return m_Closure.IsEqual(x.m_Closure);	}
	bool operator !=(const fast_delegate3 &x) const {
		return !m_Closure.IsEqual(x.m_Closure); }
	bool operator <(const fast_delegate3 &x) const {
		return m_Closure.IsLess(x.m_Closure);	}
	bool operator >(const fast_delegate3 &x) const {
		return x.m_Closure.IsLess(m_Closure);	}
	// Binding to non-const member functions
	template < class X, class Y >
	fast_delegate3(Y *pthis, result_type (X::* function_to_bind)(Param1 p1, Param2 p2, Param3 p3) ) {
		m_Closure.bindmemfunc(boost::reflect::detail::implicit_cast<X*>(pthis), function_to_bind); }
	template < class X, class Y >
	inline void bind(Y *pthis, result_type (X::* function_to_bind)(Param1 p1, Param2 p2, Param3 p3)) {
		m_Closure.bindmemfunc(boost::reflect::detail::implicit_cast<X*>(pthis), function_to_bind);	}
	// Binding to const member functions.
	template < class X, class Y >
	fast_delegate3(const Y *pthis, result_type (X::* function_to_bind)(Param1 p1, Param2 p2, Param3 p3) const) {
		m_Closure.bindconstmemfunc(boost::reflect::detail::implicit_cast<const X*>(pthis), function_to_bind);	}
	template < class X, class Y >
	inline void bind(const Y *pthis, result_type (X::* function_to_bind)(Param1 p1, Param2 p2, Param3 p3) const) {
		m_Closure.bindconstmemfunc(boost::reflect::detail::implicit_cast<const X *>(pthis), function_to_bind);	}
	// Static functions. We convert them into a member function call.
	// This constructor also provides implicit conversion
	fast_delegate3(result_type (*function_to_bind)(Param1 p1, Param2 p2, Param3 p3) ) {
		bind(function_to_bind);	}
	// for efficiency, prevent creation of a temporary
	void operator = (result_type (*function_to_bind)(Param1 p1, Param2 p2, Param3 p3) ) {
		bind(function_to_bind);	}
	inline void bind(result_type (*function_to_bind)(Param1 p1, Param2 p2, Param3 p3)) {
		m_Closure.bindstaticfunc(this, &fast_delegate3::InvokeStaticFunction, 
			function_to_bind); }
	// Invoke the delegate
	RetType operator() (Param1 p1, Param2 p2, Param3 p3) const {
	return (m_Closure.GetClosureThis()->*(m_Closure.GetClosureMemPtr()))(p1, p2, p3); }
	// Implicit conversion to "bool" using the safe_bool idiom
private:
	typedef struct SafeBoolStruct {
		int a_data_pointer_to_this_is_0_on_buggy_compilers;
		StaticFunctionPtr m_nonzero;
	} UselessTypedef;
    typedef StaticFunctionPtr SafeBoolStruct::*unspecified_bool_type;
public:
	operator unspecified_bool_type() const {
        return empty()? 0: &SafeBoolStruct::m_nonzero;
    }
	// necessary to allow ==0 to work despite the safe_bool idiom
	inline bool operator==(StaticFunctionPtr funcptr) {
		return m_Closure.IsEqualToStaticFuncPtr(funcptr);	}
	inline bool operator!=(StaticFunctionPtr funcptr) { 
		return !m_Closure.IsEqualToStaticFuncPtr(funcptr);    }
	inline bool operator ! () const	{	// Is it bound to anything?
			return !m_Closure; }
	inline bool empty() const	{
			return !m_Closure; }
	void clear() { m_Closure.clear();}
	// Conversion to and from the delegate_memento storage class
	const delegate_memento & get_memento()const { return m_Closure; }
	void set_memento(const delegate_memento &any) { m_Closure.CopyFrom(this, any); }

private:	// Invoker for static functions
	RetType InvokeStaticFunction(Param1 p1, Param2 p2, Param3 p3) const {
	return (*(m_Closure.GetStaticFunction()))(p1, p2, p3); }
};

//N=4
template<class Param1, class Param2, class Param3, class Param4, class RetType=detail::DefaultVoid>
class fast_delegate4 {
public:
	typedef typename boost::reflect::detail::default_void_to_void<RetType>::type result_type;
private:
	typedef result_type (*StaticFunctionPtr)(Param1 p1, Param2 p2, Param3 p3, Param4 p4);
	typedef RetType (*UnvoidStaticFunctionPtr)(Param1 p1, Param2 p2, Param3 p3, Param4 p4);
	typedef RetType (boost::reflect::detail::GenericClass::*GenericMemFn)(Param1 p1, Param2 p2, Param3 p3, Param4 p4);
	typedef boost::reflect::detail::ClosurePtr<GenericMemFn, StaticFunctionPtr, UnvoidStaticFunctionPtr> ClosureType;
	ClosureType m_Closure;
public:
	// Typedefs to aid generic programming
	typedef fast_delegate4 type;

	// Construction and comparison functions
	fast_delegate4() { clear(); }
	fast_delegate4(const fast_delegate4 &x) {
		m_Closure.CopyFrom(this, x.m_Closure); }
	void operator = (const fast_delegate4 &x)  {
		m_Closure.CopyFrom(this, x.m_Closure); }
	bool operator ==(const fast_delegate4 &x) const {
		return m_Closure.IsEqual(x.m_Closure);	}
	bool operator !=(const fast_delegate4 &x) const {
		return !m_Closure.IsEqual(x.m_Closure); }
	bool operator <(const fast_delegate4 &x) const {
		return m_Closure.IsLess(x.m_Closure);	}
	bool operator >(const fast_delegate4 &x) const {
		return x.m_Closure.IsLess(m_Closure);	}
	// Binding to non-const member functions
	template < class X, class Y >
	fast_delegate4(Y *pthis, result_type (X::* function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4) ) {
		m_Closure.bindmemfunc(boost::reflect::detail::implicit_cast<X*>(pthis), function_to_bind); }
	template < class X, class Y >
	inline void bind(Y *pthis, result_type (X::* function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4)) {
		m_Closure.bindmemfunc(boost::reflect::detail::implicit_cast<X*>(pthis), function_to_bind);	}
	// Binding to const member functions.
	template < class X, class Y >
	fast_delegate4(const Y *pthis, result_type (X::* function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4) const) {
		m_Closure.bindconstmemfunc(boost::reflect::detail::implicit_cast<const X*>(pthis), function_to_bind);	}
	template < class X, class Y >
	inline void bind(const Y *pthis, result_type (X::* function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4) const) {
		m_Closure.bindconstmemfunc(boost::reflect::detail::implicit_cast<const X *>(pthis), function_to_bind);	}
	// Static functions. We convert them into a member function call.
	// This constructor also provides implicit conversion
	fast_delegate4(result_type (*function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4) ) {
		bind(function_to_bind);	}
	// for efficiency, prevent creation of a temporary
	void operator = (result_type (*function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4) ) {
		bind(function_to_bind);	}
	inline void bind(result_type (*function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4)) {
		m_Closure.bindstaticfunc(this, &fast_delegate4::InvokeStaticFunction, 
			function_to_bind); }
	// Invoke the delegate
	RetType operator() (Param1 p1, Param2 p2, Param3 p3, Param4 p4) const {
	return (m_Closure.GetClosureThis()->*(m_Closure.GetClosureMemPtr()))(p1, p2, p3, p4); }
	// Implicit conversion to "bool" using the safe_bool idiom
private:
	typedef struct SafeBoolStruct {
		int a_data_pointer_to_this_is_0_on_buggy_compilers;
		StaticFunctionPtr m_nonzero;
	} UselessTypedef;
    typedef StaticFunctionPtr SafeBoolStruct::*unspecified_bool_type;
public:
	operator unspecified_bool_type() const {
        return empty()? 0: &SafeBoolStruct::m_nonzero;
    }
	// necessary to allow ==0 to work despite the safe_bool idiom
	inline bool operator==(StaticFunctionPtr funcptr) {
		return m_Closure.IsEqualToStaticFuncPtr(funcptr);	}
	inline bool operator!=(StaticFunctionPtr funcptr) { 
		return !m_Closure.IsEqualToStaticFuncPtr(funcptr);    }
	inline bool operator ! () const	{	// Is it bound to anything?
			return !m_Closure; }
	inline bool empty() const	{
			return !m_Closure; }
	void clear() { m_Closure.clear();}
	// Conversion to and from the delegate_memento storage class
	const delegate_memento & get_memento()const { return m_Closure; }
	void set_memento(const delegate_memento &any) { m_Closure.CopyFrom(this, any); }

private:	// Invoker for static functions
	RetType InvokeStaticFunction(Param1 p1, Param2 p2, Param3 p3, Param4 p4) const {
	return (*(m_Closure.GetStaticFunction()))(p1, p2, p3, p4); }
};

//N=5
template<class Param1, class Param2, class Param3, class Param4, class Param5, class RetType=detail::DefaultVoid>
class fast_delegate5 {
public:
	typedef typename boost::reflect::detail::default_void_to_void<RetType>::type result_type;
private:
	typedef result_type (*StaticFunctionPtr)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5);
	typedef RetType (*UnvoidStaticFunctionPtr)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5);
	typedef RetType (boost::reflect::detail::GenericClass::*GenericMemFn)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5);
	typedef boost::reflect::detail::ClosurePtr<GenericMemFn, StaticFunctionPtr, UnvoidStaticFunctionPtr> ClosureType;
	ClosureType m_Closure;
public:
	// Typedefs to aid generic programming
	typedef fast_delegate5 type;

	// Construction and comparison functions
	fast_delegate5() { clear(); }
	fast_delegate5(const fast_delegate5 &x) {
		m_Closure.CopyFrom(this, x.m_Closure); }
	void operator = (const fast_delegate5 &x)  {
		m_Closure.CopyFrom(this, x.m_Closure); }
	bool operator ==(const fast_delegate5 &x) const {
		return m_Closure.IsEqual(x.m_Closure);	}
	bool operator !=(const fast_delegate5 &x) const {
		return !m_Closure.IsEqual(x.m_Closure); }
	bool operator <(const fast_delegate5 &x) const {
		return m_Closure.IsLess(x.m_Closure);	}
	bool operator >(const fast_delegate5 &x) const {
		return x.m_Closure.IsLess(m_Closure);	}
	// Binding to non-const member functions
	template < class X, class Y >
	fast_delegate5(Y *pthis, result_type (X::* function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) ) {
		m_Closure.bindmemfunc(boost::reflect::detail::implicit_cast<X*>(pthis), function_to_bind); }
	template < class X, class Y >
	inline void bind(Y *pthis, result_type (X::* function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5)) {
		m_Closure.bindmemfunc(boost::reflect::detail::implicit_cast<X*>(pthis), function_to_bind);	}
	// Binding to const member functions.
	template < class X, class Y >
	fast_delegate5(const Y *pthis, result_type (X::* function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) const) {
		m_Closure.bindconstmemfunc(boost::reflect::detail::implicit_cast<const X*>(pthis), function_to_bind);	}
	template < class X, class Y >
	inline void bind(const Y *pthis, result_type (X::* function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) const) {
		m_Closure.bindconstmemfunc(boost::reflect::detail::implicit_cast<const X *>(pthis), function_to_bind);	}
	// Static functions. We convert them into a member function call.
	// This constructor also provides implicit conversion
	fast_delegate5(result_type (*function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) ) {
		bind(function_to_bind);	}
	// for efficiency, prevent creation of a temporary
	void operator = (result_type (*function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) ) {
		bind(function_to_bind);	}
	inline void bind(result_type (*function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5)) {
		m_Closure.bindstaticfunc(this, &fast_delegate5::InvokeStaticFunction, 
			function_to_bind); }
	// Invoke the delegate
	RetType operator() (Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) const {
	return (m_Closure.GetClosureThis()->*(m_Closure.GetClosureMemPtr()))(p1, p2, p3, p4, p5); }
	// Implicit conversion to "bool" using the safe_bool idiom
private:
	typedef struct SafeBoolStruct {
		int a_data_pointer_to_this_is_0_on_buggy_compilers;
		StaticFunctionPtr m_nonzero;
	} UselessTypedef;
    typedef StaticFunctionPtr SafeBoolStruct::*unspecified_bool_type;
public:
	operator unspecified_bool_type() const {
        return empty()? 0: &SafeBoolStruct::m_nonzero;
    }
	// necessary to allow ==0 to work despite the safe_bool idiom
	inline bool operator==(StaticFunctionPtr funcptr) {
		return m_Closure.IsEqualToStaticFuncPtr(funcptr);	}
	inline bool operator!=(StaticFunctionPtr funcptr) { 
		return !m_Closure.IsEqualToStaticFuncPtr(funcptr);    }
	inline bool operator ! () const	{	// Is it bound to anything?
			return !m_Closure; }
	inline bool empty() const	{
			return !m_Closure; }
	void clear() { m_Closure.clear();}
	// Conversion to and from the delegate_memento storage class
	const delegate_memento & get_memento()const { return m_Closure; }
	void set_memento(const delegate_memento &any) { m_Closure.CopyFrom(this, any); }

private:	// Invoker for static functions
	RetType InvokeStaticFunction(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) const {
	return (*(m_Closure.GetStaticFunction()))(p1, p2, p3, p4, p5); }
};

//N=6
template<class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class RetType=detail::DefaultVoid>
class fast_delegate6 {
public:
	typedef typename boost::reflect::detail::default_void_to_void<RetType>::type result_type;
private:
	typedef result_type (*StaticFunctionPtr)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6);
	typedef RetType (*UnvoidStaticFunctionPtr)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6);
	typedef RetType (boost::reflect::detail::GenericClass::*GenericMemFn)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6);
	typedef boost::reflect::detail::ClosurePtr<GenericMemFn, StaticFunctionPtr, UnvoidStaticFunctionPtr> ClosureType;
	ClosureType m_Closure;
public:
	// Typedefs to aid generic programming
	typedef fast_delegate6 type;

	// Construction and comparison functions
	fast_delegate6() { clear(); }
	fast_delegate6(const fast_delegate6 &x) {
		m_Closure.CopyFrom(this, x.m_Closure); }
	void operator = (const fast_delegate6 &x)  {
		m_Closure.CopyFrom(this, x.m_Closure); }
	bool operator ==(const fast_delegate6 &x) const {
		return m_Closure.IsEqual(x.m_Closure);	}
	bool operator !=(const fast_delegate6 &x) const {
		return !m_Closure.IsEqual(x.m_Closure); }
	bool operator <(const fast_delegate6 &x) const {
		return m_Closure.IsLess(x.m_Closure);	}
	bool operator >(const fast_delegate6 &x) const {
		return x.m_Closure.IsLess(m_Closure);	}
	// Binding to non-const member functions
	template < class X, class Y >
	fast_delegate6(Y *pthis, result_type (X::* function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6) ) {
		m_Closure.bindmemfunc(boost::reflect::detail::implicit_cast<X*>(pthis), function_to_bind); }
	template < class X, class Y >
	inline void bind(Y *pthis, result_type (X::* function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6)) {
		m_Closure.bindmemfunc(boost::reflect::detail::implicit_cast<X*>(pthis), function_to_bind);	}
	// Binding to const member functions.
	template < class X, class Y >
	fast_delegate6(const Y *pthis, result_type (X::* function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6) const) {
		m_Closure.bindconstmemfunc(boost::reflect::detail::implicit_cast<const X*>(pthis), function_to_bind);	}
	template < class X, class Y >
	inline void bind(const Y *pthis, result_type (X::* function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6) const) {
		m_Closure.bindconstmemfunc(boost::reflect::detail::implicit_cast<const X *>(pthis), function_to_bind);	}
	// Static functions. We convert them into a member function call.
	// This constructor also provides implicit conversion
	fast_delegate6(result_type (*function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6) ) {
		bind(function_to_bind);	}
	// for efficiency, prevent creation of a temporary
	void operator = (result_type (*function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6) ) {
		bind(function_to_bind);	}
	inline void bind(result_type (*function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6)) {
		m_Closure.bindstaticfunc(this, &fast_delegate6::InvokeStaticFunction, 
			function_to_bind); }
	// Invoke the delegate
	RetType operator() (Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6) const {
	return (m_Closure.GetClosureThis()->*(m_Closure.GetClosureMemPtr()))(p1, p2, p3, p4, p5, p6); }
	// Implicit conversion to "bool" using the safe_bool idiom
private:
	typedef struct SafeBoolStruct {
		int a_data_pointer_to_this_is_0_on_buggy_compilers;
		StaticFunctionPtr m_nonzero;
	} UselessTypedef;
    typedef StaticFunctionPtr SafeBoolStruct::*unspecified_bool_type;
public:
	operator unspecified_bool_type() const {
        return empty()? 0: &SafeBoolStruct::m_nonzero;
    }
	// necessary to allow ==0 to work despite the safe_bool idiom
	inline bool operator==(StaticFunctionPtr funcptr) {
		return m_Closure.IsEqualToStaticFuncPtr(funcptr);	}
	inline bool operator!=(StaticFunctionPtr funcptr) { 
		return !m_Closure.IsEqualToStaticFuncPtr(funcptr);    }
	inline bool operator ! () const	{	// Is it bound to anything?
			return !m_Closure; }
	inline bool empty() const	{
			return !m_Closure; }
	void clear() { m_Closure.clear();}
	// Conversion to and from the delegate_memento storage class
	const delegate_memento & get_memento()const { return m_Closure; }
	void set_memento(const delegate_memento &any) { m_Closure.CopyFrom(this, any); }

private:	// Invoker for static functions
	RetType InvokeStaticFunction(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6) const {
	return (*(m_Closure.GetStaticFunction()))(p1, p2, p3, p4, p5, p6); }
};

//N=7
template<class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class RetType=detail::DefaultVoid>
class fast_delegate7 {
public:
	typedef typename boost::reflect::detail::default_void_to_void<RetType>::type result_type;
private:
	typedef result_type (*StaticFunctionPtr)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7);
	typedef RetType (*UnvoidStaticFunctionPtr)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7);
	typedef RetType (boost::reflect::detail::GenericClass::*GenericMemFn)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7);
	typedef boost::reflect::detail::ClosurePtr<GenericMemFn, StaticFunctionPtr, UnvoidStaticFunctionPtr> ClosureType;
	ClosureType m_Closure;
public:
	// Typedefs to aid generic programming
	typedef fast_delegate7 type;

	// Construction and comparison functions
	fast_delegate7() { clear(); }
	fast_delegate7(const fast_delegate7 &x) {
		m_Closure.CopyFrom(this, x.m_Closure); }
	void operator = (const fast_delegate7 &x)  {
		m_Closure.CopyFrom(this, x.m_Closure); }
	bool operator ==(const fast_delegate7 &x) const {
		return m_Closure.IsEqual(x.m_Closure);	}
	bool operator !=(const fast_delegate7 &x) const {
		return !m_Closure.IsEqual(x.m_Closure); }
	bool operator <(const fast_delegate7 &x) const {
		return m_Closure.IsLess(x.m_Closure);	}
	bool operator >(const fast_delegate7 &x) const {
		return x.m_Closure.IsLess(m_Closure);	}
	// Binding to non-const member functions
	template < class X, class Y >
	fast_delegate7(Y *pthis, result_type (X::* function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7) ) {
		m_Closure.bindmemfunc(boost::reflect::detail::implicit_cast<X*>(pthis), function_to_bind); }
	template < class X, class Y >
	inline void bind(Y *pthis, result_type (X::* function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7)) {
		m_Closure.bindmemfunc(boost::reflect::detail::implicit_cast<X*>(pthis), function_to_bind);	}
	// Binding to const member functions.
	template < class X, class Y >
	fast_delegate7(const Y *pthis, result_type (X::* function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7) const) {
		m_Closure.bindconstmemfunc(boost::reflect::detail::implicit_cast<const X*>(pthis), function_to_bind);	}
	template < class X, class Y >
	inline void bind(const Y *pthis, result_type (X::* function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7) const) {
		m_Closure.bindconstmemfunc(boost::reflect::detail::implicit_cast<const X *>(pthis), function_to_bind);	}
	// Static functions. We convert them into a member function call.
	// This constructor also provides implicit conversion
	fast_delegate7(result_type (*function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7) ) {
		bind(function_to_bind);	}
	// for efficiency, prevent creation of a temporary
	void operator = (result_type (*function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7) ) {
		bind(function_to_bind);	}
	inline void bind(result_type (*function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7)) {
		m_Closure.bindstaticfunc(this, &fast_delegate7::InvokeStaticFunction, 
			function_to_bind); }
	// Invoke the delegate
	RetType operator() (Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7) const {
	return (m_Closure.GetClosureThis()->*(m_Closure.GetClosureMemPtr()))(p1, p2, p3, p4, p5, p6, p7); }
	// Implicit conversion to "bool" using the safe_bool idiom
private:
	typedef struct SafeBoolStruct {
		int a_data_pointer_to_this_is_0_on_buggy_compilers;
		StaticFunctionPtr m_nonzero;
	} UselessTypedef;
    typedef StaticFunctionPtr SafeBoolStruct::*unspecified_bool_type;
public:
	operator unspecified_bool_type() const {
        return empty()? 0: &SafeBoolStruct::m_nonzero;
    }
	// necessary to allow ==0 to work despite the safe_bool idiom
	inline bool operator==(StaticFunctionPtr funcptr) {
		return m_Closure.IsEqualToStaticFuncPtr(funcptr);	}
	inline bool operator!=(StaticFunctionPtr funcptr) { 
		return !m_Closure.IsEqualToStaticFuncPtr(funcptr);    }
	inline bool operator ! () const	{	// Is it bound to anything?
			return !m_Closure; }
	inline bool empty() const	{
			return !m_Closure; }
	void clear() { m_Closure.clear();}
	// Conversion to and from the delegate_memento storage class
	const delegate_memento & get_memento()const { return m_Closure; }
	void set_memento(const delegate_memento &any) { m_Closure.CopyFrom(this, any); }

private:	// Invoker for static functions
	RetType InvokeStaticFunction(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7) const {
	return (*(m_Closure.GetStaticFunction()))(p1, p2, p3, p4, p5, p6, p7); }
};

//N=8
template<class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class RetType=detail::DefaultVoid>
class fast_delegate8 {
public:
	typedef typename boost::reflect::detail::default_void_to_void<RetType>::type result_type;
private:
	typedef result_type (*StaticFunctionPtr)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8);
	typedef RetType (*UnvoidStaticFunctionPtr)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8);
	typedef RetType (boost::reflect::detail::GenericClass::*GenericMemFn)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8);
	typedef boost::reflect::detail::ClosurePtr<GenericMemFn, StaticFunctionPtr, UnvoidStaticFunctionPtr> ClosureType;
	ClosureType m_Closure;
public:
	// Typedefs to aid generic programming
	typedef fast_delegate8 type;

	// Construction and comparison functions
	fast_delegate8() { clear(); }
	fast_delegate8(const fast_delegate8 &x) {
		m_Closure.CopyFrom(this, x.m_Closure); }
	void operator = (const fast_delegate8 &x)  {
		m_Closure.CopyFrom(this, x.m_Closure); }
	bool operator ==(const fast_delegate8 &x) const {
		return m_Closure.IsEqual(x.m_Closure);	}
	bool operator !=(const fast_delegate8 &x) const {
		return !m_Closure.IsEqual(x.m_Closure); }
	bool operator <(const fast_delegate8 &x) const {
		return m_Closure.IsLess(x.m_Closure);	}
	bool operator >(const fast_delegate8 &x) const {
		return x.m_Closure.IsLess(m_Closure);	}
	// Binding to non-const member functions
	template < class X, class Y >
	fast_delegate8(Y *pthis, result_type (X::* function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8) ) {
		m_Closure.bindmemfunc(boost::reflect::detail::implicit_cast<X*>(pthis), function_to_bind); }
	template < class X, class Y >
	inline void bind(Y *pthis, result_type (X::* function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8)) {
		m_Closure.bindmemfunc(boost::reflect::detail::implicit_cast<X*>(pthis), function_to_bind);	}
	// Binding to const member functions.
	template < class X, class Y >
	fast_delegate8(const Y *pthis, result_type (X::* function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8) const) {
		m_Closure.bindconstmemfunc(boost::reflect::detail::implicit_cast<const X*>(pthis), function_to_bind);	}
	template < class X, class Y >
	inline void bind(const Y *pthis, result_type (X::* function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8) const) {
		m_Closure.bindconstmemfunc(boost::reflect::detail::implicit_cast<const X *>(pthis), function_to_bind);	}
	// Static functions. We convert them into a member function call.
	// This constructor also provides implicit conversion
	fast_delegate8(result_type (*function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8) ) {
		bind(function_to_bind);	}
	// for efficiency, prevent creation of a temporary
	void operator = (result_type (*function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8) ) {
		bind(function_to_bind);	}
	inline void bind(result_type (*function_to_bind)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8)) {
		m_Closure.bindstaticfunc(this, &fast_delegate8::InvokeStaticFunction, 
			function_to_bind); }
	// Invoke the delegate
	RetType operator() (Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8) const {
	return (m_Closure.GetClosureThis()->*(m_Closure.GetClosureMemPtr()))(p1, p2, p3, p4, p5, p6, p7, p8); }
	// Implicit conversion to "bool" using the safe_bool idiom
private:
	typedef struct SafeBoolStruct {
		int a_data_pointer_to_this_is_0_on_buggy_compilers;
		StaticFunctionPtr m_nonzero;
	} UselessTypedef;
    typedef StaticFunctionPtr SafeBoolStruct::*unspecified_bool_type;
public:
	operator unspecified_bool_type() const {
        return empty()? 0: &SafeBoolStruct::m_nonzero;
    }
	// necessary to allow ==0 to work despite the safe_bool idiom
	inline bool operator==(StaticFunctionPtr funcptr) {
		return m_Closure.IsEqualToStaticFuncPtr(funcptr);	}
	inline bool operator!=(StaticFunctionPtr funcptr) { 
		return !m_Closure.IsEqualToStaticFuncPtr(funcptr);    }
	inline bool operator ! () const	{	// Is it bound to anything?
			return !m_Closure; }
	inline bool empty() const	{
			return !m_Closure; }
	void clear() { m_Closure.clear();}
	// Conversion to and from the delegate_memento storage class
	const delegate_memento & get_memento()const { return m_Closure; }
	void set_memento(const delegate_memento &any) { m_Closure.CopyFrom(this, any); }

private:	// Invoker for static functions
	RetType InvokeStaticFunction(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8) const {
	return (*(m_Closure.GetStaticFunction()))(p1, p2, p3, p4, p5, p6, p7, p8); }
};


////////////////////////////////////////////////////////////////////////////////
//						Fast Delegates, part 4:
// 
//				fast_delegate<> class (Original author: Jody Hagins)
//	Allows boost::function style syntax like:
//			fast_delegate< double (int, long) >
// instead of:
//			fast_delegate2< int, long, double >
//
////////////////////////////////////////////////////////////////////////////////

#ifdef FASTDELEGATE_ALLOW_FUNCTION_TYPE_SYNTAX
/// @endcond FASTDELEGATE

// Declare fast_delegate as a class template.  It will be specialized
// later for all number of arguments.

/**
    @class fd::fast_delegate
	@brief Fast, type-safe, member-function callbacks. 

    Allows boost::function style syntax like:
            fast_delegate< double (int, long) >
    instead of: <br/>
			fast_delegate2< int, long, double >

	fast_delegate provides efficient delegates in C++ that generate only two lines of asm code!
    Documentation is found at http://www.codeproject.com/cpp/fast_delegate.asp

    @section why_not_boost_function Why not use Boost.Function?
    
    The fast_delegate implementation was chosen over boost::function<> because boost::function<> depended upon RTTI and
    heap allocations.  RTTI was unacceptable for embedded applications and heap allocations were
    unacceptable for both embedded and high-performance active object implementations that are
    creating and destroying function pointers on a regular basis.

 
    @section fast_delegate_licences Fast Delegate License

    fast_delegate is licensed under <a href="http://www.codeproject.com/info/cpol10.aspx">The Code Project Open License</a>:
    <hr/>
    This License governs Your use of the Work. This License is intended to allow developers to use the Source Code and Executable Files provided as part of the Work in any application in any form.

    The main points subject to the terms of the License are:

        - Source Code and Executable Files can be used in commercial applications;
        - Source Code and Executable Files can be redistributed; and
        - Source Code can be modified to create derivative works.
        - No claim of suitability, guarantee, or any warranty whatsoever is provided. The software is provided "as-is".
        - The Article(s) accompanying the Work may not be distributed or republished without the Author's consent

    This License is entered between You, the individual or other entity reading or otherwise making use of the Work licensed pursuant to this License and the individual or other entity which offers the Work under the terms of this License ("Author").
    <hr/>

    @author (Original author: Jody Hagins)
*/
template <typename Signature>
class fast_delegate;

/// @cond FASTDELEGATE

//N=0
// Specialization to allow use of
// fast_delegate< R (  ) >
// instead of 
// fast_delegate0 < R >
template<typename R>
class fast_delegate< R (  ) >
  // Inherit from fast_delegate0 so that it can be treated just like a fast_delegate0
  : public fast_delegate0 < R >
{
public:
  // Make using the base type a bit easier via typedef.
  typedef fast_delegate0 < R > BaseType;

  // Allow users access to the specific type of this delegate.
  typedef fast_delegate SelfType;

  // Mimic the base class constructors.
  fast_delegate() : BaseType() { }

  template < class X, class Y >
  fast_delegate(Y * pthis, 
    R (X::* function_to_bind)(  ))
    : BaseType(pthis, function_to_bind)  { }

  template < class X, class Y >
  fast_delegate(const Y *pthis,
      R (X::* function_to_bind)(  ) const)
    : BaseType(pthis, function_to_bind)
  {  }

  fast_delegate(R (*function_to_bind)(  ))
    : BaseType(function_to_bind)  { }
  void operator = (const BaseType &x)  {	  
		*static_cast<BaseType*>(this) = x; }
};

//N=1
// Specialization to allow use of
// fast_delegate< R ( Param1 ) >
// instead of 
// fast_delegate1 < Param1, R >
template<typename R, class Param1>
class fast_delegate< R ( Param1 ) >
  // Inherit from fast_delegate1 so that it can be treated just like a fast_delegate1
  : public fast_delegate1 < Param1, R >
{
public:
  // Make using the base type a bit easier via typedef.
  typedef fast_delegate1 < Param1, R > BaseType;

  // Allow users access to the specific type of this delegate.
  typedef fast_delegate SelfType;

  // Mimic the base class constructors.
  fast_delegate() : BaseType() { }

  template < class X, class Y >
  fast_delegate(Y * pthis, 
    R (X::* function_to_bind)( Param1 p1 ))
    : BaseType(pthis, function_to_bind)  { }

  template < class X, class Y >
  fast_delegate(const Y *pthis,
      R (X::* function_to_bind)( Param1 p1 ) const)
    : BaseType(pthis, function_to_bind)
  {  }

  fast_delegate(R (*function_to_bind)( Param1 p1 ))
    : BaseType(function_to_bind)  { }
  void operator = (const BaseType &x)  {	  
		*static_cast<BaseType*>(this) = x; }
};

//N=2
// Specialization to allow use of
// fast_delegate< R ( Param1, Param2 ) >
// instead of 
// fast_delegate2 < Param1, Param2, R >
template<typename R, class Param1, class Param2>
class fast_delegate< R ( Param1, Param2 ) >
  // Inherit from fast_delegate2 so that it can be treated just like a fast_delegate2
  : public fast_delegate2 < Param1, Param2, R >
{
public:
  // Make using the base type a bit easier via typedef.
  typedef fast_delegate2 < Param1, Param2, R > BaseType;

  // Allow users access to the specific type of this delegate.
  typedef fast_delegate SelfType;

  // Mimic the base class constructors.
  fast_delegate() : BaseType() { }

  template < class X, class Y >
  fast_delegate(Y * pthis, 
    R (X::* function_to_bind)( Param1 p1, Param2 p2 ))
    : BaseType(pthis, function_to_bind)  { }

  template < class X, class Y >
  fast_delegate(const Y *pthis,
      R (X::* function_to_bind)( Param1 p1, Param2 p2 ) const)
    : BaseType(pthis, function_to_bind)
  {  }

  fast_delegate(R (*function_to_bind)( Param1 p1, Param2 p2 ))
    : BaseType(function_to_bind)  { }
  void operator = (const BaseType &x)  {	  
		*static_cast<BaseType*>(this) = x; }
};

//N=3
// Specialization to allow use of
// fast_delegate< R ( Param1, Param2, Param3 ) >
// instead of 
// fast_delegate3 < Param1, Param2, Param3, R >
template<typename R, class Param1, class Param2, class Param3>
class fast_delegate< R ( Param1, Param2, Param3 ) >
  // Inherit from fast_delegate3 so that it can be treated just like a fast_delegate3
  : public fast_delegate3 < Param1, Param2, Param3, R >
{
public:
  // Make using the base type a bit easier via typedef.
  typedef fast_delegate3 < Param1, Param2, Param3, R > BaseType;

  // Allow users access to the specific type of this delegate.
  typedef fast_delegate SelfType;

  // Mimic the base class constructors.
  fast_delegate() : BaseType() { }

  template < class X, class Y >
  fast_delegate(Y * pthis, 
    R (X::* function_to_bind)( Param1 p1, Param2 p2, Param3 p3 ))
    : BaseType(pthis, function_to_bind)  { }

  template < class X, class Y >
  fast_delegate(const Y *pthis,
      R (X::* function_to_bind)( Param1 p1, Param2 p2, Param3 p3 ) const)
    : BaseType(pthis, function_to_bind)
  {  }

  fast_delegate(R (*function_to_bind)( Param1 p1, Param2 p2, Param3 p3 ))
    : BaseType(function_to_bind)  { }
  void operator = (const BaseType &x)  {	  
		*static_cast<BaseType*>(this) = x; }
};

//N=4
// Specialization to allow use of
// fast_delegate< R ( Param1, Param2, Param3, Param4 ) >
// instead of 
// fast_delegate4 < Param1, Param2, Param3, Param4, R >
template<typename R, class Param1, class Param2, class Param3, class Param4>
class fast_delegate< R ( Param1, Param2, Param3, Param4 ) >
  // Inherit from fast_delegate4 so that it can be treated just like a fast_delegate4
  : public fast_delegate4 < Param1, Param2, Param3, Param4, R >
{
public:
  // Make using the base type a bit easier via typedef.
  typedef fast_delegate4 < Param1, Param2, Param3, Param4, R > BaseType;

  // Allow users access to the specific type of this delegate.
  typedef fast_delegate SelfType;

  // Mimic the base class constructors.
  fast_delegate() : BaseType() { }

  template < class X, class Y >
  fast_delegate(Y * pthis, 
    R (X::* function_to_bind)( Param1 p1, Param2 p2, Param3 p3, Param4 p4 ))
    : BaseType(pthis, function_to_bind)  { }

  template < class X, class Y >
  fast_delegate(const Y *pthis,
      R (X::* function_to_bind)( Param1 p1, Param2 p2, Param3 p3, Param4 p4 ) const)
    : BaseType(pthis, function_to_bind)
  {  }

  fast_delegate(R (*function_to_bind)( Param1 p1, Param2 p2, Param3 p3, Param4 p4 ))
    : BaseType(function_to_bind)  { }
  void operator = (const BaseType &x)  {	  
		*static_cast<BaseType*>(this) = x; }
};

//N=5
// Specialization to allow use of
// fast_delegate< R ( Param1, Param2, Param3, Param4, Param5 ) >
// instead of 
// fast_delegate5 < Param1, Param2, Param3, Param4, Param5, R >
template<typename R, class Param1, class Param2, class Param3, class Param4, class Param5>
class fast_delegate< R ( Param1, Param2, Param3, Param4, Param5 ) >
  // Inherit from fast_delegate5 so that it can be treated just like a fast_delegate5
  : public fast_delegate5 < Param1, Param2, Param3, Param4, Param5, R >
{
public:
  // Make using the base type a bit easier via typedef.
  typedef fast_delegate5 < Param1, Param2, Param3, Param4, Param5, R > BaseType;

  // Allow users access to the specific type of this delegate.
  typedef fast_delegate SelfType;

  // Mimic the base class constructors.
  fast_delegate() : BaseType() { }

  template < class X, class Y >
  fast_delegate(Y * pthis, 
    R (X::* function_to_bind)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5 ))
    : BaseType(pthis, function_to_bind)  { }

  template < class X, class Y >
  fast_delegate(const Y *pthis,
      R (X::* function_to_bind)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5 ) const)
    : BaseType(pthis, function_to_bind)
  {  }

  fast_delegate(R (*function_to_bind)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5 ))
    : BaseType(function_to_bind)  { }
  void operator = (const BaseType &x)  {	  
		*static_cast<BaseType*>(this) = x; }
};

//N=6
// Specialization to allow use of
// fast_delegate< R ( Param1, Param2, Param3, Param4, Param5, Param6 ) >
// instead of 
// fast_delegate6 < Param1, Param2, Param3, Param4, Param5, Param6, R >
template<typename R, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6>
class fast_delegate< R ( Param1, Param2, Param3, Param4, Param5, Param6 ) >
  // Inherit from fast_delegate6 so that it can be treated just like a fast_delegate6
  : public fast_delegate6 < Param1, Param2, Param3, Param4, Param5, Param6, R >
{
public:
  // Make using the base type a bit easier via typedef.
  typedef fast_delegate6 < Param1, Param2, Param3, Param4, Param5, Param6, R > BaseType;

  // Allow users access to the specific type of this delegate.
  typedef fast_delegate SelfType;

  // Mimic the base class constructors.
  fast_delegate() : BaseType() { }

  template < class X, class Y >
  fast_delegate(Y * pthis, 
    R (X::* function_to_bind)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6 ))
    : BaseType(pthis, function_to_bind)  { }

  template < class X, class Y >
  fast_delegate(const Y *pthis,
      R (X::* function_to_bind)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6 ) const)
    : BaseType(pthis, function_to_bind)
  {  }

  fast_delegate(R (*function_to_bind)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6 ))
    : BaseType(function_to_bind)  { }
  void operator = (const BaseType &x)  {	  
		*static_cast<BaseType*>(this) = x; }
};

//N=7
// Specialization to allow use of
// fast_delegate< R ( Param1, Param2, Param3, Param4, Param5, Param6, Param7 ) >
// instead of 
// fast_delegate7 < Param1, Param2, Param3, Param4, Param5, Param6, Param7, R >
template<typename R, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7>
class fast_delegate< R ( Param1, Param2, Param3, Param4, Param5, Param6, Param7 ) >
  // Inherit from fast_delegate7 so that it can be treated just like a fast_delegate7
  : public fast_delegate7 < Param1, Param2, Param3, Param4, Param5, Param6, Param7, R >
{
public:
  // Make using the base type a bit easier via typedef.
  typedef fast_delegate7 < Param1, Param2, Param3, Param4, Param5, Param6, Param7, R > BaseType;

  // Allow users access to the specific type of this delegate.
  typedef fast_delegate SelfType;

  // Mimic the base class constructors.
  fast_delegate() : BaseType() { }

  template < class X, class Y >
  fast_delegate(Y * pthis, 
    R (X::* function_to_bind)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7 ))
    : BaseType(pthis, function_to_bind)  { }

  template < class X, class Y >
  fast_delegate(const Y *pthis,
      R (X::* function_to_bind)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7 ) const)
    : BaseType(pthis, function_to_bind)
  {  }

  fast_delegate(R (*function_to_bind)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7 ))
    : BaseType(function_to_bind)  { }
  void operator = (const BaseType &x)  {	  
		*static_cast<BaseType*>(this) = x; }
};

//N=8
// Specialization to allow use of
// fast_delegate< R ( Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8 ) >
// instead of 
// fast_delegate8 < Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, R >
template<typename R, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8>
class fast_delegate< R ( Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8 ) >
  // Inherit from fast_delegate8 so that it can be treated just like a fast_delegate8
  : public fast_delegate8 < Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, R >
{
public:
  // Make using the base type a bit easier via typedef.
  typedef fast_delegate8 < Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, R > BaseType;

  // Allow users access to the specific type of this delegate.
  typedef fast_delegate SelfType;
  typedef R result_type;

  // Mimic the base class constructors.
  fast_delegate() : BaseType() { }

  template < class X, class Y >
  fast_delegate(Y * pthis, 
    R (X::* function_to_bind)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8 ))
    : BaseType(pthis, function_to_bind)  { }

  template < class X, class Y >
  fast_delegate(const Y *pthis,
      R (X::* function_to_bind)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8 ) const)
    : BaseType(pthis, function_to_bind)
  {  }

  fast_delegate(R (*function_to_bind)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8 ))
    : BaseType(function_to_bind)  { }
  void operator = (const BaseType &x)  {	  
		*static_cast<BaseType*>(this) = x; }
};


#endif //FASTDELEGATE_ALLOW_FUNCTION_TYPE_SYNTAX

////////////////////////////////////////////////////////////////////////////////
//						Fast Delegates, part 5:
//
//				make_fast_delegate() helper function
//
//			make_fast_delegate(&x, &X::func) returns a fastdelegate of the type
//			necessary for calling x.func() with the correct number of arguments.
//			This makes it possible to eliminate many typedefs from user code.
//
////////////////////////////////////////////////////////////////////////////////

// Also declare overloads of a make_fast_delegate() global function to 
// reduce the need for typedefs.
// We need seperate overloads for const and non-const member functions.
// Also, because of the weird rule about the class of derived member function pointers,
// implicit downcasts may need to be applied later to the 'this' pointer.
// That's why two classes (X and Y) appear in the definitions. Y must be implicitly
// castable to X.

// Workaround for VC6. VC6 needs void return types converted into DefaultVoid.
// GCC 3.2 and later won't compile this unless it's preceded by 'typename',
// but VC6 doesn't allow 'typename' in this context.
// So, I have to use a macro.

#ifdef FASTDLGT_VC6
#define FASTDLGT_RETTYPE detail::void_to_default_void<RetType>::type
#else 
#define FASTDLGT_RETTYPE RetType
#endif

//N=0
template <class X, class Y, class RetType>
fast_delegate<FASTDLGT_RETTYPE()> make_fast_delegate(Y* x, RetType (X::*func)()) { 
	return fast_delegate<FASTDLGT_RETTYPE()>(x, func);
}

template <class X, class Y, class RetType>
fast_delegate<FASTDLGT_RETTYPE()> make_fast_delegate(Y* x, RetType (X::*func)() const) { 
	return fast_delegate<FASTDLGT_RETTYPE()>(x, func);
}

//N=1
template <class X, class Y, class Param1, class RetType>
fast_delegate<FASTDLGT_RETTYPE(Param1)> make_fast_delegate(Y* x, RetType (X::*func)(Param1 p1)) { 
	return fast_delegate<FASTDLGT_RETTYPE(Param1)>(x, func);
}

template <class X, class Y, class Param1, class RetType>
fast_delegate1<FASTDLGT_RETTYPE(Param1)> make_fast_delegate(Y* x, RetType (X::*func)(Param1 p1) const) { 
	return fast_delegate<FASTDLGT_RETTYPE(Param1)>(x, func);
}

//N=2
template <class X, class Y, class Param1, class Param2, class RetType>
fast_delegate<FASTDLGT_RETTYPE(Param1, Param2)> make_fast_delegate(Y* x, RetType (X::*func)(Param1 p1, Param2 p2)) { 
	return fast_delegate<FASTDLGT_RETTYPE(Param1, Param2)>(x, func);
}

template <class X, class Y, class Param1, class Param2, class RetType>
fast_delegate<FASTDLGT_RETTYPE(Param1, Param2)> make_fast_delegate(Y* x, RetType (X::*func)(Param1 p1, Param2 p2) const) { 
	return fast_delegate<FASTDLGT_RETTYPE(Param1, Param2)>(x, func);
}

//N=3
//template <class X, class Y, class Param1, class Param2, class Param3, class RetType>
//fast_delegate3<Param1, Param2, Param3, FASTDLGT_RETTYPE> make_fast_delegate(Y* x, RetType (X::*func)(Param1 p1, Param2 p2, Param3 p3)) { 
//	return fast_delegate3<Param1, Param2, Param3, FASTDLGT_RETTYPE>(x, func);
//}
template <class X, class Y, class Param1, class Param2, class Param3, class RetType>
fast_delegate<FASTDLGT_RETTYPE (Param1, Param2, Param3)> make_fast_delegate(Y* x, RetType (X::*func)(Param1 p1, Param2 p2, Param3 p3)) { 
	return fast_delegate<FASTDLGT_RETTYPE(Param1, Param2, Param3)>(x, func);
}

template <class X, class Y, class Param1, class Param2, class Param3, class RetType>
fast_delegate3<Param1, Param2, Param3, FASTDLGT_RETTYPE> make_fast_delegate(Y* x, RetType (X::*func)(Param1 p1, Param2 p2, Param3 p3) const) { 
	return fast_delegate3<Param1, Param2, Param3, FASTDLGT_RETTYPE>(x, func);
}

//N=4
template <class X, class Y, class Param1, class Param2, class Param3, class Param4, class RetType>
fast_delegate4<Param1, Param2, Param3, Param4, FASTDLGT_RETTYPE> make_fast_delegate(Y* x, RetType (X::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4)) { 
	return fast_delegate4<Param1, Param2, Param3, Param4, FASTDLGT_RETTYPE>(x, func);
}

template <class X, class Y, class Param1, class Param2, class Param3, class Param4, class RetType>
fast_delegate4<Param1, Param2, Param3, Param4, FASTDLGT_RETTYPE> make_fast_delegate(Y* x, RetType (X::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4) const) { 
	return fast_delegate4<Param1, Param2, Param3, Param4, FASTDLGT_RETTYPE>(x, func);
}

//N=5
template <class X, class Y, class Param1, class Param2, class Param3, class Param4, class Param5, class RetType>
fast_delegate5<Param1, Param2, Param3, Param4, Param5, FASTDLGT_RETTYPE> make_fast_delegate(Y* x, RetType (X::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5)) { 
	return fast_delegate5<Param1, Param2, Param3, Param4, Param5, FASTDLGT_RETTYPE>(x, func);
}

template <class X, class Y, class Param1, class Param2, class Param3, class Param4, class Param5, class RetType>
fast_delegate5<Param1, Param2, Param3, Param4, Param5, FASTDLGT_RETTYPE> make_fast_delegate(Y* x, RetType (X::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) const) { 
	return fast_delegate5<Param1, Param2, Param3, Param4, Param5, FASTDLGT_RETTYPE>(x, func);
}

//N=6
template <class X, class Y, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class RetType>
fast_delegate6<Param1, Param2, Param3, Param4, Param5, Param6, FASTDLGT_RETTYPE> make_fast_delegate(Y* x, RetType (X::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6)) { 
	return fast_delegate6<Param1, Param2, Param3, Param4, Param5, Param6, FASTDLGT_RETTYPE>(x, func);
}

template <class X, class Y, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class RetType>
fast_delegate6<Param1, Param2, Param3, Param4, Param5, Param6, FASTDLGT_RETTYPE> make_fast_delegate(Y* x, RetType (X::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6) const) { 
	return fast_delegate6<Param1, Param2, Param3, Param4, Param5, Param6, FASTDLGT_RETTYPE>(x, func);
}

//N=7
template <class X, class Y, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class RetType>
fast_delegate7<Param1, Param2, Param3, Param4, Param5, Param6, Param7, FASTDLGT_RETTYPE> make_fast_delegate(Y* x, RetType (X::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7)) { 
	return fast_delegate7<Param1, Param2, Param3, Param4, Param5, Param6, Param7, FASTDLGT_RETTYPE>(x, func);
}

template <class X, class Y, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class RetType>
fast_delegate7<Param1, Param2, Param3, Param4, Param5, Param6, Param7, FASTDLGT_RETTYPE> make_fast_delegate(Y* x, RetType (X::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7) const) { 
	return fast_delegate7<Param1, Param2, Param3, Param4, Param5, Param6, Param7, FASTDLGT_RETTYPE>(x, func);
}

//N=8
template <class X, class Y, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class RetType>
fast_delegate8<Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, FASTDLGT_RETTYPE> make_fast_delegate(Y* x, RetType (X::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8)) { 
	return fast_delegate8<Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, FASTDLGT_RETTYPE>(x, func);
}

template <class X, class Y, class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class RetType>
fast_delegate8<Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, FASTDLGT_RETTYPE> make_fast_delegate(Y* x, RetType (X::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8) const) { 
	return fast_delegate8<Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, FASTDLGT_RETTYPE>(x, func);
}


 // clean up after ourselves...
#undef FASTDLGT_RETTYPE

/// @endcond
} } // namespace boost::reflect
#endif // !defined(FASTDELEGATE_H)


