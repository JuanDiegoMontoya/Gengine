#include "stdafx.h"
#include "input.h"

#include <filesystem>
namespace fs = std::filesystem;

using namespace ImGui;
//extern GLFWwindow* window;

// currently active options menu
enum OptionsTab : int
{
	kControls,
	kGraphics,
	kSound,
	//kEditor - omitted, will be modified in the editor itself
	kCount
};

static int activeMenu = kGraphics;


// re-initializes the openGL context with (probably) new parameters
// this function be hard to make
//void reinitializeDatMfGlfwContextBruh()
//{
//	//FT_Done_Face(Text::face);
//	//FT_Done_FreeType(Text::library);
//	Settings::Get().Graphics.reinit = true;
//	glfwDestroyWindow(Game_::game->Window());
//	glfwTerminate();
//	//Render::Terminate();
//	//ImGui::End();
//	ImGui::EndFrame();
//	Game_::game->Window() = InitContext();
//
//	int width = Settings::Get().Graphics.tx;
//	int height = Settings::Get().Graphics.ty;
//
//	Render::Init();
//	EditorUIShutdown();
//	Editor::Get().UIStart(Game_::game->Window());
//	Settings::Get().Graphics.drawFrame = false;
//
//	// re-make all textures
//	Render::BoolForDebuggingPurposes = true;
//	textures.clear();
//	string n = "./resources/images/";
//	std::experimental::filesystem::v1::directory_entry ss;
//	for (auto& p : fs::directory_iterator(n))
//	{
//		string name1 = p.path().filename().string();
//		if (name1.find(".png") != string::npos || name1.find(".jpg") != string::npos)
//			Sprite::makeTexture(name1);
//	}
//
//	Game_::game->selectLevel(Game_::game->currLevel->Name()); // reload current level
//}


//bool DrawOptions()
//{
//	ImGui::SetNextWindowBgAlpha(0);
//	ImGui::SetNextWindowPos(Utilities::getResponsiveXY(ImVec2(500, 500)));
//	ImGui::SetNextWindowSize(Utilities::getResponsiveXY(ImVec2(200, 200)));
//
//	ImGui::Begin("Options");
//
//	if (Selectable("Controls", activeMenu == kControls, 0, ImVec2(60, 15)))
//		activeMenu = kControls;
//	SameLine();
//	if (Selectable("Graphics", activeMenu == kGraphics, 0, ImVec2(60, 15)))
//		activeMenu = kGraphics;
//	SameLine();
//	if (Selectable("Sound", activeMenu == kSound, 0, ImVec2(60, 15)))
//		activeMenu = kSound;
//
//	if (activeMenu == kControls)
//	{
//		/*
//			Explanation of this disgusting-looking code:
//			by casting the int's address to a char*, we can insert it into ImGui text field
//			to be modified. The buffer size is 2 to account for the char + the nul character.
//		*/
//		ImGui::PushItemWidth(15);
//		ImGui::InputText("Jump", (char*)&Settings::Get().Controls.keyUp, 2);
//		SameLine();
//		PushItemWidth(15);
//		InputText("Alt Jump", (char*)&Settings::Get().Controls.keyUpAlt, 2);
//
//		ImGui::PushItemWidth(15);
//		ImGui::InputText("Move Left", (char*)&Settings::Get().Controls.keyLeft, 2);
//		ImGui::PushItemWidth(15);
//		ImGui::InputText("Move Right", (char*)&Settings::Get().Controls.keyRight, 2);
//		ImGui::PushItemWidth(15);
//		ImGui::InputText("Down/Dive", (char*)&Settings::Get().Controls.keyDown, 2);
//	}
//	else if (activeMenu == kGraphics)
//	{
//		int res[2];
//		res[0] = Settings::Get().Graphics.ScreenX();
//		res[1] = Settings::Get().Graphics.ScreenY();
//		if (InputInt2("Resolution (restart to take effect)", res))
//			//if (InputInt2("Resolution (restart to take effect)", res, ImGuiInputTextFlags_EnterReturnsTrue))
//		{
//			Settings::Get().Graphics.setWidth((unsigned)res[0]);
//			Settings::Get().Graphics.setHeight((unsigned)res[1]);
//			//reinitializeDatMfGlfwContextBruh();
//		}
//
//		if (SliderInt("MSAA Amount", (int*)&Settings::Get().Graphics.multisamples, 0, 8))
//		{
//			glfwWindowHint(GL_MULTISAMPLE, Settings::Get().Graphics.multisamples);
//		}
//
//		if (Button("Apply"))
//		{
//			reinitializeDatMfGlfwContextBruh();
//			return true;
//		}
//	}
//	else if (activeMenu == kSound)
//	{
//
//	}
//
//	ImGui::End();
//	return false;
//}