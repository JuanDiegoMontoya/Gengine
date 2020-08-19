// CAUTION. THERE MAY BE ISSUES WITH PROPERTIES THAT USE STRINGS CONTAINING PARENTHESIS OR COMMAS. IF THAT HAPPENS I NEED TO REWRITE SOME SHTUFF HERE.
// CAUTION 2, THERE MUST BE A SPACE BEFORE THE WORD PROPERTY, OTHERWISE I ASSUME IT IS COMMENTED OUT.

#include <iostream>
#include <chrono>
#include <thread>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

namespace fs = std::filesystem;
using namespace std::chrono_literals;

std::string ws2s(const std::wstring& wstr)
{
  std::string str;
  for (char x : wstr)
    str += x;
  return str;
}

int main()
{
  int counter = 0;
  std::string output = "";
  std::string headers = "";
  std::string componentFileIDs = "";
  std::string eventFileIDs = "";
  std::string systemFileIDs = "";
    //Each System
  //include the header file
  //Find and store every property
    //Each System
      //Create a Register Function for Systems
  systemFileIDs += "//SystemIDs\n";

  for (auto& p : fs::directory_iterator("../VoxelEngine/Headers/Systems"))
  {
    std::wstring filepath(p.path());
    size_t startOfName = filepath.find_last_of('\\') + 1;
    std::string file = ws2s(filepath.substr(startOfName, filepath.length() - startOfName - 2));

    if (file == std::string("System") || file == std::string("AllSystemHeaders")) continue;
    std::cout << file.c_str() << '\n';

    headers += std::string("#include \"") + file + ".h\"\n";
    output += std::string("Factory::SystemPropertyMap[\"") + file + "\"] = std::vector<PropertyID>({\n  ";
    systemFileIDs += std::string("const ID c") + file + " = " + std::to_string(counter) + ";\n";
    ++counter;

    std::ifstream f(filepath);
    std::string fileContents;
    if (f) 
    {
      std::ostringstream oss;
      oss << f.rdbuf();
      fileContents = oss.str();

      //search for GuiIfy function
      size_t location = 0;
      while ((location = fileContents.find("PROPERTY", location)) != std::string::npos)
      {
        location = fileContents.find_first_of("(", location) + 1;
        std::string type = fileContents.substr(location, fileContents.find_first_of(",", location) - location);

        location = fileContents.find_first_of(", ", location);
        location = fileContents.find_first_not_of(", ", location);
        std::string name = fileContents.substr(location, fileContents.find_first_of(",", location) - location);

        location = fileContents.find_first_of(", ", location);
        location = fileContents.find_first_not_of(", ", location);
        std::string defaultValue = fileContents.substr(location, fileContents.find_first_of(")", location) - location);

        output += std::string("PropertyID(\"") + name + "\", " + file + "::" + name + "_id" + ", offsetof(" + file + ", " + name + "), sizeof(" + type + ")),\n  ";
      }

    }
    output[output.length() - 2] = ' ';
    output += "});\n";

  }
  output = std::string("\
#ifndef SYSTEM_COUNT\n#define SYSTEM_COUNT ") + std::to_string(counter) + "\n#endif\n\n" +
"#ifdef FACTORY_RUNNING\n\n#include \"../Headers/Factory.h\"\n" + headers + "\n\nvoid RegisterSystems()\n{\n" + output + "}\n\n#endif // !FACTORY_RUNNING";

  std::ofstream ofs;
  ofs.open("../VoxelEngine/Headers/Systems/AllSystemHeaders.h");
  ofs << output;
  ofs.close();

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  output = "";
  headers = "";
  counter = 0;
  std::string registerFunctions = "";
  componentFileIDs += "//ComponentIDs\n";

  for (auto& p : fs::directory_iterator("../VoxelEngine/Headers/Components"))
  {
    std::wstring filepath(p.path());
    size_t startOfName = filepath.find_last_of('\\') + 1;
    std::string file = ws2s(filepath.substr(startOfName, filepath.length() - startOfName - 2));

    if (file == std::string("Component") || file == std::string("AllComponentHeaders")) continue;
    std::cout << file.c_str() << '\n';

    headers += std::string("#include \"") + file + ".h\"\n";
    output += std::string("Factory::ComponentPropertyMap[\"") + file + "\"] = std::vector<PropertyID>({\n  ";
    registerFunctions += std::string("inline std::unique_ptr<Component> Component") + std::to_string(counter) + "() { return std::move(" + file +  "::Register" + file + "()); }\n";
    componentFileIDs += std::string("const ID c") + file + " = " + std::to_string(counter) + ";\n";
    ++counter;

    std::ifstream f(filepath);
    std::string fileContents;
    if (f)
    {
      std::ostringstream oss;
      oss << f.rdbuf();
      fileContents = oss.str();

      //search for GuiIfy function
      size_t location = 0;
      while ((location = fileContents.find(" PROPERTY", location)) != std::string::npos)
      {
        location = fileContents.find_first_of("(", location) + 1;
        std::string type = fileContents.substr(location, fileContents.find_first_of(",", location) - location);

        location = fileContents.find_first_of(", ", location);
        location = fileContents.find_first_not_of(", ", location);
        std::string name = fileContents.substr(location, fileContents.find_first_of(",", location) - location);

        location = fileContents.find_first_of(", ", location);
        location = fileContents.find_first_not_of(", ", location);
        std::string defaultValue = fileContents.substr(location, fileContents.find_first_of(")", location) - location);

        output += std::string("PropertyID(\"") + name + "\", " + file + "::" + name + "_id" + ", offsetof(" + file + ", " + name + "), sizeof(" + type + ")),\n  ";
      }

    }
    output[output.length() - 2] = ' ';
    output += "});\n";

  }
  output = std::string("\
#ifndef COMPONENT_COUNT\n#define COMPONENT_COUNT ") + std::to_string(counter) + "\n#endif\n\n" +
"#ifdef FACTORY_RUNNING\n\n#include \"../Headers/Factory.h\"\n" + headers + "\n\nvoid RegisterComponents()\n{\n" + output + "}\n\n" + 
registerFunctions + "\n\n#endif // !FACTORY_RUNNING";

  ofs;
  ofs.open("../VoxelEngine/Headers/Components/AllComponentHeaders.h");
  ofs << output;
  ofs.close();

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  output = "";
  headers = "";
  registerFunctions = "";
  counter = 0;

  eventFileIDs += "//EventIDs\n";

  for (auto& p : fs::directory_iterator("../VoxelEngine/Headers/Events"))
  {
    std::wstring filepath(p.path());
    size_t startOfName = filepath.find_last_of('\\') + 1;
    std::string file = ws2s(filepath.substr(startOfName, filepath.length() - startOfName - 2));

    if (file == std::string("Event") || file == std::string("AllEventHeaders")) continue;
    std::cout << file.c_str() << '\n';

    headers += std::string("#include \"") + file + ".h\"\n";
    output += std::string("Factory::EventPropertyMap[\"") + file + "\"] = std::vector<PropertyID>({\n  ";
    registerFunctions += std::string("inline std::unique_ptr<Event> Event") + std::to_string(counter) + "() { return std::move(" + file + "::Register" + file + "()); }\n";
    eventFileIDs += std::string("const ID c") + file + " = " + std::to_string(counter) + ";\n";
    ++counter;

    std::ifstream f(filepath);
    std::string fileContents;
    if (f)
    {
      std::ostringstream oss;
      oss << f.rdbuf();
      fileContents = oss.str();

      //search for GuiIfy function
      size_t location = 0;
      while ((location = fileContents.find("PROPERTY", location)) != std::string::npos)
      {
        location = fileContents.find_first_of("(", location) + 1;
        std::string type = fileContents.substr(location, fileContents.find_first_of(",", location) - location);

        location = fileContents.find_first_of(", ", location);
        location = fileContents.find_first_not_of(", ", location);
        std::string name = fileContents.substr(location, fileContents.find_first_of(",", location) - location);

        location = fileContents.find_first_of(", ", location);
        location = fileContents.find_first_not_of(", ", location);
        std::string defaultValue = fileContents.substr(location, fileContents.find_first_of(")", location) - location);

        output += std::string("PropertyID(\"") + name + "\", " + file + "::" + name + "_id" + ", offsetof(" + file + ", " + name + "), sizeof(" + type + ")),\n  ";
      }

    }
    output[output.length() - 2] = ' ';
    output += "});\n";

  }
  output = std::string("\
#ifndef EVENT_COUNT\n#define EVENT_COUNT ") + std::to_string(counter) + "\n#endif\n\n" +
"#ifdef FACTORY_RUNNING\n\n#include \"../Headers/Factory.h\"\n" + headers + "\n\nvoid RegisterEvents()\n{\n" + output + "}\n\n" +
registerFunctions + "\n\n#endif // !FACTORY_RUNNING";

  ofs;
  ofs.open("../VoxelEngine/Headers/Events/AllEventHeaders.h");
  ofs << output;
  ofs.close();

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  counter = 0;
    //Every File
      //Find and create and else if for every #ifdef GUIFY Piece of code

  output = "\
#include \"../Headers/Factory.h\"                           \n\
#include <sstream>                                          \n\
#include <imgui/imgui.h>                                    \n\
void Factory::GuiIfy(const void * instance_, PropertyID& p) \n\
{                                                           \n\
std::string name = std::get<0>(p);                          \n\
ID id = std::get<1>(p);                                     \n\
size_t offset = std::get<2>(p);                             \n\
size_t type = std::get<3>(p);                               \n\
char* instance = (char*)instance_;                          \n\
instance += offset;                                         \n";

  bool firstPass = true;


  for (auto& p : fs::recursive_directory_iterator("../VoxelEngine/Headers"))
  {
    if (p.is_directory()) continue;

    std::wstring filepath(p.path());
    size_t startOfName = filepath.find_last_of('\\') + 1;
    std::wstring file = filepath.substr(startOfName, filepath.length() - startOfName - 2);

    if (file == L"Factory") continue;

    std::wcout << file.c_str() << '\n';

    //Load in file
    std::ifstream f(filepath); //taking file as inputstream
    std::string fileContents;
    if (f) {
      std::ostringstream oss;
      oss << f.rdbuf(); // reading data
      fileContents = oss.str();

      //search for GuiIfy function
      size_t location = 0;
      while ((location = fileContents.find("void GuiIfy", location)) != std::string::npos && fileContents[location - 1] != '/')
      {
        location = fileContents.find_first_of("(", location) + 1;
        std::string type = fileContents.substr(location, fileContents.find_first_of("*", location) - location);

        location = fileContents.find_first_of("{", location) + 1;

        int bracketCounter = 1;
        size_t endLocation = location;
        while (bracketCounter > 0 && (endLocation = fileContents.find_first_of("{}", endLocation)) != std::string::npos)
          bracketCounter += (fileContents[endLocation++] == '{') ? 1 : -1;
        output += std::string(firstPass ? "if" : "else if") + "(type == typeid(" + type + ").hash_code())\n{" + fileContents.substr(location, endLocation - location) + "\n\n";

        firstPass = false;
      }

    }

  }
  if(output.size() != 669) //magic number for number of characters when else if is not needed as there is nothing that can be guiified
    output += "else \n  throw(\"this type is not yet compatible with GUI, please add it to properties\"); \n\n";
  output += "}";
  
  ofs.open("../VoxelEngine/Source/GuiIfy.cpp");
  ofs << output;
  ofs.close();

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  output = std::string("                                           \
#ifndef FactoryID_Guard                                          \n\
#define FactoryID_Guard                                          \n\
                                                                 \n\
#include <memory>                                                \n\
                                                                 \n\
#include \"PreProcessorMagic.h\"                                 \n\
#include \"Containers/Properties.h\"                             \n\
                                                                 \n\
                                                                 \n\
enum class FactoryID                                             \n\
  {                                                              \n\
    cEngine,                                                     \n\
    cSystem,                                                     \n\
    cSpace,                                                      \n\
    cObject,                                                     \n\
    cComponent,                                                  \n\
    cEvent,                                                      \n\
    cCount                                                       \n\
  };                                                             \n\
                                                                 \n\
                                                                 \n") + 
systemFileIDs + "\n" + componentFileIDs + "\n" + eventFileIDs + "\n\
#endif // !FactoryID_Guard                                       \n\
  ";

  ofs.open("../VoxelEngine/Headers/FactoryID.h");
  ofs << output;
  ofs.close();

  //std::this_thread::sleep_for(1s);
  return 0;
}

//C:\Users\colto\Desktop\VoxelEngine\VoxelEngine\x64\Release\PrePP.exe










///*HEADER_GOES_HERE*/
//#ifndef FactoryID_Guard
//#define FactoryID_Guard
//
//#include <memory>
//
//#include "PreProcessorMagic.h"
//#include "Containers/Properties.h"
//
//
//enum class FactoryID
//{
//  cEngine,
//  cSystem,
//  cSpace,
//  cObject,
//  cComponent,
//  cEvent,
//  cCount
//};
//
//#endif // !FactoryID_Guard
