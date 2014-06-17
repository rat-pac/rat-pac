#ifndef OS_FIXES_HPP
#define OS_FIXES_HPP
/*------------------------------------------------------------------------------

  Author:    Andy Rushton
  Copyright: (c) Andy Rushton, 2004
  License:   BSD License, see ../docs/license.html

  Contains work arounds for OS or Compiler specific problems to try to make
  them look more alike

  It is strongly recommended that this header be included as the first
  #include in every source file

  ------------------------------------------------------------------------------*/

////////////////////////////////////////////////////////////////////////////////
// Problems with unnecessary or unfixable compiler warnings
////////////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER)
// Microsoft Visual Studio
// shut up the following irritating warnings
//   4275 - VC6, exported class was derived from a class that was not exported
//   4786 - VC6, identifier string exceeded maximum allowable length and was truncated (only affects debugger)
//   4305 - VC6, identifier type was converted to a smaller type
//   4503 - VC6, decorated name was longer than the maximum the compiler allows (only affects debugger)
//   4309 - VC6, type conversion operation caused a constant to exceeded the space allocated for it
//   4290 - VC6, C++ exception specification ignored
//   4800 - VC6, forcing value to bool 'true' or 'false' (performance warning)
//   4675 - VC7.1, "change" in function overload resolution _might_ have altered program
#pragma warning(disable: 4275 4786 4305 4503 4309 4290 4800 4675)
#endif

#if defined(__BORLANDC__)
// Borland
// Shut up the following irritating warnings
//   8022 - A virtual function in a base class is usually overridden by a
//          declaration in a derived class.
//          In this case, a declaration with the same name but different
//          argument types makes the virtual functions inaccessible to further
//          derived classes
//   8008 - Condition is always true.
//          Whenever the compiler encounters a constant comparison that (due to
//          the nature of the value being compared) is always true or false, it
//          issues this warning and evaluates the condition at compile time.
//   8060 - Possibly incorrect assignment.
//          This warning is generated when the compiler encounters an assignment
//          operator as the main operator of a conditional expression (part of
//          an if, while, or do-while statement). This is usually a
//          typographical error for the equality operator.
//   8066 - Unreachable code.
//          A break, continue, goto, or return statement was not followed by a
//          label or the end of a loop or function. The compiler checks while,
//          do, and for loops with a constant test condition, and attempts to
//          recognize loops that can't fall through.
#pragma warn -8022
#pragma warn -8008
#pragma warn -8060
#pragma warn -8066
#endif

////////////////////////////////////////////////////////////////////////////////
// Problems with redefinition of min/max in various different versions of library headers
////////////////////////////////////////////////////////////////////////////////

// The Windoze headers define macros called max/min which conflict with the templates std::max and std::min.
// So, to avoid conflicts, MS removed the std::max/min rather than fixing the problem!
// From Visual Studio .NET (SV7, compiler version 13.00) the STL templates have been added correctly.
// This fix switches off the macros and reinstates the STL templates for earlier versions (SV6).
// Note that this could break MFC applications that rely on the macros (try it and see).

// For MFC compatibility, only undef min and max in non-MFC programs - some bits of MFC
// use macro min/max in headers. For VC7 both the macros and template functions exist
// so there is no real need for the undefs but to it anyway for consistency. So, if
// using VC6 and MFC then template functions will not exist

// I've created extra template function definitions minimum/maximum that avoid all the problems above

#if defined(_MSC_VER) && !defined(_MFC_VER)
#define NOMINMAX
#undef max
#undef min
// replace missing template definitions in VC6
#if defined(_MSC_VER) && (_MSC_VER < 1300)
namespace std 
{
  template<typename T> const T& max(const T& l, const T& r) {return l > r ? l : r;}
  template<typename T> const T& min(const T& l, const T& r) {return l < r ? l : r;}
}
#endif
#endif

template<typename T> const T& maximum(const T& l, const T& r) {return l > r ? l : r;}
template<typename T> const T& minimum(const T& l, const T& r) {return l < r ? l : r;}

////////////////////////////////////////////////////////////////////////////////
// Problem with missing __FUNCTION__ macro
////////////////////////////////////////////////////////////////////////////////
// this macro is used in debugging but was missing in Visual Studio prior to version 7
// it also has a different name in Borland

#if defined(_MSC_VER) && (_MSC_VER < 1300)
#define __FUNCTION__ 0
#endif

#if defined(__BORLANDC__)
#define __FUNCTION__ __FUNC__
#endif

#ifndef __FUNCTION__
#define __FUNCTION__ 0
#endif

////////////////////////////////////////////////////////////////////////////////
// Problems with differences between namespaces
////////////////////////////////////////////////////////////////////////////////

// problem in gcc pre-v3 where the sub-namespaces in std aren't present
// this mean that the statement "using namespace std::rel_ops" created an error because the namespace didn't exist

// I've done a fix here that creates an empty namespace for this case, but I
// do *not* try to move the contents of std::rel_ops into namespace std
// This fix only works if you use "using namespace std::rel_ops" to bring in the template relational operators (e.g. != defined i.t.o. ==)

#if defined(__GNUC__)
namespace std
{
  namespace rel_ops
  {
  }
}
#endif

////////////////////////////////////////////////////////////////////////////////
// Problems with the typename keyword
////////////////////////////////////////////////////////////////////////////////

// There are problems with using the 'typename' keyword. Technically, if you
// use a typedef member of a template class, you need to tell the compiler
// that it is a type name. This is because the compiler cannot work out
// whether a member is a type, a method or a data field at compile time.
// However, support for the typename keyword has traditionally been incomplete
// in both gcc and Visual Studio. I have used macros to try to resolve this
// issue. The macros add the keyword for compiler versions that require it and
// omit it for compiler versions that do not support it

// Typedefs:
// GCC pre-version 3 didn't handle typename in typedefs
//     after version 3, typename is required for a typedef in a template function
// Visual Studio
//     these cases are handled by the definition of the TYPEDEF_TYPENAME macro

// Function Parameters:
// Visual Studio version 7.1 requires a typename in a parameter specification in similarly obscure situations
//     this appears to be specific to VC7.1 (.NET 2003) and after and is not compatible with any gcc version
//     this case is handled by the definition of the PARAMETER_TYPENAME macro

// Template Instantiation Parameters:
// Visual studio cannot hack typename within a template instantiation parameter list
//     this is required by gcc v3.4 and optional before that
//     this case is handled by the definition of the TEMPLATE_TYPENAME macro

#if defined(__GNUC__)

// gcc compiler variants
#if __GNUC__ >= 3
// gcc v3.0 onwards
#define TYPEDEF_TYPENAME typename
#define PARAMETER_TYPENAME typename
#define TEMPLATE_TYPENAME typename
#else
// gcc prior to v3
#define TYPEDEF_TYPENAME
#define PARAMETER_TYPENAME
#define TEMPLATE_TYPENAME
#endif

#else
#if defined(_MSC_VER)

// Microsoft Visual Studio variants
#define TYPEDEF_TYPENAME
#if _MSC_VER >= 1300
// Visual Studio .NET
#define PARAMETER_TYPENAME typename
#else
// Visual Studio version 6
#define PARAMETER_TYPENAME
#endif
#define TEMPLATE_TYPENAME

#endif
#define TEMPLATE_TYPENAME typename
#endif

#ifdef __GNUC__
#define SUN_TYPENAME_HACK typename
#else
#define SUN_TYPENAME_HACK
#endif

////////////////////////////////////////////////////////////////////////////////
// problems with missing functions
////////////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER) || defined(__BORLANDC__)
unsigned sleep(unsigned seconds);
#else
#include <unistd.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// Function for establishing endian-ness
////////////////////////////////////////////////////////////////////////////////
// Different machine architectures store data using different byte orders.
// This is referred to as Big- and Little-Endian Byte Ordering. 
//
// The issue is: where does a pointer to an integer type actually point?
//
// In both conventions, the address points to the left of the word but:
// Big-Endian - The most significant byte is on the left end of a word
// Little-Endian - The least significant byte is on the left end of a word
//
// Bytes are addressed left to right, so in big-endian order byte 0 is the
// msB, whereas in little-endian order byte 0 is the lsB. For example,
// Intel-based machines store data in little-endian byte order so byte 0 is
// the lsB.
//
// This function establishes byte order at run-time

bool little_endian(void);

////////////////////////////////////////////////////////////////////////////////
#endif
