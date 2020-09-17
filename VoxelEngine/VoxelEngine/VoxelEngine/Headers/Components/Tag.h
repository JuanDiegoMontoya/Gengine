#ifndef Tag_Guard
#define Tag_Guard

#include <iostream>
#include "../Containers/Properties.h"
#include "Component.h"

//typedef class UpdateEvent UpdateEvent;
//typedef class DrawEvent DrawEvent;


class Tag : public Component
{
public:
  static const ID componentType = cTag;

  //PROPERTY(Int, componentData, 5999);

  Tag() : Component(cTag) {}
  Tag(std::string tag_) : Component(cTag), tag(tag_) {}
  ~Tag() {}
  void Init() {};
  void End() {};

  std::unique_ptr<Component> Clone() const
  {
    assert(false); // not implemented
    return nullptr;
  }

  std::string GetName() { return "Tag"; }

  static std::unique_ptr<Tag> RegisterTag()
  {
    return std::make_unique<Tag>();
  }

  std::string tag;

private:

  friend void RegisterComponents();
};

#endif // !Tag_Guard