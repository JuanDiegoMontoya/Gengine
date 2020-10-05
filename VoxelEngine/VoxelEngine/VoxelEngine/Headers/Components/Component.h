/*HEADER_GOES_HERE*/
#ifndef Component_DEF
#define Component_DEF
#include "AllComponentHeaders.h"
#include "../FactoryID.h"
#include <string>
#include <memory>

typedef class Object Object;
typedef class GameEngine GameEngine;
typedef class Space Space;
typedef class Graphics Graphics;
typedef unsigned short ID;

class Component
{
public:
  static const FactoryID factoryID = FactoryID::cComponent;
  const ID type;

  Component(ID type_, Object* parent_ = nullptr) : type(type_), parent(parent_) { };

  virtual ~Component() { };
  virtual void Init() { };
  virtual void End() { };

  virtual void EditorVerify() { };

  virtual std::unique_ptr<Component> Clone() const = 0;
  virtual std::string GetName() = 0;

  Object* GetParent() const { return parent; }
  Space* GetSpace() const;

  Object* parent;

private:
};


#endif // !Component_DEF