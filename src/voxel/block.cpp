#include "vPCH.h"
#include <voxel/block.h>

namespace Voxels
{
  const std::vector<BlockProperties> Block::PropertiesTable =
  {
    {BlockProperties("air",        {0, 0, 0, 0}, 0, 0.0f, false, Visibility::Invisible)},
    {BlockProperties("stone",      {0, 0, 0, 0}, 1, 2.0f, false)},
    {BlockProperties("dirt",       {0, 0, 0, 0}, 1, 0.5f, true, Visibility::Opaque)},
    {BlockProperties("metal",      {0, 0, 0, 0}, 1)},
    {BlockProperties("grass",      {0, 0, 0, 0}, 1, 0.25f)},
    {BlockProperties("sand",       {0, 0, 0, 0}, 1)},
    {BlockProperties("snow",       {0, 0, 0, 0}, 1)},
    {BlockProperties("water",      {0, 0, 0, 0}, 1)},
    {BlockProperties("oak wood",   {0, 0, 0, 0}, 1)},
    {BlockProperties("oak leaves", {0, 0, 0, 0}, 0, 0.1f, true, Visibility::Partial)},
    {BlockProperties("error",      {0, 0, 0, 0}, 1)},
    {BlockProperties("dry grass",  {0, 0, 0, 0}, 1)},
    {BlockProperties("Olight",     {15, 8, 0, 0}, 1, 0.1f)},
    {BlockProperties("Rlight",     {15, 0, 0, 0}, 1, 0.1f)},
    {BlockProperties("Glight",     {0, 15, 0, 0}, 1, 0.1f)},
    {BlockProperties("Blight",     {0, 0, 15, 0}, 1, 0.1f)},
    {BlockProperties("Smlight",    {15, 15, 15, 0}, 1, 0.1f)},
    {BlockProperties("Ylight",     {15, 15, 0, 0}, 1, 0.1f)},
    {BlockProperties("RGlass",     {0, 0, 0, 0}, 1, 0.1f, true, Visibility::Partial)},
    {BlockProperties("GGlass",     {0, 0, 0, 0}, 1, 0.1f, true, Visibility::Partial)},
    {BlockProperties("BGlass",     {0, 0, 0, 0}, 1, 0.1f, true, Visibility::Partial)},
    {BlockProperties("BGlass",     {0, 0, 0, 0}, 1, 0.1f, true, Visibility::Partial)},
    {BlockProperties("DevValue100",{0, 0, 0, 0}, 1, 0.1f) },
    {BlockProperties("DevValue90", {0, 0, 0, 0}, 1, 0.1f) },
    {BlockProperties("DevValue80", {0, 0, 0, 0}, 1, 0.1f) },
    {BlockProperties("DevValue70", {0, 0, 0, 0}, 1, 0.1f) },
    {BlockProperties("DevValue60", {0, 0, 0, 0}, 1, 0.1f) },
    {BlockProperties("DevValue50", {0, 0, 0, 0}, 1, 0.1f) },
    {BlockProperties("DevValue40", {0, 0, 0, 0}, 1, 0.1f) },
    {BlockProperties("DevValue30", {0, 0, 0, 0}, 1, 0.1f) },
    {BlockProperties("DevValue20", {0, 0, 0, 0}, 1, 0.1f) },
    {BlockProperties("DevValue10", {0, 0, 0, 0}, 1, 0.1f) },
    {BlockProperties("DevValue00", {0, 0, 0, 0}, 1, 0.1f) },
  };
}