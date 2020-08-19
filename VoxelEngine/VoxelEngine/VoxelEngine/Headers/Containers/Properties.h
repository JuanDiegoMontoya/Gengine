/*HEADER_GOES_HERE*/
#ifndef Properites_Guard
#define Properites_Guard

#define NAME std::string("name")
#define INSTANCE nullptr

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <glm/glm.hpp>

#include <boost/preprocessor/cat.hpp>
#include <imgui\imgui.h> //For GuiIfying the properties

enum struct ResourceType
{
  cMesh,
  cProgram,
  cUniform,
  cTexture,
  cFrameBuffer,

  cCount
};


typedef unsigned short ID;
/***********_PROPERTIES_***********/
//Property will eventually fire an event that contains address in memory, and what the value will be changed to.
//The catcher will store the bytes of what it was after, and the bytes of what it was before, and the address 
//for undo/redo functionallity. 

//a name and offset in bytes are also provided for modifying existing objects with
//a given property name. 

//unique ID's are used for reverse lookup of names. this can be used for debug printing name when modifying data

/***********_PROPERTY_CONTAINERS_***********/
//Nothing is needed to make property containers work with state changes. 

//When modifying existing objects, you'll provide the name of the property container. this will iterate over all 
//contained properties with something to the extent of for(auto i : container) manipulate(i);
//or just give an index for a specific value

//In the case of a container of properties, each child property will not have a unique ID. instead, the container will. 
//Each child is assigned the parent containers ID at creation. containers are restricted to arrays for simplicity

/***********_PROPERTIES_IDS***********/
//Property IDs are used by the factory to grab all properites/property containers of entities, 
//and know their name, and how to modify them (location offset and type). Looking up is also possible if you're given a property
//by use of it's ID.
//name, unique variable ID, offset, type ID
typedef std::tuple<std::string, ID, size_t, size_t> PropertyID;

#define PROPERTY(type, name, default_value) static const ID name ##_id = __COUNTER__; type name = AssignedProperty(default_value, name ##_id)

#define PROPERTY_ID(type, name) PropertyID( \
#name , \
name .id = name ##_id, \
reinterpret_cast<char*>(& name ) - reinterpret_cast<char*>(this), \
typeid( type ).hash_code())

#define PROPERTY_ARRAY(type, name, default_value) ID name ##_id = __COUNTER__; PropertyContainer< type > name

const ID unknown = 0xffff;

//generic properties trade off exposed functionallity for simple property functionallity. 
//Basically they just cut down server implimentation time at the cost of client implimentation time.
template<class T>
class Generic : public T
{
public:
  Generic() { }
  Generic(const T val) : T(val) { } 
};

template<ResourceType type>
class Resource
{
public:
  ID value = 0;
  Resource() { }
  Resource(const ID val) : value(val) { }
  operator ID() const { return value; }
};

template<class T, size_t N>
class PropertyArray : public std::array<T, N>
{
public:
  ID id = unknown;
};

template <class T, class U, size_t Count>
constexpr PropertyArray<T, Count> property_array_initialized(const U value, ID id_)
{
  PropertyArray<T, Count> result;
  result.fill(value);
  for (auto i : result) i.id = id_;
  return result;
}

template<class T>
class Property {
public:
  T value;
  ID id = unknown;
  typedef T value_type;
  Property() :value() {}
  Property(T v) :value(v) {}
  operator T() const { return value; }

  //modifiers
  Property& operator=(T v) { value = v; return *this; }
  Property& operator+=(T v) { value += v; return *this; }
  Property& operator-=(T v) { value -= v; return *this; }
  Property& operator*=(T v) { value *= value; return *this; }
  Property& operator/=(T v) { value /= value; return *this; }
  Property& operator%=(T v) { value %= value; return *this; }
  Property& operator++() { ++value; return *this; }
  Property& operator--() { --value; return *this; }
  Property operator++(int) { return Property(value++); }
  Property operator--(int) { return Property(value--); }
  Property& operator&=(T v) { value &= v; return *this; }
  Property& operator|=(T v) { value |= v; return *this; }
  Property& operator^=(T v) { value ^= v; return *this; }
  Property& operator<<=(T v) { value <<= v; return *this; }
  Property& operator>>=(T v) { value >>= v; return *this; }

  //accessors
  Property operator+() const { return Property(+value); }
  Property operator-() const { return Property(-value); }
  Property operator!() const { return Property(!value); }
  Property operator~() const { return Property(~value); }

  //friends
  friend Property operator+(Property iw, Property v) { return iw += v; }
  friend Property operator+(Property iw, T v) { return iw += v; }
  friend Property operator+(T v, Property iw) { return Property(v) += iw; }
  friend Property operator-(Property iw, Property v) { return iw -= v; }
  friend Property operator-(Property iw, T v) { return iw -= v; }
  friend Property operator-(T v, Property iw) { return Property(v) -= iw; }
  friend Property operator*(Property iw, Property v) { return iw *= v; }
  friend Property operator*(Property iw, T v) { return iw *= v; }
  friend Property operator*(T v, Property iw) { return Property(v) *= iw; }
  friend Property operator/(Property iw, Property v) { return iw /= v; }
  friend Property operator/(Property iw, T v) { return iw /= v; }
  friend Property operator/(T v, Property iw) { return Property(v) /= iw; }
  friend Property operator%(Property iw, Property v) { return iw %= v; }
  friend Property operator%(Property iw, T v) { return iw %= v; }
  friend Property operator%(T v, Property iw) { return Property(v) %= iw; }
  friend Property operator&(Property iw, Property v) { return iw &= v; }
  friend Property operator&(Property iw, T v) { return iw &= v; }
  friend Property operator&(T v, Property iw) { return Property(v) &= iw; }
  friend Property operator|(Property iw, Property v) { return iw |= v; }
  friend Property operator|(Property iw, T v) { return iw |= v; }
  friend Property operator|(T v, Property iw) { return Property(v) |= iw; }
  friend Property operator^(Property iw, Property v) { return iw ^= v; }
  friend Property operator^(Property iw, T v) { return iw ^= v; }
  friend Property operator^(T v, Property iw) { return Property(v) ^= iw; }
  friend Property operator<<(Property iw, Property v) { return iw <<= v; }
  friend Property operator<<(Property iw, T v) { return iw <<= v; }
  friend Property operator<<(T v, Property iw) { return Property(v) <<= iw; }
  friend Property operator>>(Property iw, Property v) { return iw >>= v; }
  friend Property operator>>(Property iw, T v) { return iw >>= v; }
  friend Property operator>>(T v, Property iw) { return Property(v) >>= iw; }
};

template<class U>
class Property<Generic<U>>
{
public:
  U value;
  ID id = unknown;
  Property() : value() {}
  Property(U v) :value(v) {}
  U get() const { return value; }
  void set(const U& v) { value = v; }
};

template<ResourceType type>
class Property<Resource<type>>
{
public:
  ID value;
  ID id = unknown;
  Property() : value() {}
  Property(ID v) :value(v) {}
  Property(Resource<type> v) : value(v.operator ID()) {}
  ID get() const { return value; }
  void set(const ID& v) { value = v; }
};

template<>
class Property<std::string>{
public:
  std::string value;
  ID id = unknown;
  typedef std::string value_type;
  Property() :value() {}
  Property(std::string v) :value(v) {}
  Property(const char* v) : value(v) {};
  Property(char* v) : value(v) {};
  operator std::string() const { return value; }
  //modifiers
  Property& operator=(std::string v) { value = v; return *this; }
  Property& operator=(const char* v) { value = v; return *this; }
  Property& operator+=(std::string v) { value += v; return *this; }
  Property& operator+=(const char* v) { value += v; return *this; }
  //friends
  friend Property operator+(Property iw, Property v) { return iw += v; }
  friend Property operator+(Property iw, std::string v) { return iw += v; }
  friend Property operator+(std::string v, Property iw) { return Property(v) += iw; }
};

class enoom
{
public:
  int value;
  enoom() : value() {}
  enoom(int v, std::vector<std::string>* names_) :value(v), names(names_) {}
  enoom(const enoom& v) : value(v.value), names(v.names) {}
  operator int() const { return value; }

  std::vector<std::string>* names = nullptr;

  enoom& operator=(int v) { value = v; return *this; }
  enoom& operator=(const enoom& v) { value = v.value; return *this; }

  bool operator!() const { return !value; }

};

template <>
class Property<enoom>
{
public:
  enoom value;
  ID id = unknown;
  typedef enoom value_type;
  Property() :value() {}
  Property(enoom v) :value(v) {}
  Property(const Property& v) : value(v.value) {}
  operator enoom() const { return value; }
  operator int() const { return value.value; }

  std::vector<std::string>* values = nullptr;

  Property& operator=(enoom v) { value = v; return *this; }
  Property& operator=(const Property& v) { value = v.value; return *this; }

  bool operator!() const { return !value; }
};

template <class T>
constexpr Property<T> AssignedProperty(T value, ID id_)
{
  Property<T> result = value;
  result.id = id_;
  return result;
}

template<class T>
constexpr Property<Generic<T>>  AssignedProperty(Generic<T> value, ID id_)
{
  Property<Generic<T>> result = value;
  result.id = id_;
  return result;
}

template<ResourceType type>
constexpr Property<Resource<type>>  AssignedProperty(Resource<type> value, ID id_)
{
  Property<Resource<type>> result = value;
  result.id = id_;
  return result;
}

typedef Property<int> Int;
//typedef Property<unsigned> Unsigned;
//typedef Property<short> Short;
//typedef Property<unsigned short> Ushort;
//typedef Property<char> Char;
//typedef Property<long long> Long;
//typedef Property<unsigned long long> Ulong;
typedef Property<float> Float;
//typedef Property<double> Double;
//typedef Property<long double> LongDouble;
typedef Property<bool> Bool;
typedef Property<uint32_t> Uint32_t;
typedef Property<std::string> String;
typedef Property<Generic<glm::vec2>> Vec2;
typedef Property<Generic<glm::vec3>> Vec3;
typedef Property<Generic<glm::vec4>> Vec4;
typedef Property<enoom> Enoom;

typedef Int Saveable_ID;

////GuiIfy shit
//void GuiIfy(String* instance, std::string name)
//{
//  constexpr int bufferSize = 128;
//  static char buffer[bufferSize] = { 0 };
//  std::string& value = (*(String*)(instance)).value;
//  strcpy_s(buffer, bufferSize, value.c_str());
//  ImGui::InputText((std::stringstream() << name << "##" << instance).str().c_str(), buffer, bufferSize);
//  value = buffer;
//
//}
//
//void GuiIfy(Int* instance, std::string name)
//{
//  ImGui::InputInt((std::stringstream() << name + "##" << instance).str().c_str(), &(*(Int*)(instance)).value);
//}
//
//void GuiIfy(Float* instance, std::string name)
//{
//  ImGui::InputFloat((std::stringstream() << name + "##" << instance).str().c_str(), &(*(Float*)(instance)).value);
//}
//
//void GuiIfy(Bool* instance, std::string name)
//{
//  ImGui::Checkbox((std::stringstream() << name + "##" << instance).str().c_str(), &(*(Bool*)(instance)).value);
//}
//
//void GuiIfy(Vec2* instance, std::string name)
//{
//  ImGui::InputFloat2((std::stringstream() << name + "##" << instance).str().c_str(), &((*(Vec2*)(instance)).value[0]));
//}
//
//void GuiIfy(Vec3* instance, std::string name)
//{
//  ImGui::InputFloat3((std::stringstream() << name + "##" << instance).str().c_str(), &((*(Vec3*)(instance)).value[0]));
//}
//
//void GuiIfy(Vec4* instance, std::string name)
//{
//  ImGui::InputFloat4((std::stringstream() << name + "##" << instance).str().c_str(), &((*(Vec4*)(instance)).value[0]));
//}
//
//void GuiIfy(Enoom* instance, std::string name)
//{
//  Enoom& enoom = (*(Enoom*)(instance));
//  std::string enoomName = (*(enoom.value.names))[enoom.value.value];
//  const char* selectableName = enoomName.c_str();
//  if (ImGui::BeginCombo((std::stringstream() << name << "##" << instance).str().c_str(), selectableName))
//  {
//    int index = 0;
//    for (auto& i : *enoom.value.names)
//    {
//      if (ImGui::Selectable(i.c_str())) { enoom.value.value = index; }
//      ++index;
//    }
//    ImGui::EndCombo();
//  }
//}


#endif // !Properites_Guard
