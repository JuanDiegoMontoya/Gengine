/*HEADER_GOES_HERE*/
#ifndef Component_DEF
#define Component_DEF
#include "AllComponentHeaders.h"
#include "../FactoryID.h"
#include <string>
#include <memory>
#include <engine_assert.h>
#include <Containers/Entity.h>

class GameEngine;
class Space;
class Graphics;
//class Entity;
using ID = unsigned short;

class Component
{
public:
  static const FactoryID factoryID = FactoryID::cComponent;
  const ID type;

  Component(ID type_, Entity parent_ = {}) : type(type_), parent(parent_) { }
  Component(Component&& other) noexcept : type(other.type), parent(std::move(other.parent)) { }
  Component& operator=(Component&& other) noexcept
  {
    ASSERT(type == other.type);
    parent = std::move(other.parent);
    return *this;
  }
  Component& operator=(const Component& other)
  {
    ASSERT(type == other.type);
    parent = other.parent;
    return *this;
  }

  virtual ~Component() { };
  virtual void Init() { };
  virtual void End() { };

  virtual void EditorVerify() { };

  virtual std::unique_ptr<Component> Clone() const = 0;
  virtual std::string GetName() = 0;

  Entity GetParent() const { return parent; }
  Space* GetSpace() const;

  Entity parent;

private:
};


#endif // !Component_DEF