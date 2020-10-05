/*HEADER_GOES_HERE*/
#ifndef Factory_Guard
#define Factory_Guard
#include <map>
#include <vector>
#include <any>
#include <memory>

#include "FactoryID.h"
#include "../Headers/Events/Event.h"
#include "../Headers/Systems/System.h"
#include "../Headers/Containers/Space.h"
#include "../Headers/Containers/Object.h"
#include "../Headers/Containers/BidirectionalMap.h"

#define CLONE_COMPONENT(type) Factory::CloneSystem< type >( type ::systemType)
#define CLONE_EVENT(type) Factory::CloneEvent< type >( type ::eventType)

typedef class Object Object;
typedef class Space Space;
typedef class Factory Factory;

class Memory
{
public:
  uint8_t* data = nullptr; //!< Pointer to data.
  uint32_t size = 0; //!< Data size.

  ~Memory();
private:
  Memory();
  Memory(const Memory&);
  Memory operator=(const Memory&);
  friend class Factory;
};

class Factory
{
public:
  static std::map<std::string, std::vector<PropertyID>> SystemPropertyMap;
  static std::map<std::string, std::vector<PropertyID>> EventPropertyMap;
  static std::map<std::string, std::vector<PropertyID>> ManagerPropertyMap;
  static std::vector<PropertyID> SpaceProperties;
  static std::vector<PropertyID> EngineProperties;
  static std::vector<PropertyID> ObjectProperties;

  static std::map<std::string, std::unique_ptr<Event>> EventNameMap;
  static std::map<std::string, std::unique_ptr<System>> SystemNameMap;

  static std::map<ID, std::unique_ptr<Event>> EventIDMap;
  static std::map<ID, std::unique_ptr<System>> SystemIDMap;

  static std::unique_ptr<Object> emptyObject;
  static std::unique_ptr<Space> emptySpace;

  static void Register();
  static Saveable_ID GenID();

  static const std::shared_ptr<Memory> ReadFile(std::string fileName);
  static void WriteFile(std::string fileName, const uint8_t* data, uint32_t size);


  template<class OWNER, class VAR>
  static VAR& GetPropertyValue(OWNER* OwnerPointer, ID propID)
  {
    std::string ownerName = OwnerPointer->GetName();

    if constexpr (OwnerPointer->factoryID == FactoryID::cSystem)
      for (int i = 0; i < Factory::SystemPropertyMap[ownerName].size(); ++i)
        if (std::get<1>(Factory::SystemPropertyMap[ownerName][i]) == propID)
          return *reinterpret_cast<VAR*>(reinterpret_cast<char*>(OwnerPointer) + std::get<2>(Factory::SystemPropertyMap[ownerName][i]));

    if constexpr (OwnerPointer->factoryID == FactoryID::cEvent)
      for (int i = 0; i < Factory::EventPropertyMap[ownerName].size(); ++i)
        if (std::get<1>(Factory::EventPropertyMap[ownerName][i]) == propID)
          return *reinterpret_cast<VAR*>(reinterpret_cast<char*>(OwnerPointer) + std::get<2>(Factory::EventPropertyMap[ownerName][i]));

    if constexpr (OwnerPointer->factoryID == FactoryID::cManager)
      for (int i = 0; i < Factory::ManagerPropertyMap[ownerName].size(); ++i)
        if (std::get<1>(Factory::ManagerPropertyMap[ownerName][i]) == propID)
          return *reinterpret_cast<VAR*>(reinterpret_cast<char*>(OwnerPointer) + std::get<2>(Factory::ManagerPropertyMap[ownerName][i]));

    if constexpr (OwnerPointer->factoryID == FactoryID::cSpace)
      for (int i = 0; i < Factory::SpaceProperties.size(); ++i)
        if (std::get<1>(Factory::SpaceProperties[i]) == propID)
          return *reinterpret_cast<VAR*>(reinterpret_cast<char*>(OwnerPointer) + std::get<2>(Factory::SpaceProperties[i]));

    if constexpr (OwnerPointer->factoryID == FactoryID::cObject)
      for (int i = 0; i < Factory::ObjectProperties.size(); ++i)
        if (std::get<1>(Factory::ObjectProperties[i]) == propID)
          return *reinterpret_cast<VAR*>(reinterpret_cast<char*>(OwnerPointer) + std::get<2>(Factory::ObjectProperties[i]));

    if constexpr (OwnerPointer->factoryID == FactoryID::cEngine)
      for (int i = 0; i < Factory::EngineProperties.size(); ++i)
        if (std::get<1>(Factory::EngineProperties[i]) == propID)
          return *reinterpret_cast<VAR*>(reinterpret_cast<char*>(OwnerPointer) + std::get<2>(Factory::EngineProperties[i]));
  }

  template<class OWNER, class VAR>
  static VAR& GetPropertyValue(OWNER* OwnerPointer, std::string propName)
  {
    std::string ownerName = OwnerPointer->GetName();

    if constexpr (OwnerPointer->factoryID == FactoryID::cSystem)
      for (int i = 0; i < Factory::SystemPropertyMap[ownerName].size(); ++i)
        if (std::get<0>(Factory::SystemPropertyMap[ownerName][i]) == propName)
          return *reinterpret_cast<VAR*>(reinterpret_cast<char*>(OwnerPointer) + std::get<2>(Factory::SystemPropertyMap[ownerName][i]));

    if constexpr (OwnerPointer->factoryID == FactoryID::cEvent)
      for (int i = 0; i < Factory::EventPropertyMap[ownerName].size(); ++i)
        if (std::get<0>(Factory::EventPropertyMap[ownerName][i]) == propName)
          return *reinterpret_cast<VAR*>(reinterpret_cast<char*>(OwnerPointer) + std::get<2>(Factory::EventPropertyMap[ownerName][i]));

    if constexpr (OwnerPointer->factoryID == FactoryID::cManager)
      for (int i = 0; i < Factory::ManagerPropertyMap[ownerName].size(); ++i)
        if (std::get<0>(Factory::ManagerPropertyMap[ownerName][i]) == propName)
          return *reinterpret_cast<VAR*>(reinterpret_cast<char*>(OwnerPointer) + std::get<2>(Factory::ManagerPropertyMap[ownerName][i]));

    if constexpr (OwnerPointer->factoryID == FactoryID::cObject)
      for (int i = 0; i < Factory::ObjectProperties.size(); ++i)
        if (std::get<0>(Factory::ObjectProperties[i]) == propName)
          return *reinterpret_cast<VAR*>(reinterpret_cast<char*>(OwnerPointer) + std::get<2>(Factory::ObjectProperties[i]));

    if constexpr (OwnerPointer->factoryID == FactoryID::cSpace)
      for (int i = 0; i < Factory::SpaceProperties.size(); ++i)
        if (std::get<0>(Factory::SpaceProperties[i]) == propName)
          return *reinterpret_cast<VAR*>(reinterpret_cast<char*>(OwnerPointer) + std::get<2>(Factory::SpaceProperties[i]));

    if constexpr (OwnerPointer->factoryID == FactoryID::cEngine)
      for (int i = 0; i < Factory::EngineProperties.size(); ++i)
        if (std::get<0>(Factory::EngineProperties[i]) == propName)
          return *reinterpret_cast<VAR*>(reinterpret_cast<char*>(OwnerPointer) + std::get<2>(Factory::EngineProperties[i]));

    throw("This does not exist in the factory");

  }

  template<class OWNER, class VAR>
  static PropertyID GetPropertyID(OWNER* OwnerPointer, std::string propName)
  {
    VAR getIt;
    std::string ownerName = OwnerPointer->GetName();

    if constexpr (OwnerPointer->factoryID == FactoryID::cSystem)
      for (int i = 0; i < Factory::SystemPropertyMap[ownerName].size(); ++i)
        if (std::get<0>(Factory::SystemPropertyMap[ownerName][i]) == propName)
          return Factory::SystemPropertyMap[ownerName][i];

    if constexpr (OwnerPointer->factoryID == FactoryID::cEvent)
      for (int i = 0; i < Factory::EventPropertyMap[ownerName].size(); ++i)
        if (std::get<0>(Factory::EventPropertyMap[ownerName][i]) == propName)
          return Factory::EventPropertyMap[ownerName][i];

    if constexpr (OwnerPointer->factoryID == FactoryID::cManager)
      for (int i = 0; i < Factory::ManagerPropertyMap[ownerName].size(); ++i)
        if (std::get<0>(Factory::ManagerPropertyMap[ownerName][i]) == propName)
          return Factory::ManagerPropertyMap[ownerName][i];

    if constexpr (OwnerPointer->factoryID == FactoryID::cObject)
      for (int i = 0; i < Factory::ObjectProperties.size(); ++i)
        if (std::get<0>(Factory::ObjectProperties[i]) == propName)
          Factory::ObjectProperties[i];

    if constexpr (OwnerPointer->factoryID == FactoryID::cSpace)
      for (int i = 0; i < Factory::SpaceProperties.size(); ++i)
        if (std::get<0>(Factory::SpaceProperties[i]) == propName)
          return Factory::SpaceProperties[i];

    if constexpr (OwnerPointer->factoryID == FactoryID::cEngine)
      for (int i = 0; i < Factory::EngineProperties.size(); ++i)
        if (std::get<0>(Factory::EngineProperties[i]) == propName)
          return Factory::EngineProperties[i];
  }

  //Do not use with Managers or Engines
  template<typename T>
  static std::unique_ptr<T> CloneSystem(ID systemType)
  {
    return std::unique_ptr<T>(static_cast<T*>(Factory::SystemIDMap[systemType]->Clone().release()));
  }
  template<typename T>
  static std::unique_ptr<T> CloneEvent(ID eventType)
  {
    return std::unique_ptr<T>(static_cast<T*>(Factory::EventIDMap[eventType]->Clone().release()));
  }
  static std::unique_ptr<Space> CloneSpace()
  {
    return std::move(Factory::emptySpace->Clone());
  }
  static Object* CloneObject(Space* space)
  {
    return Factory::emptyObject->Clone(space);
  }

  static std::unique_ptr<System> CloneSystem(ID systemID)
  {
    return std::move(Factory::SystemIDMap[systemID]->Clone());
  }

  static std::unique_ptr<System> CloneSystem(std::string systemName)
  {
    return std::move(Factory::SystemNameMap[systemName]->Clone());
  }

  static std::unique_ptr<Event> CloneEvent(ID eventID)
  {
    return std::move(Factory::EventIDMap[eventID]->Clone());
  }

  static std::unique_ptr<Event> CloneEvent(std::string eventName)
  {
    return std::move(Factory::EventNameMap[eventName]->Clone());
  }

  static void GuiIfy(const void* instance_, PropertyID& p);

private:
  Factory();
};

#endif // !Factory_Guard