#include "../Headers/Factory.h"                           
#include <sstream>                                          
#include <imgui/imgui.h>                                    
void Factory::GuiIfy(const void * instance_, PropertyID& p) 
{                                                           
std::string name = std::get<0>(p);                          
ID id = std::get<1>(p);                                     
size_t offset = std::get<2>(p);                             
size_t type = std::get<3>(p);                               
char* instance = (char*)instance_;                          
instance += offset;                                         
if(type == typeid(String).hash_code())
{
  constexpr int bufferSize = 128;
  static char buffer[bufferSize] = { 0 };
  std::string& value = (*(String*)(instance)).value;
  strcpy_s(buffer, bufferSize, value.c_str());
  ImGui::InputText((std::stringstream() << name << "##" << instance).str().c_str(), buffer, bufferSize);
  value = buffer;

}

else if(type == typeid(Int).hash_code())
{
  ImGui::InputInt((std::stringstream() << name + "##" << instance).str().c_str(), &(*(Int*)(instance)).value);
}

else if(type == typeid(Float).hash_code())
{
  ImGui::InputFloat((std::stringstream() << name + "##" << instance).str().c_str(), &(*(Float*)(instance)).value);
}

else if(type == typeid(Bool).hash_code())
{
  ImGui::Checkbox((std::stringstream() << name + "##" << instance).str().c_str(), &(*(Bool*)(instance)).value);
}

else if(type == typeid(Vec2).hash_code())
{
  ImGui::InputFloat2((std::stringstream() << name + "##" << instance).str().c_str(), &((*(Vec2*)(instance)).value[0]));
}

else if(type == typeid(Vec3).hash_code())
{
  ImGui::InputFloat3((std::stringstream() << name + "##" << instance).str().c_str(), &((*(Vec3*)(instance)).value[0]));
}

else if(type == typeid(Vec4).hash_code())
{
  ImGui::InputFloat4((std::stringstream() << name + "##" << instance).str().c_str(), &((*(Vec4*)(instance)).value[0]));
}

else if(type == typeid(Enoom).hash_code())
{
  Enoom& enoom = (*(Enoom*)(instance));
  std::string enoomName = (*(enoom.value.names))[enoom.value.value];
  const char* selectableName = enoomName.c_str();
  if (ImGui::BeginCombo((std::stringstream() << name << "##" << instance).str().c_str(), selectableName))
  {
    int index = 0;
    for (auto& i : *enoom.value.names)
    {
      if (ImGui::Selectable(i.c_str())) { enoom.value.value = index; }
      ++index;
    }
    ImGui::EndCombo();
  }
}

else 
  throw("this type is not yet compatible with GUI, please add it to properties"); 

}