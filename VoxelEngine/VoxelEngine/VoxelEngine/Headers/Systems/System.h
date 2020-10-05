/*HEADER_GOES_HERE*/
#ifndef System_DEF
#define System_DEF
#include "AllSystemHeaders.h"
#include "../FactoryID.h"
#include <string>
#include <memory>

typedef class Object Object;
typedef class Space Space;
typedef unsigned short ID;

class System
{
public:
  static const FactoryID factoryID = FactoryID::cSystem;
  const ID type;

  System(ID type_, Object* parent_ = nullptr) : type(type_), parent(parent_) { };

  virtual ~System() { };
  virtual void Init() { };
  virtual void End() { };

  virtual void EditorVerify() { };

  virtual std::unique_ptr<System> Clone() const = 0;
  virtual std::string GetName() = 0;

  Object* GetParent() const { return parent; }
  Space* GetSpace() const;

  Object* parent;

private:
};


#endif // !System_DEF