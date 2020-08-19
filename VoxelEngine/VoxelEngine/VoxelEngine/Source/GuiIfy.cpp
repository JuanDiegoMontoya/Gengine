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
}