#pragma once
#include <Base/Always.h>
#include <filesystem>
#include <ddraw.h>
#include <Helpers/CompileTime.h>
#include <Utilities/Patch.h>

enum class RendererType {
	Unknown,
	D3D9,        // ts-ddraw/cnc-ddraw D3D mode
	D3D10,
	D3D11,
	D3D12,
	OpenGL       // cnc-ddraw OpenGL mode
};

struct GraphicsRuntimeAPI
{
	enum class Type
	{
		UNK, DX9, DX10, DX11, DX12, DXGI, OGL, VK
	};

	GraphicsRuntimeAPI(const std::vector<dllData>& dlls);

	~GraphicsRuntimeAPI() = default;

	FORCEDINLINE COMPILETIMEEVAL const char* GetName() const
	{
		return name.c_str();
	}

	FORCEDINLINE COMPILETIMEEVAL Type GetType()
	{
		return type;
	}

private:
	std::string name;
	Type type;
};

struct ReShadeIntegration
{
	static COMPILETIMEEVAL reference<LPDIRECTDRAW*, 0x8A0094> DirectDrawObject {};

	static RendererType g_detected_renderer;
	static bool g_enabled;
	static const char* version;
	static std::filesystem::path g_reshade_dll_path;
	static std::filesystem::path g_reshade_base_path;
	static std::filesystem::path g_target_executable_path;
	static HMODULE g_module_handle;
	static bool DXHooks;
	static bool OGLHooks;

	static void InitializeFromDllMain(HMODULE phobos_module);
	static void Unload();
	static void InstallHooks(GraphicsRuntimeAPI::Type tt);
	static bool ReshadeAlreadyRunning(HMODULE phobos_module);
	static void KeyPressed();

	static void InstallDx9Hooks();
	static void InstallDx10Hooks();
	static void InstallDx11Hooks();
	static void InstallDx12Hooks();
	static void InstallOglHooks();

};