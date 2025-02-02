#include "Phobos.UI.h"

#ifdef EXPERIMENTAL_IMGUI

/*
	Based from : https://github.com/CCHyper/Vinifera/commits/dev/imgui-dev/
	Credits : CCHyper
*/

#include "Phobos.h"

#include <Utilities/Debug.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_impl_dx11.h>
#include <d3d11.h>

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <TriggerTypeClass.h>
#include <TriggerClass.h>
#include <TagTypeClass.h>

HWND ImGuiMainWindow = nullptr;
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

void ImGui_CreateRenderTarget()
{
	if (!g_pd3dDevice)
	{
		return;
	}

	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

void ImGui_New_Frame()
{
	if (!g_pd3dDeviceContext || !g_pSwapChain)
	{
		return;
	}

	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGui_Render_Frame()
{
	if (!g_pd3dDeviceContext || !g_pSwapChain)
	{
		return;
	}

	// Rendering
	ImGui::Render();

	static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
	g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
	g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	g_pSwapChain->Present(1, 0); // Present with vsync
	//g_pSwapChain->Present(0, 0); // Present without vsync
}

void ImGui_End_Frame()
{
	ImGui::EndFrame();
}

void ImGui_CleanupRenderTarget()
{
	if (g_mainRenderTargetView)
	{
		g_mainRenderTargetView->Release();
		g_mainRenderTargetView = nullptr;
	}
}

bool ImGui_CreateDeviceD3D(HWND hWnd)
{
	if (g_pd3dDevice)
	{
		return false;
	}

	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
	{
		return false;
	}

	ImGui_CreateRenderTarget();

	return true;
}

void ImGui_CleanupDeviceD3D()
{
	ImGui_CleanupRenderTarget();

	if (g_pSwapChain)
	{
		g_pSwapChain->Release();
		g_pSwapChain = nullptr;
	}
	if (g_pd3dDeviceContext)
	{
		g_pd3dDeviceContext->Release();
		g_pd3dDeviceContext = nullptr;
	}
	if (g_pd3dDevice)
	{
		g_pd3dDevice->Release();
		g_pd3dDevice = nullptr;
	}
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI Audio_Debug_Main_Window_Procedure(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, Message, wParam, lParam))
	{
		return true;
	}

	switch (Message)
	{
	case WM_SIZE:
		if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
		{
			ImGui_CleanupRenderTarget();
			g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
			ImGui_CreateRenderTarget();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, Message, wParam, lParam);
}

bool PhobosWindowClass::Create()
{
	SetLastError(0);

	Debug::Log("Developer - Creating window.\n");

	ImGui_ImplWin32_EnableDpiAwareness();

	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_CLASSDC;
	wc.lpfnWndProc = Audio_Debug_Main_Window_Procedure;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = (HINSTANCE)Phobos::hInstance;
	wc.hIcon = nullptr;
	wc.hCursor = nullptr;
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = "Developer Mode Window";
	wc.hIconSm = nullptr;

	BOOL rc = RegisterClassEx(&wc);
	if (!rc)
	{
		Debug::Log("Developer - Failed to register window class!\n");
		return false;
	}

	HWND hwnd = CreateWindowEx(
		0,
		wc.lpszClassName,
		"Developer Mode Window",
		WS_OVERLAPPEDWINDOW,
		0,
		0,
		GameOptionsClass::Instance->ScreenWidth,
		GameOptionsClass::Instance->ScreenHeight,
		nullptr,
		nullptr,
		wc.hInstance,
		nullptr);

	if (!hwnd)
	{
		Debug::Log("Developer - Failed to create window!\n");
		return false;
	}

	Debug::Log("Developer - Setting window size.\n");

	// Resposition and resize the window based on the monitor scale.
	float scale = 100.0f;

	SetWindowPos(hwnd,
		nullptr,
		GetSystemMetrics(SM_CXSCREEN) - GameOptionsClass::Instance->ScreenWidth,
		GetSystemMetrics(SM_CYSCREEN) - GameOptionsClass::Instance->ScreenHeight,
		int(GameOptionsClass::Instance->ScreenWidth * scale),
		int(GameOptionsClass::Instance->ScreenHeight * scale),
		SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);

	Debug::Log("Developer - Creating Direct3D device.\n");

	// Initialize Direct3D
	if (!ImGui_CreateDeviceD3D(hwnd))
	{
		Debug::Log("Developer - Failed to create Direct3D device!\n");
		ImGui_CleanupDeviceD3D();
		UnregisterClass(wc.lpszClassName, wc.hInstance);
		return false;
	}

	// Store the window handle.
	ImGuiMainWindow = hwnd;

	// Show the window
	ShowWindow(hwnd, SW_SHOWDEFAULT);
	UpdateWindow(hwnd);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	Debug::Log("Developer - Setting up platform and renderer.\n");

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(ImGuiMainWindow);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

	Debug::Log("Developer: Window created.\n");

	return true;
}

bool PhobosWindowClass::Destroy()
{
	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();

	ImGui::DestroyContext();

	ImGui_CleanupDeviceD3D();

	DestroyWindow(ImGuiMainWindow);
	//UnregisterClass(wc.lpszClassName, wc.hInstance);

	return true;
}

void PhobosWindowClass::Loop()
{
	if (!ImGuiMainWindow)
	{
		return;
	}
	// Update and scale the UI.
	//{
	//    ImGuiStyle& style = ImGui::GetStyle();
	//    float scale = Get_Monitor_DPI_Scale(ImGuiMainWindow);
	//    style.ScaleAllSizes(scale);
	//}

	ImGui_New_Frame();

#if 0
	if (ImGui::BeginMainMenuBar())
	{

		/**
		 *  x
		 */
		if (ImGui::BeginMenu("Tools"))
		{

			//if (ImGui::MenuItem("Save", "Ctrl+S", false, MapClass::InMap())) {
			//    //menu_action = "FileSave";
			//}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Terrain"))
		{
			ImGui::EndMenu();
		}

		ImGui::Separator();

		if (ImGui::BeginMenu("Debug"))
		{
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

#endif

	TeamList();

#if 0


	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	static bool show_demo_window = true;
	//if (show_demo_window) {
	ImGui::ShowDemoWindow(&show_demo_window);
	//}

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		//ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		//ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		//ImGui::Checkbox("Another Window", &show_another_window);
		//
		//ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		//ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
		//
		//if (ImGui::Button("Button")) {                          // Buttons return true when clicked (most widgets return true when edited/activated)
		//    counter++;
		//}
		//
		//ImGui::SameLine();

#if 0
		//ImGui::Text("Tracker.Count = %d", AudioManagerClass::SoundTrackerArrayType::CollectionCount);
		ImGui::Text("Music.Count = %d", AudioManager.SoundTracker.Raw(AUDIO_GROUP_MUSIC).Count());
		ImGui::Text("MusicAmbient.Count = %d", AudioManager.SoundTracker.Raw(AUDIO_GROUP_MUSIC_AMBIENT).Count());
		ImGui::Text("Speech.Count = %d", AudioManager.SoundTracker.Raw(AUDIO_GROUP_SPEECH).Count());
		ImGui::Text("SoundEffect.Count = %d", AudioManager.SoundTracker.Raw(AUDIO_GROUP_SOUND_EFFECT).Count());
		ImGui::Text("Event.Count = %d", AudioManager.SoundTracker.Raw(AUDIO_GROUP_EVENT).Count());
#endif

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	{
		ImGui::Begin("Hello, world! 2");
		ImGui::Text("UnitTypes.Count = %d", UnitTypes.Count());
		ImGui::Text("Units.Count = %d", Units.Count());
		ImGui::End();
	}

	// Scenario
	{
		if (Scen)
		{
			ImGui::Begin("Scenario");

			bool IsShadowGrow = Scen->SpecialFlags.IsShadowGrow;
			bool IsSpeedBuild = Scen->SpecialFlags.IsSpeedBuild;
			bool IsFromInstall = Scen->SpecialFlags.IsFromInstall;
			bool IsCaptureTheFlag = Scen->SpecialFlags.IsCaptureTheFlag;
			bool IsInert = Scen->SpecialFlags.IsInert;
			bool IsTGrowth = Scen->SpecialFlags.IsTGrowth;
			bool IsTSpread = Scen->SpecialFlags.IsTSpread;
			bool IsMCVDeploy = Scen->SpecialFlags.IsMCVDeploy;
			bool InitialVeteran = Scen->SpecialFlags.InitialVeteran;
			bool FixedAlliance = Scen->SpecialFlags.FixedAlliance;
			bool HarvesterImmune = Scen->SpecialFlags.HarvesterImmune;
			bool FogOfWar = Scen->SpecialFlags.FogOfWar;
			bool Bit2_16 = Scen->SpecialFlags.Bit2_16;
			bool TiberiumExplosive = Scen->SpecialFlags.TiberiumExplosive;
			bool DestroyableBridges = Scen->SpecialFlags.DestroyableBridges;
			bool Meteorites = Scen->SpecialFlags.Meteorites;
			bool IonStorms = Scen->SpecialFlags.IonStorms;
			bool IsVisceroids = Scen->SpecialFlags.IsVisceroids;

			ImGui::Checkbox("IsShadowGrow", &IsShadowGrow);
			ImGui::Checkbox("IsSpeedBuild", &IsSpeedBuild);
			ImGui::Checkbox("IsFromInstall", &IsFromInstall);
			ImGui::Checkbox("IsCaptureTheFlag", &IsCaptureTheFlag);
			ImGui::Checkbox("IsInert", &IsInert);
			ImGui::Checkbox("IsTGrowth", &IsTGrowth);
			ImGui::Checkbox("IsTSpread", &IsTSpread);
			ImGui::Checkbox("IsMCVDeploy", &IsMCVDeploy);
			ImGui::Checkbox("InitialVeteran", &InitialVeteran);
			ImGui::Checkbox("FixedAlliance", &FixedAlliance);
			ImGui::Checkbox("HarvesterImmune", &HarvesterImmune);
			ImGui::Checkbox("FogOfWar", &FogOfWar);
			ImGui::Checkbox("Bit2_16", &Bit2_16);
			ImGui::Checkbox("TiberiumExplosive", &TiberiumExplosive);
			ImGui::Checkbox("DestroyableBridges", &DestroyableBridges);
			ImGui::Checkbox("Meteorites", &Meteorites);
			ImGui::Checkbox("IonStorms", &IonStorms);
			ImGui::Checkbox("IsVisceroids", &IsVisceroids);

			Scen->SpecialFlags.IsShadowGrow = IsShadowGrow;
			Scen->SpecialFlags.IsSpeedBuild = IsSpeedBuild;
			Scen->SpecialFlags.IsFromInstall = IsFromInstall;
			Scen->SpecialFlags.IsCaptureTheFlag = IsCaptureTheFlag;
			Scen->SpecialFlags.IsInert = IsInert;
			Scen->SpecialFlags.IsTGrowth = IsTGrowth;
			Scen->SpecialFlags.IsTSpread = IsTSpread;
			Scen->SpecialFlags.IsMCVDeploy = IsMCVDeploy;
			Scen->SpecialFlags.InitialVeteran = InitialVeteran;
			Scen->SpecialFlags.FixedAlliance = FixedAlliance;
			Scen->SpecialFlags.HarvesterImmune = HarvesterImmune;
			Scen->SpecialFlags.FogOfWar = FogOfWar;
			Scen->SpecialFlags.Bit2_16 = Bit2_16;
			Scen->SpecialFlags.TiberiumExplosive = TiberiumExplosive;
			Scen->SpecialFlags.DestroyableBridges = DestroyableBridges;
			Scen->SpecialFlags.Meteorites = Meteorites;
			Scen->SpecialFlags.IonStorms = IonStorms;
			Scen->SpecialFlags.IsVisceroids = IsVisceroids;

			ImGui::SliderInt("AmbientOriginal", &Scen->AmbientOriginal, 0, 10000, "%d");
			ImGui::SliderInt("AmbientCurrent", &Scen->AmbientCurrent, 0, 10000, "%d");
			ImGui::SliderInt("AmbientTarget", &Scen->AmbientTarget, 0, 10000, "%d");

			ImGui::SliderInt("Red", &Scen->Red, 0, 10000, "%d");
			ImGui::SliderInt("Green", &Scen->Green, 0, 10000, "%d");
			ImGui::SliderInt("Blue", &Scen->Blue, 0, 10000, "%d");
			ImGui::SliderInt("Ground", &Scen->Ground, 0, 10000, "%d");
			ImGui::SliderInt("Level", &Scen->Level, 0, 10000, "%d");
			ImGui::Checkbox("IsFreeRadar", &Scen->IsFreeRadar);
			ImGui::Checkbox("IsTrainCrate", &Scen->IsTrainCrate);
			ImGui::Checkbox("IsTiberiumGrowth", &Scen->IsTiberiumGrowth);
			ImGui::Checkbox("IsVeinGrowth", &Scen->IsVeinGrowth);
			ImGui::Checkbox("IsIceGrowth", &Scen->IsIceGrowth);
			ImGui::Checkbox("IsBridgeChanged", &Scen->IsBridgeChanged);
			ImGui::Checkbox("IsFlagChanged", &Scen->IsFlagChanged);
			ImGui::Checkbox("IsAmbientChanged", &Scen->IsAmbientChanged);
			ImGui::Checkbox("IsEndOfGame", &Scen->IsEndOfGame);
			ImGui::Checkbox("IsInheritTimer", &Scen->IsInheritTimer);
			ImGui::Checkbox("IsSkipScore", &Scen->IsSkipScore);
			ImGui::Checkbox("IsOneTimeOnly", &Scen->IsOneTimeOnly);
			ImGui::Checkbox("IsNoMapSel", &Scen->IsNoMapSel);
			ImGui::Checkbox("IsTruckCrate", &Scen->IsTruckCrate);
			ImGui::Checkbox("IsMoneyTiberium", &Scen->IsMoneyTiberium);
			ImGui::Checkbox("IsTiberiumDeathToVisceroid", &Scen->IsTiberiumDeathToVisceroid);
			ImGui::Checkbox("IsIgnoreGlobalAITriggers", &Scen->IsIgnoreGlobalAITriggers);
			ImGui::Checkbox("IsGDI", &Scen->IsGDI);
			ImGui::Checkbox("IsMultiplayerOnly", &Scen->IsMultiplayerOnly);
			ImGui::Checkbox("IsRandom", &Scen->IsRandom);
			ImGui::Checkbox("CratePickedUp", &Scen->CratePickedUp);

			ImGui::End();
		}
	}

	// 3. Show another simple window.
	//if (show_another_window) {
	//    ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
	//    ImGui::Text("Hello from another window!");
	//    if (ImGui::Button("Close Me")) {
	//        show_another_window = false;
	//    }
	//    ImGui::End();
	//}
#endif

	ImGui_Render_Frame();

	ImGui_End_Frame();

#if 0
	// Update and Render additional Platform Windows
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
#endif
}

void PhobosWindowClass::MessageHandler()
{
	if (!ImGuiMainWindow)
	{
		return;
	}

	MSG msg;

	// Poll and handle messages (inputs, window resize, etc.)
	// See the WndProc() function below for our to dispatch events to the Win32 backend.
	while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_QUIT)
		{
			break;
		}
	}
}

bool PhobosWindowClass::TeamList()
{
	ImGui::Begin("Team List");

	for (int index = 0; index < TeamClass::Array->Count; ++index)
	{
		TeamClass* team = TeamClass::Array->Items[index];
		if (!team) continue;

		// Use SetNextItemOpen() so set the default state of a node to be open. We could
		// also use TreeNodeEx() with the ImGuiTreeNodeFlags_DefaultOpen flag to achieve the same thing!
		if (index == 0)
		{
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		}

		const auto& [cur, act] = team->CurrentScript->GetCurrentAction();

		auto DrawColored = [&]() {
			const ImU32 color = (int)cur == -1 ? IM_COL32(255, 0, 0, 255) : IM_COL32(0, 255, 0, 255);
			ImGui::PushStyleColor(ImGuiCol_Text, color);
			bool imguiresult = ImGui::TreeNode((void*)(intptr_t)index, team->Type->ID);
			ImGui::PopStyleColor();
			return imguiresult;
		};

		if (DrawColored())
		{
			if(ImGui::TreeNode((void*)(intptr_t)1,"Taskforces [%s[0x%x]", team->Type->TaskForce ? team->Type->TaskForce->ID : GameStrings::NoneStr(), team->Type->TaskForce)){
				if (team->Type->TaskForce) {
					ImGui::Text("CountEntries %d", team->Type->TaskForce->CountEntries);

					for (int i = 0; i < std::size(team->Type->TaskForce->Entries); ++i) {
						ImGui::Text("Entry[%d] 0x%x(%s - %s) - %d", i ,
							team->Type->TaskForce->Entries[i].Type ,
							team->Type->TaskForce->Entries[i].Type ? team->Type->TaskForce->Entries[i].Type->ID : GameStrings::NoneStr(),
							team->Type->TaskForce->Entries[i].Type ? team->Type->TaskForce->Entries[i].Type->GetThisClassName() : GameStrings::NoneStr(),
							team->Type->TaskForce->Entries[i].Amount
						);
					}
				}

				ImGui::TreePop();
			}

			ImGui::Text("Current [Act %d Val %d]", (int)cur, act);

			const auto& [Nextcur, Nextact] = team->CurrentScript->GetNextAction();
			ImGui::Text("Next [Act %d Val %d]", Nextcur, Nextact);

			FootClass* pCur = nullptr;
			if (auto pFirst = team->FirstUnit)
			{
				auto pNext = pFirst->NextTeamMember;

				do
				{
					ImGui::Text("Foot[%s(0x%x) - %s]", pFirst->GetTechnoType()->ID, pFirst, pFirst->GetThisClassName());
					pCur = pNext;

					if (pNext)
						pNext = pNext->NextTeamMember;

					pFirst = pCur;

				}
				while (pCur);
			}

			for (int i = 0; i < 6; ++i)
			{
				ImGui::Text("CountObjects %d : %d", i, team->CountObjects[i]);
			}

			if (ImGui::TreeNode((void*)(intptr_t)0, "Script : %s(0x%x)", team->CurrentScript->Type->ID, team->CurrentScript))
			{
				for (int i = 0; i < team->CurrentScript->Type->ActionsCount; ++i)
				{
					const auto& [_cur, _act] = team->CurrentScript->Type->ScriptActions[i];
					ImGui::Text("Action at %d [Act %d Val %d]", i, _cur, _act);
				}
				ImGui::TreePop();
			}

			ImGui::Text("Tag : %s(0x%x)", team->Tag ? team->Tag->Type->ID : GameStrings::NoneStr(), team->Tag);
			ImGui::Text("Owner [%s(0x%x)]", team->Owner ? team->Owner->Type->ID : GameStrings::NoneStr(), team->Owner);
			ImGui::Text("Target [%s(0x%x)]", team->Target ? team->Target->Type->ID : GameStrings::NoneStr(), team->Target);
			ImGui::Text("TotalObjects %d", team->TotalObjects);
			ImGui::Text("TotalThreatValue %d", team->TotalThreatValue);
			ImGui::Text("CreationFrame %d", team->CreationFrame);
			ImGui::Text("IsTransient %d", team->IsTransient);
			ImGui::Text("NeedsReGrouping %d", team->NeedsReGrouping);
			ImGui::Text("GuardSlowerIsNotUnderStrength %d", team->GuardSlowerIsNotUnderStrength);
			ImGui::Text("IsForcedActive %d", team->IsForcedActive);
			ImGui::Text("IsHasBeen %d", team->IsHasBeen);
			ImGui::Text("IsFullStrength %d", team->IsFullStrength);
			ImGui::Text("IsUnderStrength %d", team->IsUnderStrength);
			ImGui::Text("IsReforming %d", team->IsReforming);
			ImGui::Text("IsLagging %d", team->IsLagging);
			ImGui::Text("NeedsToDisappear %d", team->NeedsToDisappear);
			ImGui::Text("JustDisappeared %d", team->JustDisappeared);
			ImGui::Text("IsMoving %d", team->IsMoving);
			ImGui::Text("StepCompleted %d", team->StepCompleted);
			ImGui::Text("TargetNotAssigned %d", team->TargetNotAssigned);
			ImGui::Text("IsLeavingMap %d", team->IsLeavingMap);
			ImGui::Text("IsSuspended %d", team->IsSuspended);
			ImGui::Text("AchievedGreatSuccess %d", team->AchievedGreatSuccess);
			ImGui::Text("QueuedFocus [0x%x(%s)]", team->QueuedFocus , team->QueuedFocus ? team->QueuedFocus->GetThisClassName() : GameStrings::NoneStr());
			ImGui::Text("ArchiveTarget [0x%x(%s)]", team->ArchiveTarget, team->ArchiveTarget ? team->ArchiveTarget->GetThisClassName() : GameStrings::NoneStr());
			ImGui::Text("SpawnCell [0x%x(%d , %d)]", team->SpawnCell, team->SpawnCell ? team->SpawnCell->MapCoords.X : 0 , team->SpawnCell ? team->SpawnCell->MapCoords.Y : 0);

			ImGui::TreePop();
		}
	}

	ImGui::End();

	return true;
}

bool PhobosWindowClass::ScriptTypeList()
{
	ImGui::Begin("ScriptType List");

	for (int index = 0; index < ScriptTypeClass::Array->Count; ++index)
	{
		ScriptTypeClass* scriptType = ScriptTypeClass::Array->Items[index];
		if (!scriptType) continue;

		// Use SetNextItemOpen() so set the default state of a node to be open. We could
		// also use TreeNodeEx() with the ImGuiTreeNodeFlags_DefaultOpen flag to achieve the same thing!
		if (index == 0)
		{
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		}

		if (ImGui::TreeNode((void*)(intptr_t)index, scriptType->ID))
		{
			for (int i = 0; i < scriptType->ActionsCount; ++i)
			{
				const auto& [_cur, _act] = scriptType->ScriptActions[i];
				ImGui::Text("Action at %d [Act %d Val %d]", i, _cur, _act);
			}

			ImGui::TreePop();
		}
	}

	ImGui::End();

	return true;
}

bool PhobosWindowClass::TriggerList()
{
	//ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize;

	//ImVec2 window_pos(1350, 0);
	//ImGui::SetNextWindowPos(window_pos);

	//ImVec2 window_size(500, 720);
	//ImGui::SetNextWindowSize(window_size);

	//ImGui::Begin("Trigger List", nullptr, window_flags);
	ImGui::Begin("Trigger List");

	if (ImGui::TreeNode("Triggers"))
	{
		for (int index = 0; index < TriggerClass::Array->Count; ++index)
		{

			TriggerClass* trigger = TriggerClass::Array->Items[index];
			if (!trigger) continue;

			// Use SetNextItemOpen() so set the default state of a node to be open. We could
			// also use TreeNodeEx() with the ImGuiTreeNodeFlags_DefaultOpen flag to achieve the same thing!
			if (index == 0)
			{
				ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			}

			if (ImGui::TreeNode((void*)(intptr_t)index, trigger->Type->ID))
			{
				ImGui::Text("blah blah");
				ImGui::SameLine();
				if (ImGui::SmallButton("button"))
				{
				}
				ImGui::TreePop();
			}

		}

		ImGui::TreePop();
	}

	if (ImGui::Button("New"))
	{
	}

	if (ImGui::Button("Edit"))
	{
	}

	if (ImGui::Button("Delete"))
	{
	}

	if (ImGui::Button("Place"))
	{
	}

	ImGui::End();

	return true;
}

#endif