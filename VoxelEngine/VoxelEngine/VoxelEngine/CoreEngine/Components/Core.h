#pragma once
#include <string>

namespace Component
{
  // A human-readable identifier. Not guaranteed to be unique.
  struct Tag
  {
    std::string tag;
  };

  // The entity will be scheduled to be deleted when `remainingSeconds` 
  // reaches 0 and is active.
  // If inactive, `remainingSeconds` will not tick down  and this component 
  // will not trigger deletion.
  struct Lifetime
  {
    float remainingSeconds{};
    bool active{};
  };

  // The entity will be deleted at the beginning of the next update tick.
  // Do not add this component yourself unless you know what you are doing!
  struct ScheduledDeletion
  {
    [[no_unique_address]] char8_t dummy;
  };
}