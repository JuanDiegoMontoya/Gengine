/*HEADER_GOES_HERE*/
#include "../../Headers/Containers/EventManger.h"

EventManager::EventManager() : eventDeque(eventQueue.Container())
{
  Init();
}

EventManager::~EventManager()
{
  End();
}

void EventManager::Init()
{
}

void EventManager::Update(float dt)
{
  if (newListenersAdded > 0)  //If there's new listeners
  {
    bool exitLoop = false;
    for (auto i : listeners)  //for every type of listner
    {
      for (auto j : i)  //for every listener of that type
      {
        if (j->bufferUsage) //check if it's being buffered
        {
          j->bufferUsage = false;     //if it is, remove it's buffer
          --newListenersAdded;        //account that it has been found.
          if (newListenersAdded == 0) //check for early out.
          {
            exitLoop = true;
            break;
          }
        }
      }
      if (exitLoop)
        break;
    }
  }

  for (auto& i : eventDeque) //For every event update the timer
    if (!eventListenerActiveToggles[i->type]) //don't decrease timers of events when queue is paused
      i->timer -= dt;

  while (eventQueue.size() > 0) //While there are events in the queue
  {
    Event* i(&*eventQueue.top()); //Grab the next event.
    if (i->timer >= 0)             //If it still needs time, stop
      break;
    DispatchEvent(i);             //If it is time to dispatch, do so
    eventQueue.pop();             //Pop the pointer
  }

}

void EventManager::End()
{
  for (std::vector<EventManager::ListenerBase*>& i : listeners) //Clear all the listeners, delete all the callback wrappers.
  {
    for (EventManager::ListenerBase* j : i)
    {
      delete j;
    }
    i.clear();
    i.shrink_to_fit();
    i.~vector();
  }
  ;
}

void EventManager::DispatchEvent(Event* event_)
{
  int eventTypeIndex = event_->type;

  if (event_->OnUpdate)
  {
    event_->OnUpdate();
  }

  for (unsigned i = 0; i < (listeners[eventTypeIndex]).size(); ++i)
  {
    ListenerBase* listener = listeners[eventTypeIndex][i];
    if (listener->bufferUsage)     //If the listener was jussst created
      return;             //Skip it


    (*listener)(&*event_);  //Send the event
    if (event_->destroy == true)  //If the listener wants to cut off the event from being sent to other listeners, do so.
      return;
  }
}
