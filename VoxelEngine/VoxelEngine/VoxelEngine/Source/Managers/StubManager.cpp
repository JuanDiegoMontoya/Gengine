/*HEADER_GOES_HERE*/
#include "../../Headers/Managers/StubManager.h"
#include "../../Headers/Factory.h"

StubManager* StubManager::pStubManager = nullptr;

StubManager::StubManager()
{
}

StubManager::~StubManager()
{
  pStubManager = nullptr;
}

std::string StubManager::GetName()
{
  return "StubManager";
}

void StubManager::Init()
{
}

void StubManager::End()
{
}