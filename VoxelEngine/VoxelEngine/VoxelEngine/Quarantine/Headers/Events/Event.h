/*HEADER_GOES_HERE*/
#ifndef Event_Guard
#define Event_Guard
//#pragma warning( disable : 4100 )

#include "AllEventHeaders.h"
#include "../FactoryID.h"

#include <functional>
#include <string>
#include <utility>
#include <memory>
#include <boost/preprocessor/cat.hpp>

  //Used if an event is not to held over time.
#define HANDLE_INSTANTLY -1 
#define HANDLE_NEXT_FRAME 0 

#define GenerateEvent(eventType) BOOST_PP_CAT(eventType, BOOST_PP_CAT(::, BOOST_PP_CAT(Generate, BOOST_PP_CAT(eventType, ()))))



class Event //Base class for an event to be inherited.
{
public:
  static const FactoryID factoryID = FactoryID::cEvent;
  const ID type;

  virtual ~Event() {}
  virtual std::string GetName() = 0;
  virtual std::unique_ptr<Event> Clone() = 0;

  void Update(float dt) { timer -= dt; }


  float timer;
  std::function<void()> OnUpdate;

  bool destroy = false; //Allows for short circuiting event process, i.e. event is scheduled to sent to OBJ 1-8, but 5 stops it from going on to 6-8
protected:
  Event(const ID type_, float timer_ = HANDLE_INSTANTLY) : type(type_), timer(timer_) {}

};

#endif //Event_Guard