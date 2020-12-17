#include <Voxels/block.h>

const std::vector<BlockProperties> Block::PropertiesTable =
{
  {BlockProperties("air",        {0, 0, 0, 0}, 0.0f, false, Visibility::Invisible)},
  {BlockProperties("stone",      {0, 0, 0, 0}, 2.0f, false)},
  {BlockProperties("dirt",       {0, 0, 0, 0}, 0.5f, true, Visibility::Opaque)},
  {BlockProperties("metal",      {0, 0, 0, 0})},
  {BlockProperties("grass",      {0, 0, 0, 0}, 0.25f)},
  {BlockProperties("sand",       {0, 0, 0, 0})},
  {BlockProperties("snow",       {0, 0, 0, 0})},
  {BlockProperties("water",      {0, 0, 0, 0})},
  {BlockProperties("oak wood",   {0, 0, 0, 0})},
  {BlockProperties("oak leaves", {0, 0, 0, 0}, 0.2f, true, Visibility::Partial)},
  {BlockProperties("error",      {0, 0, 0, 0})},
  {BlockProperties("dry grass",  {0, 0, 0, 0})},
  {BlockProperties("Olight",     {15, 8, 0, 0}, 0.1f)},
  {BlockProperties("Rlight",     {15, 0, 0, 0}, 0.1f)},
  {BlockProperties("Glight",     {0, 15, 0, 0}, 0.1f)},
  {BlockProperties("Blight",     {0, 0, 15, 0}, 0.1f)},
  {BlockProperties("Smlight",    {15, 15, 15, 0}, 0.1f)},
  {BlockProperties("Ylight",     {15, 15, 0, 0}, 0.1f)},
  {BlockProperties("RGlass",     {0, 0, 0, 0}, 0.1f, true, Visibility::Partial)},
  {BlockProperties("GGlass",     {0, 0, 0, 0}, 0.1f, true, Visibility::Partial)},
  {BlockProperties("BGlass",     {0, 0, 0, 0}, 0.1f, true, Visibility::Partial)},
};
