/*HEADER_GOES_HERE*/
#ifndef EventManager_Guard
#define EventManager_Guard

#include <queue>
#include <vector>
#include <array>
#include <functional>
#include <memory>

#include "../Events/Event.h"

template <typename T, typename U>
class priority_queue_adapter : public std::priority_queue<T, std::deque<T>, U>
{
public:

  std::deque<T>& Container() { return this->c; }
};

class comparator_functor
{
public:
  bool operator()(const std::unique_ptr<Event>& lhs, const std::unique_ptr<Event>& rhs)
  {
    return lhs->timer > rhs->timer;
  }
};

class EventManager
{
public: 
  EventManager();
  ~EventManager();

  void Init();
  void Update(float dt);
  void End();

  template <typename T>
  void AttachEvent(std::unique_ptr<T> eventToAttach_)
  {
    Event* eventToAttach = &*eventToAttach_;
    if (eventToAttach->timer == HANDLE_INSTANTLY)
    {
      //override the handle "instantly functionallity" if the channel is paused
      if (eventListenerActiveToggles[eventToAttach->type] == true)
        eventToAttach->timer = 0;
      else
      {
        DispatchEvent(eventToAttach);
        return;
      }
    }

    eventQueue.push(std::move(eventToAttach_));
  }

  template <typename T>
  void AttachEventRef(std::unique_ptr<T>& eventToAttach_)
  {
    Event* eventToAttach = &*eventToAttach_;
    if (eventToAttach->timer == HANDLE_INSTANTLY)
    {
      //override the handle instantly functionallity if the channel is paused
      if (eventListenerActiveToggles[eventToAttach->type] == true)
        eventToAttach->timer = 0;
      else
      {
        DispatchEvent(eventToAttach);
        return;
      }
    }

    eventQueue.push(std::move(eventToAttach_));
  }

  //Registers an object with a listener to the event manager.
  template <typename T, typename E>
  void RegisterListener(T* object_, void(T::* callbackFunction_)(E*))
  {
    Listener<T, E>* listener = new Listener<T, E>(object_, callbackFunction_);
    listeners[E::eventType].push_back(reinterpret_cast<ListenerBase*>(listener));
    ++newListenersAdded;
  }

  //Unregisters an object with a listener from the event manager.
  template <typename T, typename E>
  void UnregisterListener(T* object_, void(T::* callbackFunction_)(E*))
  {
    //For every listener of the given type
    for (int i = 0; i < listeners[E::eventType].size(); ++i)
    {
      //Find the listener
      ListenerBase* listener = listeners[E::eventType][i];
      //If the listener has the object we're looking for
      if (reinterpret_cast<Listener<T, E>*>(listener)->object == object_ && 
          reinterpret_cast<Listener<T, E>*>(listener)->callbackFunction == callbackFunction_)
      {
        delete listener;                                            //Delete it
        listeners[E::eventType].erase(listeners[E::eventType].begin() + i); //Remove it from the list of listeners
        return;
      }
    }
  }

  std::array<bool, EVENT_COUNT> eventListenerActiveToggles = { 0 };

private: 
    //Sends out a given event.
  void DispatchEvent(Event* event_);

  //Base class for a listener wrapper
  class ListenerBase
  {
  public:
    virtual void operator()(Event* event_) = 0; //Needed function to send an event.

    virtual ~ListenerBase() { }

    bool bufferUsage = true;                    //Used to buffer event reception until next frame.
  private:
  };

  //More specific listener
  template<typename T, typename E>
  class Listener : ListenerBase
  {
  public:
    //Constructor
    Listener(T* object_, void (T::* callbackFunction_)(E*))
    {
      //Assign varables
      object = object_;
      callbackFunction = callbackFunction_;
    }

    //Function to send the event
    void operator()(Event* event_)
    {

      E* castedEvent = reinterpret_cast<E*>(event_);  //Specify the type of event
      (object->*callbackFunction)(castedEvent);        //Send the specific event
    }

    T* object;                                  //Object that the listener function belongs to
    void (T::* callbackFunction)(E*) = nullptr;  //Function of the listener

  private:
    Listener();
    Listener(Listener&);

  };

  //Queue of events with access to underlying container.
  priority_queue_adapter < std::unique_ptr<Event>, comparator_functor > eventQueue;

  std::array<std::vector<ListenerBase*>, EVENT_COUNT> listeners;
  std::deque<std::unique_ptr<Event>>& eventDeque;

  int newListenersAdded = 0; //Counter of how many new listeners need to be accounted for at end of update (helps with early out)

};


#endif // !EventManager_Guard