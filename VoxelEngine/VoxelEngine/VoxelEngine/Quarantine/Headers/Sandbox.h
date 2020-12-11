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

//void Sandbox()
//{
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

//}

#include "../3rdParty/entt/src/entt.hpp"
#include <cstdint>

struct position {
  float x;
  float y;
};

struct velocity {
  float dx;
  float dy;
};

void update(entt::registry& registry) {
  auto view = registry.view<position, velocity>();

  for (auto entity : view) {
    // gets only the systems that are going to be used ...

    auto& vel = view.get<velocity>(entity);

    vel.dx = 0.;
    vel.dy = 0.;

    // ...
  }
}

void update(std::uint64_t dt, entt::registry& registry) {
  registry.view<position, velocity>().each([dt](auto& pos, auto& vel) {
    // gets all the systems of the view at once ...

    pos.x += vel.dx * dt;
    pos.y += vel.dy * dt;

    // ...
    });
}


void Sandbox()
{
  entt::registry registry;
  std::uint64_t dt = 16;

  for (auto i = 0; i < 10; ++i) {
    auto entity = registry.create();
    registry.emplace<position>(entity, i * 1.f, i * 1.f);
    if (i % 2 == 0) { registry.emplace<velocity>(entity, i * .1f, i * .1f); }
  }

  update(dt, registry);
  update(registry);
}