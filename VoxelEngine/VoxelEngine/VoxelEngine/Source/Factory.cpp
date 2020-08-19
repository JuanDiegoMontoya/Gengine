/*HEADER_GOES_HERE*/
//This is the all knowing factory
#define FACTORY_RUNNING
#include "../Headers/Systems/AllSystemHeaders.h"
#include "../Headers/Events/AllEventHeaders.h"
#include "../Headers/Components/AllComponentHeaders.h"
#undef FACTORY_RUNNING
#include "../Headers/Containers/EventManger.h"

#include "../Headers/Factory.h"

#include "../Headers/Engine.h"

#include "../Headers/Containers/Space.h"
#include "../Headers/Containers/Object.h"
#include "../Headers/Systems/System.h"

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

std::map<std::string, std::vector<PropertyID>> Factory::ComponentPropertyMap;
std::map<std::string, std::vector<PropertyID>> Factory::SystemPropertyMap;
std::vector<PropertyID> Factory::SpaceProperties;
std::vector<PropertyID> Factory::EngineProperties;
std::vector<PropertyID> Factory::ObjectProperties;

std::map<std::string, std::unique_ptr<Event>> Factory::EventNameMap;
std::map<std::string, std::unique_ptr<Component>> Factory::ComponentNameMap;

std::map<ID, std::unique_ptr<Event>> Factory::EventIDMap;
std::map<ID, std::unique_ptr<Component>> Factory::ComponentIDMap;

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
    memcpy_s(mem->data, mem->size, fileContents.data, mem->size);
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

//Creates one of every component and event for duplication later.
void Factory::Register()
{
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
  //Register all components
  std::vector<std::unique_ptr<Component>(*)()> component_table = { BOOST_PP_REPEAT(COMPONENT_COUNT, MAKE_COMPONENT_FUNCT, ~) };
  for (int i = 0; i < component_table.size(); ++i)
  {
    auto pCreatedComponent = component_table[i]();
#ifdef _DEBUG
    std::cout << "Component_" << i << " included: This is the " << pCreatedComponent->GetName() << " file\n";
#endif
    ComponentIDMap[pCreatedComponent->type] = std::move(pCreatedComponent->Clone());
    ComponentNameMap[pCreatedComponent->GetName()] = std::move(pCreatedComponent);
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
