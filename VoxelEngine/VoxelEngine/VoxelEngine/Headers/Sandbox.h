#pragma once

//#define SANDBOX

//#include <boost/preprocessor/slot/counter.hpp>
//#include <boost/preprocessor/cat.hpp>
//#include <boost/preprocessor/repetition.hpp>
//#include <boost/preprocessor/punctuation/comma_if.hpp>
//#include <boost/preprocessor/stringize.hpp>
//#include <boost/preprocessor/seq.hpp>
//#include <boost/preprocessor/variadic.hpp>
//
//#include <iostream>
//#include <string>
//#include <vector>

//#define _PPSTUFF_OUTVAR1(_var) BOOST_PP_STRINGIZE(_var) " = " << (_var) << std::endl
//#define _PPSTUFF_OUTVAR2(r, d, _var) << _PPSTUFF_OUTVAR1(_var) 
//#define _PPSTUFF_OUTVAR_SEQ(vseq) _PPSTUFF_OUTVAR1(BOOST_PP_SEQ_HEAD(vseq)) \
//        BOOST_PP_SEQ_FOR_EACH(_PPSTUFF_OUTVAR2,,BOOST_PP_SEQ_TAIL(vseq)) 
//#define OUTVAR(...) _PPSTUFF_OUTVAR_SEQ(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))
//
//#define DO_SOMETHING_COOL(Name, ...) Name, __VA_ARGS__)

//ENUM(triggers,
//cStarted,
//cActive,
//cEnded,
//cInactive
//);
//
//std::vector<std::string> triggers =  
//{
//  "cStarted",
//  "cActive",
//  "cEnded",
//  "cInactive"
//}
//enoom cStarted(0, triggers
//
//

void Sandbox()
{
  //std::string tester[5] = {
  //  DO_SOMETHING_COOL("a", "b", "c", "d"), "e"
  //};


  //int a = 3;
  //char b[] = "foo";

  //std::cout << OUTVAR(a);

  //// Expands to: 
  ////
  //// std::cout << "a" " = " << (a ) << std::endl  ;
  ////
  //// Output:
  ////
  //// a = 3

  //std::cout << OUTVAR(a, b);

  // Expands to: 
  //
  // std::cout << "a" " = " << (a ) << std::endl << "b" " = " << (b) << std::endl  ;
  //
  // Output:
  //
  // a = 3
  // b = foo

}