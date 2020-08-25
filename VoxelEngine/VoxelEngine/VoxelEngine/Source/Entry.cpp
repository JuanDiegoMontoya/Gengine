//#include <iostream>
//#include <vector>
//#include <functional>
//
//#include "../Headers/Systems/System.h"
//#include "../Headers/Systems/System.h"
//#include "../Headers/Engine.h"
//#include "../Headers/Factory.h"

//#include "common.h"
//#include "bgfx_utils.h"
//
//entry::AppI* application = nullptr;

//namespace
//{
//
//class Application : public entry::AppI
//{
//public:
//	Application(const char* _name, const char* _description, const char* _url)
//		: entry::AppI(_name, _description, _url)
//	{
//		application = this;
//		entry::setWindowTitle(DEFAULT_WINDOW_HANDLE, _name);
//	}
//
//	void init(int32_t _argc, const char* const* _argv, uint32_t _width, uint32_t _height) override
//	{
//		Factory::Register();
//		Graphics::GetGraphics()->args = Args(_argc, _argv);
//		Graphics::GetGraphics()->width = _width;
//		Graphics::GetGraphics()->height = _height;
//		Engine::GetEngine()->Init();
//	}
//
//	virtual int shutdown() override
//	{
//		Engine::GetEngine()->End();
//
//		return 0;
//	}
//
//	bool update() override
//	{
//		Engine::GetEngine()->Update();
//
//		return !Engine::GetEngine()->quitEngine;
//	}
//};
//
//} // namespace
//
//ENTRY_IMPLEMENT_MAIN(
//	Application
//	, "Custom Engine"
//	, ""
//	, ""
//	);

#include "../Headers/Engine.h"
#include "../Headers/Factory.h"
#include "../Headers/Sandbox.h"
int main()
{
#ifndef SANDBOX
  Factory::Register();
  Engine::GetEngine()->Init();
	while (!Engine::GetEngine()->quitEngine)
	{
		Engine::GetEngine()->Update();
	}
  Engine::GetEngine()->End();

#else
  Sandbox();
#endif

}
