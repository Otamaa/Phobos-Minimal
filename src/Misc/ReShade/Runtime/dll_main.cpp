/*
 * Copyright (C) 2014 Patrick Mours
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Modified for static integration into Phobos.dll.
 *
 * Differences from upstream 6.x that matter:
 *  - This module is NOT named d3d9/dxgi/opengl32/etc. It is "Phobos", so every
 *    upstream code path that keyed off the module filename is either dead or has
 *    silently changed meaning. Those places are marked below.
 *  - We are not loaded by proxy, so the "only initialize when a ReShade.ini
 *    exists" gate is disabled. That gate is what upstream used to stop the
 *    implicit Vulkan layer from loading everywhere; it does not apply here.
 *  - Lifetime is owned by Phobos's DllMain, which may call Detach() even if
 *    Attach() failed. Guarded with a latch.
 */
#include "dll_main.h"
#include "Reshadeversion.h"
#include "dll_log.hpp"
#include "ini_file.hpp"
#include "hook_manager.hpp"
#include <Windows.h>
#include <Psapi.h>
#include <delayimp.h> // Delay-load helpers

 // Export special symbol to identify modules as ReShade instances.
 //
 // NOTE: this makes Phobos.dll itself advertise as a ReShade instance. A user's
 // own d3d9.dll/dxgi.dll ReShade will see this export and abort its own
 // initialization, and we will do the same to it (see HasOtherReshadeInstance).
 // That mutual exclusion is deliberate - two instances hooking the same device
 // is worse than none.
extern "C" __declspec(dllexport) const char* ReShadeVersion = RESHADE_VERSION_STRING_PRODUCT;

HANDLE g_exit_event = nullptr;
HMODULE g_module_handle = nullptr;
std::filesystem::path g_reshade_dll_path;
std::filesystem::path g_reshade_base_path;
std::filesystem::path g_target_executable_path;

extern bool resolve_path(std::filesystem::path& path, std::error_code& ec, const std::filesystem::path& base);

// ---------------------------------------------------------------------------
// Integration policy
//
// These were config-driven or filename-driven upstream. In a static integration
// they are decisions about how Phobos behaves, so they are pinned here rather
// than left to an ini file a user might not have.
// ---------------------------------------------------------------------------

// Put the base path next to gamemd.exe so ReShade.ini and reshade-shaders\ land
// where users expect them, not next to wherever Phobos.dll happens to sit.
static constexpr bool POLICY_BASE_AT_TARGET_EXECUTABLE = true;

// Never abort just because ReShade.ini is missing - Phobos owns our lifetime.
static constexpr bool POLICY_REQUIRE_CONFIG_FILE = false;

// PreventUnloading would pin Phobos.dll, not a standalone ReShade DLL. Syringe
// already keeps Phobos resident, so honouring this ini key would only let a
// stray config change Phobos's unload semantics.
static constexpr bool POLICY_ALLOW_PREVENT_UNLOADING = false;

// ---------------------------------------------------------------------------
// Environment queries
// ---------------------------------------------------------------------------

/// <summary>
/// Checks whether the current application is an UWP app.
/// </summary>
bool is_uwp_app()
{
	const auto GetCurrentPackageFullName = reinterpret_cast<LONG(WINAPI*)(UINT32*, PWSTR)>(GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "GetCurrentPackageFullName"));
	if (GetCurrentPackageFullName == nullptr)
		return false;

	// This will return APPMODEL_ERROR_NO_PACKAGE if not a packaged UWP app
	UINT32 length = 0;
	return GetCurrentPackageFullName(&length, nullptr) == ERROR_INSUFFICIENT_BUFFER;
}

/// <summary>
/// Checks whether the current operating system is Windows 7 or earlier.
/// </summary>
bool is_windows7()
{
	ULONGLONG condition = 0;
	VER_SET_CONDITION(condition, VER_MAJORVERSION, VER_LESS_EQUAL);
	VER_SET_CONDITION(condition, VER_MINORVERSION, VER_LESS_EQUAL);

	OSVERSIONINFOEX verinfo_windows7 = { sizeof(verinfo_windows7), 6, 1 };
	return VerifyVersionInfo(&verinfo_windows7, VER_MAJORVERSION | VER_MINORVERSION, condition) != FALSE;
}

// ---------------------------------------------------------------------------
// Path helpers
// ---------------------------------------------------------------------------

/// <summary>
/// Returns the path to the Windows System32 directory.
/// </summary>
std::filesystem::path get_system_path()
{
	WCHAR buf[4096];
	const UINT length = GetSystemDirectoryW(buf, ARRAYSIZE(buf));
	if (length == 0 || length > ARRAYSIZE(buf))
		return std::filesystem::path();

	return std::filesystem::path(buf, buf + length);
}

/// <summary>
/// Returns the path to the module file identified by the specified <paramref name="module"/> handle.
/// </summary>
std::filesystem::path get_module_path(HMODULE module)
{
	WCHAR buf[4096];
	const DWORD length = GetModuleFileNameW(module, buf, ARRAYSIZE(buf));
	if (length == 0 || length >= ARRAYSIZE(buf))
		return std::filesystem::path();

	return std::filesystem::path(buf, buf + length);
}

/// <summary>
/// Reads the RESHADE_BASE_PATH_OVERRIDE environment variable, if set.
/// </summary>
static std::filesystem::path get_base_path_from_environment()
{
	WCHAR buf[4096];
	const DWORD length = GetEnvironmentVariableW(L"RESHADE_BASE_PATH_OVERRIDE", buf, ARRAYSIZE(buf));
	if (length == 0 || length >= ARRAYSIZE(buf))
		return std::filesystem::path();

	return std::filesystem::path(buf, buf + length);
}

/// <summary>
/// Returns the path that should be used as base for relative paths.
/// </summary>
std::filesystem::path get_base_path(bool default_to_target_executable_path = false)
{
	const std::filesystem::path reshade_dll_parent_path = g_reshade_dll_path.parent_path();
	const std::filesystem::path target_executable_parent_path = g_target_executable_path.parent_path();

	std::error_code ec;

	// Cannot use global config here yet, since it uses base path for look up, so
	// look at config file next to target executable instead
	std::filesystem::path path_override;
	if (reshade::ini_file(target_executable_parent_path / L"ReShade.ini").get("INSTALL", "BasePath", path_override) &&
		resolve_path(path_override, ec, reshade_dll_parent_path) && std::filesystem::is_directory(path_override, ec))
		return path_override;

	path_override = get_base_path_from_environment();
	if (!path_override.empty() &&
		resolve_path(path_override, ec, reshade_dll_parent_path) && std::filesystem::is_directory(path_override, ec))
		return path_override;

	return default_to_target_executable_path ? target_executable_parent_path : reshade_dll_parent_path;
}

// ---------------------------------------------------------------------------
// Config helpers
//
// 'ini_file::get' leaves its output untouched when the key is absent, and the
// single-argument form therefore reports false for "missing" as well as for
// "explicitly disabled". Every boolean read below goes through this so a
// missing key means the intended default, not silently off.
// ---------------------------------------------------------------------------

static bool ConfigFlag(const reshade::ini_file& config, const char* section, const char* key, bool fallback)
{
	bool value = fallback;
	config.get(section, key, value);
	return value;
}

static bool HasEnvironmentFlag(const wchar_t* name)
{
	return GetEnvironmentVariableW(name, nullptr, 0) != 0;
}

#ifndef RESHADE_TEST_APPLICATION

// Set only once Attach() has fully succeeded, so Detach() cannot tear down state
// that was never built. Phobos's DllMain does not need to track this itself.
static bool s_attached = false;

// ---------------------------------------------------------------------------
// Initialization steps
// ---------------------------------------------------------------------------

/// <summary>
/// Resolves the module, target executable and base paths.
/// </summary>
static void InitPaths(HMODULE hModule)
{
	g_module_handle = hModule;
	g_reshade_dll_path = get_module_path(hModule);
	g_target_executable_path = get_module_path(nullptr);

	// UWP apps do not have write access to the application directory, so never
	// default the base path to it for them. gamemd.exe never is one, but the
	// check costs nothing and keeps the intent explicit.
	const bool base_at_target_executable = POLICY_BASE_AT_TARGET_EXECUTABLE && !is_uwp_app();

	g_reshade_base_path = get_base_path(base_at_target_executable);
}

/// <summary>
/// Upstream refused to initialize unless a configuration file existed for the
/// target executable. Kept behind a policy constant for reference; disabled for
/// the Phobos build.
/// </summary>
static bool IsConfigFilePresent(const reshade::ini_file& config)
{
	if constexpr (!POLICY_REQUIRE_CONFIG_FILE)
	{
		(void)config;
		return true;
	}
	else
	{
		if (HasEnvironmentFlag(L"RESHADE_DISABLE_LOADING_CHECK"))
			return true;

		std::error_code ec;
		if (std::filesystem::exists(config.path(), ec))
			return true;

#ifndef NDEBUG
		// Log was not yet opened at this point, so this only writes to debug output
		reshade::log::message(reshade::log::level::warning, "ReShade was not enabled for '%s'! Aborting initialization ...", g_target_executable_path.u8string().c_str());
#endif
		return false;
	}
}

/// <summary>
/// Opens the log file next to the configuration file, retrying with numbered
/// extensions when the default is already held by another process.
/// </summary>
static void OpenLogFile(const reshade::ini_file& config)
{
	// NOTE: verify that 'global_config()' in your tree resolves to
	// 'g_reshade_base_path / "ReShade.ini"' and does not derive the name from
	// 'g_reshade_dll_path'. In the latter case this would be looking for
	// "Phobos.ini" / "Phobos.log", which is a common source of "my settings are
	// being ignored" reports after a rename.
	std::filesystem::path log_path = config.path();
	log_path.replace_extension(L".log");

	std::error_code ec;
	if (reshade::log::open_log_file(log_path, ec))
		return;

	// Try a different file if the default failed to open (e.g. because currently in use by another ReShade instance)
	for (int log_index = 0; log_index < 10 && std::filesystem::exists(log_path, ec); ++log_index)
	{
		log_path.replace_extension(L".log" + std::to_wstring(log_index + 1));

		if (reshade::log::open_log_file(log_path, ec))
			return;
	}

#ifndef NDEBUG
	if (ec)
		reshade::log::message(reshade::log::level::error, "Opening the ReShade log file failed with error code %d.", ec.value());
#endif
}

/// <summary>
/// Writes the startup banner.
/// </summary>
static void LogBanner()
{
	reshade::log::message(reshade::log::level::info,
		"Initializing crosire's ReShade version '" RESHADE_VERSION_STRING_FILE "' "
		"(32-bit, hosted by Phobos) "
		"loaded from '%s' into '%s' (0x%X) ...",
		g_reshade_dll_path.string().c_str(),
#ifndef NDEBUG
		static_cast<const char*>(GetCommandLineA()),
#else
		// Do not log full command-line in release builds, since it may contain sensitive information like authentication tokens
		g_target_executable_path.string().c_str(),
#endif
		static_cast<unsigned int>(std::hash<std::string>()(g_target_executable_path.stem().string()) & 0xFFFFFFFF));
}

/// <summary>
/// Checks whether another ReShade instance was already loaded into the process.
/// </summary>
static bool HasOtherReshadeInstance(HMODULE hModule)
{
	HMODULE modules[1024];
	DWORD modules_size = 0;

	// Use kernel32 variant which is available in DllMain
	if (!K32EnumProcessModules(GetCurrentProcess(), modules, sizeof(modules), &modules_size))
		return false;

	const DWORD count = std::min<DWORD>(modules_size / sizeof(HMODULE), static_cast<DWORD>(std::size(modules)));

	// Skip first module (the main application module)
	for (DWORD i = 1; i < count; ++i)
	{
		if (modules[i] == hModule)
			continue;

		if (GetProcAddress(modules[i], "ReShadeVersion") == nullptr)
			continue;

		reshade::log::message(reshade::log::level::warning, "Another ReShade instance was already loaded from '%s'! Aborting initialization ...", get_module_path(modules[i]).u8string().c_str());
		return true;
	}

	return false;
}

// ---------------------------------------------------------------------------
// Hook registration
// ---------------------------------------------------------------------------

/// <summary>
/// Registers the proxy export module, if one is configured. Returns its path so
/// the graphics hooks know whether to accept any module of a given name rather
/// than only the system one.
/// </summary>
static std::filesystem::path RegisterProxyModule(const reshade::ini_file& config)
{
	// Proxying makes little sense for a static integration - Phobos.dll is not
	// standing in for anything - so this defaults off.
	if (!ConfigFlag(config, "PROXY", "EnableProxyLibrary", false))
		return std::filesystem::path();

	std::filesystem::path export_module_path;
	if (!config.get("PROXY", "ProxyLibrary", export_module_path))
		return std::filesystem::path();

	reshade::hooks::register_export_module(g_reshade_base_path / export_module_path);
	return export_module_path;
}

static void RegisterInputHooks()
{
	if (HasEnvironmentFlag(L"RESHADE_DISABLE_INPUT_HOOK"))
		return;

	g_exit_event = CreateEvent(nullptr, TRUE, FALSE, nullptr);

	reshade::hooks::register_module(L"user32.dll");

	reshade::hooks::register_module(get_system_path() / L"dinput.dll");
	reshade::hooks::register_module(get_system_path() / L"dinput8.dll");
}

static void RegisterNetworkHooks()
{

}

static void RegisterGraphicsHooks(const reshade::ini_file& config, const std::filesystem::path& export_module_path)
{
	if (HasEnvironmentFlag(L"RESHADE_DISABLE_GRAPHICS_HOOK"))
		return;

	// Upstream keyed this off the module not being named opengl32.dll. Defaults
	// to true here: with the single-argument 'get' a missing key silently
	// disabled every graphics hook, which looks exactly like ReShade loading and
	// then doing nothing at all.
	if (!ConfigFlag(config, "INSTALL", "HookDirectX", true))
		return;

	reshade::hooks::register_module(get_system_path() / L"d3d9.dll");
}

static void RegisterHookModules(const reshade::ini_file& config)
{
	const std::filesystem::path export_module_path = RegisterProxyModule(config);

	RegisterInputHooks();
	RegisterNetworkHooks();
	RegisterGraphicsHooks(config, export_module_path);
}

// ---------------------------------------------------------------------------
// Early add-on loading
// ---------------------------------------------------------------------------

static void LoadEarlyAddons(const reshade::ini_file& config)
{
}

// ---------------------------------------------------------------------------
// Attach / Detach
// ---------------------------------------------------------------------------

/// <summary>
/// Initializes ReShade. Call from Phobos's DllMain on DLL_PROCESS_ATTACH.
///
/// Returning FALSE means "ReShade is disabled, carry on" - it must NOT be
/// propagated as a DllMain failure. Upstream returned FALSE to make the
/// 'LoadLibrary' that loaded ReShade fail; failing Phobos's load instead would
/// take the whole game down.
///
/// Do NOT call 'DisableThreadLibraryCalls' in the hosting DllMain: ReShade links
/// against the static CRT, which requires thread notifications to work properly.
/// (It does not do anything when static TLS is used anyway, which is the case -
/// see https://docs.microsoft.com/windows/win32/api/libloaderapi/nf-libloaderapi-disablethreadlibrarycalls)
/// </summary>
BOOL ReshadeContainer::Attach(HMODULE hModule)
{
	if (s_attached)
		return TRUE;

	InitPaths(hModule);

	const reshade::ini_file& config = reshade::global_config();

	if (!IsConfigFilePresent(config))
		return FALSE;

	if (ConfigFlag(config, "INSTALL", "Logging", !HasEnvironmentFlag(L"RESHADE_DISABLE_LOGGING")))
		OpenLogFile(config);

	LogBanner();

	if (HasOtherReshadeInstance(hModule)) {
		return FALSE;
	}

	if constexpr (POLICY_ALLOW_PREVENT_UNLOADING)
	{
		if (ConfigFlag(config, "INSTALL", "PreventUnloading", false))
			GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_PIN | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCWSTR>(hModule), &hModule);
	}

	RegisterHookModules(config);

	reshade::log::message(reshade::log::level::info, "Initialized.");

	LoadEarlyAddons(config);

	s_attached = true;
	return TRUE;
}

/// <summary>
/// Tears ReShade down. Safe to call unconditionally - it is a no-op when Attach
/// never completed.
/// </summary>
void ReshadeContainer::Detach()
{
	if (!s_attached)
		return;

	s_attached = false;

	reshade::log::message(reshade::log::level::info, "Exiting ...");

	reshade::hooks::uninstall();

	// Module is now invalid, so break out of any message loops that may still have it in the call stack (see 'HookGetMessage' implementation in input_windows.cpp)
	// This is necessary since a different thread may have called into the 'GetMessage' hook from ReShade, but may not receive a message until after the ReShade module was unloaded
	// At that point it would return to code that was already unloaded and crash
	// Hooks were already uninstalled now, so after returning from any existing 'GetMessage' hook call, application will call the real one next and things continue to work
	//
	// NOTE: this sleeps for a full second. Acceptable on process exit; if Phobos
	// ever calls Detach() at any other point, this will visibly stall the game.
	if (g_exit_event != nullptr)
	{
		SetEvent(g_exit_event);
		Sleep(1000);
		CloseHandle(g_exit_event);
		g_exit_event = nullptr;
	}

	reshade::log::message(reshade::log::level::info, "Finished exiting.");
}

// ---------------------------------------------------------------------------
// Delay-load helper
//
// There is exactly one '__pfnDliNotifyHook2' per module. Phobos.dll now spends
// it here - if Phobos ever needs its own delay-load notify hook, these must be
// merged into a single dispatcher rather than defined twice.
// ---------------------------------------------------------------------------

static FARPROC WINAPI DliNotifyHook2(unsigned dliNotify, PDelayLoadInfo pdli)
{
	if (dliNotify != dliNotePreLoadLibrary || _stricmp(pdli->szDll, "D3DCompiler_47.dll") != 0)
		return nullptr;

	// Prefer loading up-to-date system D3DCompiler DLL over local variants.
	// Do not check system path when running in Wine though, since the
	// D3DCompiler DLL there does not support various features.
	if (GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "wine_get_version") == nullptr)
	{
		if (const HMODULE d3dcompiler_47_module = LoadLibraryW((get_system_path() / L"D3DCompiler_47.dll").c_str()))
			return reinterpret_cast<FARPROC>(d3dcompiler_47_module);
	}

	if (const HMODULE d3dcompiler_47_module = LoadLibraryW(L"D3DCompiler_47.dll"))
		return reinterpret_cast<FARPROC>(d3dcompiler_47_module);

	// Fall back to older D3DCompiler version
	if (const HMODULE d3dcompiler_43_module = LoadLibraryW(L"D3DCompiler_43.dll"))
		return reinterpret_cast<FARPROC>(d3dcompiler_43_module);

	return nullptr;
}

// See https://learn.microsoft.com/cpp/build/reference/understanding-the-helper-function
extern "C" const PfnDliHook __pfnDliNotifyHook2 = DliNotifyHook2;

#endif