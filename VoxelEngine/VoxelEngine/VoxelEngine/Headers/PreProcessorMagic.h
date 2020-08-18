/*HEADER_GOES_HERE*/
#ifndef PreProcessorMagicIncluded
#define PreProcessorMagicIncluded

#include <boost/preprocessor/slot/counter.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>

//A level of indirection is required for stringifying macros
#define xxstr(s) #s
#define xstr(s) xxstr(s)

//A level of indirection is also required for concatinating macros
#define MACRO9(x) ComponentIDs/Component_##x
#define MACRO8(x) MACRO9(x)

#define MACRO11(x) EventIDs/Event_##x
#define MACRO10(x) MACRO11(x)

#define MACRO3(s) s##.h
#define MACRO2(s) MACRO3(s)

//This is the next header to see if we can include
#define COMP_HEADER_FILE xstr(MACRO8(MACRO2(BOOST_PP_COUNTER)))
#define EVENT_HEADER_FILE xstr(MACRO10(MACRO2(BOOST_PP_COUNTER)))

//Yet another indirection for concatination
#define MACRO5(s) Component##s
#define MACRO4(s) MACRO5(s)

//This is a way to create a unique function name that can be called from another file without even knowing it's name directly
#define REG_COMPONENT (MACRO4(BOOST_PP_COUNTER))

//Yet another indirection for concatination
#define MACRO13(s) Event##s
#define MACRO12(s) MACRO13(s)

//This is a way to create a unique function name that can be called from another file without even knowing it's name directly
#define REG_EVENT (MACRO12(BOOST_PP_COUNTER))

//Callback "function" for generating the list of function pointers
#define MAKE_COMPONENT_FUNCT(z, n, unused) BOOST_PP_COMMA_IF(n) &Component##n
#define MAKE_EVENT_FUNCT(z, n, unused) BOOST_PP_COMMA_IF(n) &Event##n

//#define str(s) #s
//#define xstr(s) str(s)
//#define MACRO7(s) NowLookAtMeAgain_##s
//#define MACRO6(s) MACRO7(s)
//#define VooDoo xstr(MACRO6(__COUNTER__))
//
//const char* NowLookAtMeAgain = VooDoo;
#endif