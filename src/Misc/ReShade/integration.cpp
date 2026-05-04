#include "integration.h"
#include "ini_file.hpp"

#include "Hooks/hook_manager.hpp"

#include <Utilities/Debug.h>

#include <Psapi.h>

const char*  ReShadeIntegration::version = "Main Branch v6";
extern "C" __declspec(dllexport) const char* ReShadeVersion = ReShadeIntegration::version;

std::filesystem::path ReShadeIntegration::g_reshade_dll_path;
std::filesystem::path ReShadeIntegration::g_reshade_base_path;
std::filesystem::path ReShadeIntegration::g_target_executable_path;
HMODULE ReShadeIntegration::g_module_handle;
bool ReShadeIntegration::DXHooks;
bool ReShadeIntegration::OGLHooks;

/// <summary>
/// Returns the path that should be used as base for relative paths.
/// </summary>
std::filesystem::path get_base_path(bool default_to_target_executable_path)
{
	const std::filesystem::path reshade_dll_parent_path = ReShadeIntegration::g_reshade_dll_path.parent_path();
	const std::filesystem::path target_executable_parent_path = ReShadeIntegration::g_target_executable_path.parent_path();

	std::error_code ec;
	std::filesystem::path path_override;

	// Cannot use global config here yet, since it uses base path for look up, so look at config file next to target executable instead
	if (reshade::ini_file(target_executable_parent_path / L"ReShade.ini").get("INSTALL", "BasePath", path_override) &&
		PhobosCRT::resolve_path(path_override, ec, reshade_dll_parent_path) && std::filesystem::is_directory(path_override, ec))
		return path_override;

	WCHAR buf[4096];
	path_override.assign(buf, buf + GetEnvironmentVariableW(L"RESHADE_BASE_PATH_OVERRIDE", buf, ARRAYSIZE(buf)));
	if (PhobosCRT::resolve_path(path_override, ec, reshade_dll_parent_path) && std::filesystem::is_directory(path_override, ec))
		return path_override;

	return default_to_target_executable_path ? target_executable_parent_path : reshade_dll_parent_path;
}

RendererType ReShadeIntegration::g_detected_renderer = RendererType::Unknown;
bool ReShadeIntegration::g_enabled = false;

GraphicsRuntimeAPI::GraphicsRuntimeAPI(const std::vector<dllData>& dlls)
	: name { "Unknown" }, type { Type::UNK }
{
	for (auto& dll : dlls)
	{
		if (_strnicmp(dll.ModuleName.c_str(), "d3d", 3) == 0)
		{
			if (IS_SAME_STR_("d3d9.dll", dll.ModuleName.c_str()))
			{
				name = "DirectX9";
				type = Type::DX9;
			}
			else if (IS_SAME_STR_("d3d10.dll", dll.ModuleName.c_str()))
			{
				name = "DirectX10";
				type = Type::DX10;
			}
			else if (IS_SAME_STR_("d3d11.dll", dll.ModuleName.c_str()))
			{
				name = "DirectX11";
				type = Type::DX11;
			}
			else if (IS_SAME_STR_("d3d12.dll", dll.ModuleName.c_str()))
			{
				name = "DirectX12";
				type = Type::DX12;
			}

			break;
		}
		else if (IS_SAME_STR_("opengl32.dll", dll.ModuleName.c_str()))
		{
			name = "OpenGL";
			type = Type::OGL;
			break;
		}
		else if (IS_SAME_STR_("vulkan-1.dll", dll.ModuleName.c_str()))
		{
			name = "Vulkan";
			type = Type::VK;
			break;
		}
	}
}


void ReShadeIntegration::InitializeFromDllMain(HMODULE phobos_module)
{
	ReShadeIntegration::g_reshade_dll_path = PhobosCRT::get_module_path(phobos_module);
	ReShadeIntegration::g_target_executable_path = PhobosCRT::get_module_path(nullptr);
	ReShadeIntegration::g_reshade_base_path = get_base_path(true);

	// 1. Load config (safe in DllMain)
	reshade::ini_file& config = reshade::global_config();

	// 2. Check if disabled by config
	std::error_code ec;
	if (!std::filesystem::exists(config.path(), ec))
	{
		config.clear();
		g_enabled = false;
		Debug::LogDeferred("[ReShade] Config not found, disabled\n");
		return;
	}

	if (!config.get("GENERAL", "Enabled", g_enabled) || !g_enabled)
	{
		config.clear();
		g_enabled = false;
		Debug::LogDeferred("[ReShade] Disabled in config\n");
		return;
	}

	if (ReshadeAlreadyRunning(phobos_module)) {
		config.clear();
		g_enabled = false;
		Debug::LogDeferred("[ReShade] Another ReShade instance is already running\n");
		return;
	}

	ReShadeIntegration::DXHooks = config.get("INSTALL", "HookDirectX");
	ReShadeIntegration::OGLHooks = config.get("INSTALL", "HookOpenGL");

	if (!ReShadeIntegration::DXHooks && !ReShadeIntegration::OGLHooks)
	{
		config.clear();
		g_enabled = false;
		Debug::LogDeferred("[ReShade] Another ReShade instance is already running\n");
		return;
	}

	ReShadeIntegration::g_module_handle = phobos_module;

	if (config.get("INSTALL", "PreventUnloading"))
	{
		HMODULE pinned = nullptr;
		GetModuleHandleExW(
			GET_MODULE_HANDLE_EX_FLAG_PIN | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
			reinterpret_cast<LPCWSTR>(phobos_module),
			&pinned
		);
	}

	Debug::LogDeferred("[ReShade] DllMain init complete\n");
}

void ReShadeIntegration::InstallDx9Hooks()
{
	auto sysPath = PhobosCRT::get_system_path();
	reshade::hooks::register_module(sysPath / L"d3d9.dll");
	reshade::hooks::register_module(sysPath / L"dxgi.dll");
	g_detected_renderer = RendererType::D3D9;
}

void ReShadeIntegration::InstallDx10Hooks()
{
	auto sysPath = PhobosCRT::get_system_path();
	reshade::hooks::register_module(sysPath / L"d3d10.dll");
	reshade::hooks::register_module(sysPath / L"d3d10_1.dll");
	reshade::hooks::register_module(sysPath / L"dxgi.dll");
	g_detected_renderer = RendererType::D3D10;
}

void ReShadeIntegration::InstallDx11Hooks()
{
	auto sysPath = PhobosCRT::get_system_path();
	reshade::hooks::register_module(sysPath / L"d3d11.dll");
	reshade::hooks::register_module(sysPath / L"dxgi.dll");
	g_detected_renderer = RendererType::D3D11;
}

void ReShadeIntegration::InstallDx12Hooks()
{
	auto sysPath = PhobosCRT::get_system_path();
	reshade::hooks::register_module(sysPath / L"d3d12.dll");
	reshade::hooks::register_module(sysPath / L"dxgi.dll");
	g_detected_renderer = RendererType::D3D12;
}

void ReShadeIntegration::InstallOglHooks()
{
	auto sysPath = PhobosCRT::get_system_path();
	reshade::hooks::register_module(sysPath / L"opengl32.dll");
	g_detected_renderer = RendererType::OpenGL;
}

void ReShadeIntegration::Unload()
{
	if (!g_enabled)
		return;

	reshade::hooks::uninstall();
}

void ReShadeIntegration::InstallHooks(GraphicsRuntimeAPI::Type tt)
{
	if (!g_enabled) return;

	switch (tt)
	{
	case GraphicsRuntimeAPI::Type::UNK:
		return;
	case GraphicsRuntimeAPI::Type::DX9:
		if (!ReShadeIntegration::DXHooks)
			return;

		g_detected_renderer = RendererType::D3D9;
		InstallDx9Hooks();
		break;
	case GraphicsRuntimeAPI::Type::DX10:
		if (!ReShadeIntegration::DXHooks)
			return;

		g_detected_renderer = RendererType::D3D10;
		InstallDx10Hooks();
		break;
	case GraphicsRuntimeAPI::Type::DX11:
		if (!ReShadeIntegration::DXHooks)
			return;

		g_detected_renderer = RendererType::D3D11;
		InstallDx11Hooks();
		break;
	case GraphicsRuntimeAPI::Type::DX12:
		if (!ReShadeIntegration::DXHooks)
			return;

		g_detected_renderer = RendererType::D3D12;
		InstallDx12Hooks();
		break;
	case GraphicsRuntimeAPI::Type::OGL:

		if (!ReShadeIntegration::OGLHooks)
			return;

		g_detected_renderer = RendererType::OpenGL;
		InstallOglHooks();
		break;
	default:
		break;
	}
}

bool ReShadeIntegration::ReshadeAlreadyRunning(HMODULE hModule) {

	DWORD fdwReason;
	if (HMODULE modules[1024]; K32EnumProcessModules(GetCurrentProcess(), modules, sizeof(modules), &fdwReason)) // Use kernel32 variant which is available in DllMain { 
	{
		// Skip first module (the main application module)
		for (DWORD i = 1; i < std::min<DWORD>(fdwReason / sizeof(HMODULE), std::size(modules)); ++i) {
			if (modules[i] != hModule && GetProcAddress(modules[i], "ReShadeVersion") != nullptr) {
				return true; // Make the 'LoadLibrary' call that loaded this instance fail
			}
		}
	}

	return false;
}

//TODO : hook onto the game 
void ReShadeIntegration::KeyPressed() {

}