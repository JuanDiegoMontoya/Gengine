#include "stdafx.h"
#include "pipeline.h"
#include "block.h"

const std::vector<BlockProperties> Block::PropertiesTable =
{
	{BlockProperties("air",        {0, 0, 0, 0}, Visibility::Invisible)},
	{BlockProperties("stone",      {0, 0, 0, 0})},
	{BlockProperties("dirt",       {0, 0, 0, 0}, Visibility::Opaque)},
	{BlockProperties("metal",      {0, 0, 0, 0})},
	{BlockProperties("grass",      {0, 0, 0, 0})},
	{BlockProperties("sand",       {0, 0, 0, 0})},
	{BlockProperties("snow",       {0, 0, 0, 0})},
	{BlockProperties("water",      {0, 0, 0, 0})},
	{BlockProperties("oak wood",   {0, 0, 0, 0})},
	{BlockProperties("oak leaves", {0, 0, 0, 0}, Visibility::Partial)},
	{BlockProperties("error",      {0, 0, 0, 0})},
	{BlockProperties("dry grass",  {0, 0, 0, 0})},
	{BlockProperties("Olight",     {15, 8, 0, 0})},
	{BlockProperties("Rlight",     {15, 0, 0, 0})},
	{BlockProperties("Glight",     {0, 15, 0, 0})},
	{BlockProperties("Blight",     {0, 0, 15, 0})},
	{BlockProperties("Smlight",    {15, 15, 15, 0})},
	{BlockProperties("Ylight",     {15, 15, 0, 0})},
	{BlockProperties("RGlass",     {0, 0, 0, 0}, Visibility::Partial)},
	{BlockProperties("GGlass",     {0, 0, 0, 0}, Visibility::Partial)},
	{BlockProperties("BGlass",     {0, 0, 0, 0}, Visibility::Partial)},
};
