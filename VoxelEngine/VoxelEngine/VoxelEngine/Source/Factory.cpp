/*HEADER_GOES_HERE*/
//This is the all knowing factory
#include "../Headers/Managers/Manager.h"
#include "../Headers/Events/Event.h"
#include "../Headers/Systems/System.h"
#include "../Headers/Containers/EventManger.h"

#include "../Headers/Factory.h"

#include "../Headers/Engine.h"

#include "../Headers/Containers/Space.h"
#include "../Headers/Containers/Object.h"
#include "../Headers/Managers/Manager.h"

#include "../Headers/PreProcessorMagic.h"

#include "imgui/imgui.h"

#ifdef _DEBUG
#include <iostream>
#endif

//#include <bgfx_utils.h>
//#include <bx/file.h>
//#include <entry/entry.h>

#include <vector>
#include <functional>
#include <memory>
#include <sstream>
#include <fstream>

std::map<std::string, std::vector<PropertyID>> Factory::SystemPropertyMap;
std::map<std::string, std::vector<PropertyID>> Factory::EventPropertyMap;
std::map<std::string, std::vector<PropertyID>> Factory::ManagerPropertyMap;
std::vector<PropertyID> Factory::SpaceProperties;
std::vector<PropertyID> Factory::EngineProperties;
std::vector<PropertyID> Factory::ObjectProperties;

std::map<std::string, std::unique_ptr<Event>> Factory::EventNameMap;
std::map<std::string, std::unique_ptr<System>> Factory::SystemNameMap;

std::map<ID, std::unique_ptr<Event>> Factory::EventIDMap;
std::map<ID, std::unique_ptr<System>> Factory::SystemIDMap;

std::unique_ptr<Object> Factory::emptyObject = std::unique_ptr<Object>(new Object("empty"));
std::unique_ptr<Space> Factory::emptySpace = std::unique_ptr<Space>(new Space("empty"));

const std::shared_ptr<Memory> Factory::ReadFile(std::string fileName)
{
  std::ifstream ifs;
  ifs.open(fileName);
  if (ifs.is_open())
  {
    std::string fileContents;
    std::ostringstream oss;
    oss << ifs.rdbuf();
    fileContents = oss.str();

    auto mem = std::shared_ptr<Memory>(new Memory());
    mem->size = (uint32_t)fileContents.length();
    mem->data = new uint8_t[mem->size];
    memcpy_s(mem->data, mem->size, fileContents.data(), mem->size);
    ifs.close();
    return mem;
  }
  else
    throw(std::string("could not load file ") + fileName);

  //bx::FileReaderI* reader = entry::getFileReader();
  //if (bx::open(reader, "test.bin"))
  //{
  //  auto mem = std::shared_ptr<Memory>(new Memory());
  //  mem->size = (uint32_t)bx::getSize(reader);
  //  mem->data = new uint8_t[mem->size];
  //  bx::read(reader, mem->data, mem->size);
  //  bx::close(reader);
  //  return mem;
  //}
  //else
  //  throw(std::string("could not load file ") + fileName);
}
void Factory::WriteFile(std::string fileName, const uint8_t* data, uint32_t size)
{
  std::ofstream ofs;
  ofs.open(fileName);
  ofs.write((const char*)(data), size);
  ofs.close();
  //bx::FileWriterI* writer = entry::getFileWriter();
  //if (bx::open(writer, "test.bin"))
  //{
  //  bx::write(writer, (const void*)(data), size);
  //}
  //bx::close(writer);
}

#define FACTORY_RUNNING
#include "../Headers/Managers/AllManagerHeaders.h"
#include "../Headers/Systems/AllSystemHeaders.h"
#include "../Headers/Events/AllEventHeaders.h"
//Creates one of every system and event for duplication later.
void Factory::Register()
{
  RegisterManagers();
  RegisterSystems();
  RegisterEvents();
  std::vector<std::unique_ptr<Event>(*)()> event_table = { BOOST_PP_REPEAT(EVENT_COUNT, MAKE_EVENT_FUNCT, ~) };
  for (int i = 0; i < event_table.size(); ++i)
  {
    auto pCreatedEvent = event_table[i]();
#ifdef _DEBUG
    std::cout << "Event_" << i << " included:     This is the " << pCreatedEvent->GetName() << " file\n";
#endif
    EventIDMap[pCreatedEvent->type] = std::move(pCreatedEvent->Clone());
    EventNameMap[pCreatedEvent->GetName()] = std::move(pCreatedEvent);
  }
  //Register all systems
  std::vector<std::unique_ptr<System>(*)()> system_table = { BOOST_PP_REPEAT(COMPONENT_COUNT, MAKE_COMPONENT_FUNCT, ~) };
#undef FACTORY_RUNNING
  for (int i = 0; i < system_table.size(); ++i)
  {
    auto pCreatedSystem = system_table[i]();
#ifdef _DEBUG
    std::cout << "System_" << i << " included: This is the " << pCreatedSystem->GetName() << " file\n";
#endif
    SystemIDMap[pCreatedSystem->type] = std::move(pCreatedSystem->Clone());
    SystemNameMap[pCreatedSystem->GetName()] = std::move(pCreatedSystem);
  }
}

Saveable_ID Factory::GenID()
{
  static Saveable_ID current_ID = 0;
  return ++current_ID;
}

Memory::~Memory()
{
  delete[] data;
}

Memory::Memory()
{
}
