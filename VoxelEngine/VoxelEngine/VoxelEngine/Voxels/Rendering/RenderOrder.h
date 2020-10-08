#pragma once

// helps determines the order in which things are updated
namespace RenderOrder
{
  enum Render
  {
    RenderClear,
    RenderSky,
    RenderClientPlayers,
    RenderDrawAll,
    RenderInterfaceImGui,
    RenderInterfaceUpdate,
    RenderChunkRenderUpdate,
    RenderHUD,
  };

  enum Update
  {
    WorldUpdate,
  };
}