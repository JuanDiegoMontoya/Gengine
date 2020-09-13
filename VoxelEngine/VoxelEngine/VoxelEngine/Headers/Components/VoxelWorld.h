#ifndef VoxelWorld_Guard
#define VoxelWorld_Guard

#include <iostream>
#include "../Containers/Properties.h"
#include "Component.h"

//typedef class UpdateEvent UpdateEvent;
//typedef class DrawEvent DrawEvent;


class VoxelWorld : public Component
{
public:
  static const ID componentType = cVoxelWorld;

  //PROPERTY(Int, componentData, 5999);

  VoxelWorld(
    //Int componentData_ = 5999                                             /*** "= 5999" Only needed if you want a default constructor***/
  );
  ~VoxelWorld();
  void Init();
  void End();

  std::unique_ptr<Component> Clone() const;
  std::string GetName();

  //void UpdateEventsListen(UpdateEvent* updateEvent);
  //void DrawEventsListen(DrawEvent* drawEvent);

  static std::unique_ptr<VoxelWorld> RegisterVoxelWorld();

private:

  friend void RegisterComponents();
};

#endif // !VoxelWorld_Guard