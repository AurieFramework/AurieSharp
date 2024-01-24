#include "NetHost.hpp"
#include <cassert>
using namespace Aurie;

AurieStatus RuntimeManager::Initialize(
	IN const char* ManagedComponentName
)
{
	DWORD last_error = 0;
	m_NetHost = GetModuleHandleA("nethost.dll");

	if (!m_NetHost)
	{
		MessageBoxA(
			0,
			"nethost.dll is not loaded in the current process.\n"
			"Scripts will not run, and the runtime will be unavailable!"
			"If you haven't done so, place the nethost DLL in the \"native\" directory.",
			"AurieSharp Script Runtime Host",
			MB_OK | MB_ICONERROR | MB_SETFOREGROUND
		);

		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}

	auto hostfxr_get_path = reinterpret_cast<decltype(&get_hostfxr_path)>(
		GetProcAddress(m_NetHost, "get_hostfxr_path")
	);

	if (!hostfxr_get_path)
	{
		MessageBoxA(
			0,
			"Failed to get function get_hostfxr_path!",
			"AurieSharp Script Runtime Host",
			MB_OK | MB_ICONERROR | MB_SETFOREGROUND
		);

		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}

	// Get the path to the hostfxr.dll file
	wchar_t hostfxr_path[MAX_PATH] = { 0 };
	size_t hostfxr_path_length = MAX_PATH;

	last_error = hostfxr_get_path(
		hostfxr_path, 
		&hostfxr_path_length, 
		nullptr
	);

	if (last_error)
	{
		MessageBoxA(
			0,
			"Failed to find HostFXR location!\n"
			"Do you have .NET 8.0.0 installed?",
			"AurieSharp Script Runtime Host",
			MB_OK | MB_ICONERROR | MB_SETFOREGROUND
		);

		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}

	m_HostFXR = LoadLibraryW(
		hostfxr_path
	);

	if (!m_HostFXR)
	{
		MessageBoxA(
			0,
			"Failed to load HostFXR library!\n"
			"Do you have .NET 8.0.0 installed?",
			"AurieSharp Script Runtime Host",
			MB_OK | MB_ICONERROR | MB_SETFOREGROUND
		);

		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}

	// Get the GAMEDIR folder path
	fs::path game_root_directory;
	AurieStatus last_status = Internal::MdpGetImageFolder(
		g_ArInitialImage,
		game_root_directory
	);

	if (!AurieSuccess(last_status))
		return last_status;

	// Try to create the GAMEDIR/mods/Managed directory
	m_NativeModDirectory = game_root_directory / "mods" / "Native";
	m_AurieModDirectory = game_root_directory / "mods" / "Aurie";
	m_ManagedModDirectory = game_root_directory / "Mods" / "Managed";
	try
	{
		if (!fs::exists(m_ManagedModDirectory))
			fs::create_directory(m_ManagedModDirectory);
	}
	catch (...)
	{
		return AURIE_EXTERNAL_ERROR;
	}

	if (!m_HostFXR)
	{
		MessageBoxA(
			0,
			"HostFXR-8.0.0.dll is not loaded in the current process.\n"
			"Scripts will not run, and the runtime will be unavailable!"
			"If you haven't done so, place the hostfxr DLL in the \"native\" directory.",
			"AurieSharp Script Runtime Host",
			MB_OK | MB_ICONERROR | MB_SETFOREGROUND
		);

		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}

	m_HostFXR_InitializeForRuntimeConfig = reinterpret_cast<hostfxr_initialize_for_runtime_config_fn>(
		GetProcAddress(
			m_HostFXR,
			"hostfxr_initialize_for_runtime_config"
		)
	);

	assert(m_HostFXR_InitializeForRuntimeConfig);

	m_HostFXR_GetRuntimeDelegate = reinterpret_cast<hostfxr_get_runtime_delegate_fn>(
		GetProcAddress(
			m_HostFXR,
			"hostfxr_get_runtime_delegate"
		)
	);

	assert(m_HostFXR_GetRuntimeDelegate);

	m_HostFXR_Close = reinterpret_cast<hostfxr_close_fn>(
		GetProcAddress(
			m_HostFXR,
			"hostfxr_close"
		)
	);

	assert(m_HostFXR_Close);

	hostfxr_handle host_context_handle;

	fs::path runtime_config_path = m_AurieModDirectory / "AurieSharp.runtimeconfig.json";
	
	// Initialize .NET Core
	last_error = m_HostFXR_InitializeForRuntimeConfig(
		runtime_config_path.native().c_str(),
		nullptr,
		&host_context_handle
	);

	// 0 (Success), 1 (Success_HostAlreadyInitialized), and 2 (Success_DifferentRuntimeProperties) are success codes
	// https://github.com/dotnet/runtime/blob/main/docs/design/features/host-error-codes.md
	if (last_error < 0 || last_error > 2)
	{
		MessageBoxA(
			0,
			".NET Core failed to initialize.\n"
			"Scripts will not run, and the runtime will be unavailable!",
			"AurieSharp Script Runtime Host",
			MB_OK | MB_ICONERROR | MB_SETFOREGROUND
		);

		return AURIE_MODULE_INITIALIZATION_FAILED;
	}

	last_error = m_HostFXR_GetRuntimeDelegate(
		host_context_handle,
		hdt_load_assembly_and_get_function_pointer,
		(PVOID*)(&m_LoadAssemblyAndGetFunctionPointer)
	);

	assert(last_error == 0 && "Failed to get hdt_load_assembly_and_get_function_pointer");

	last_error = m_HostFXR_GetRuntimeDelegate(
		host_context_handle,
		hdt_get_function_pointer,
		(PVOID*)(&m_GetFunctionPointer)
	);

	assert(last_error == 0 && "Failed to get hdt_get_function_pointer");

	m_HostFXR_Close(host_context_handle);

	// ASM stands for AurieSharpManaged here, not assembly language
	fs::path managed_dll_path = m_ManagedModDirectory / ManagedComponentName;

	PVOID dummy;
	last_error = m_LoadAssemblyAndGetFunctionPointer(
		managed_dll_path.c_str(),
		L"AurieSharpManaged.AurieSharpManaged, AurieSharpManaged",
		L"ModuleInitialize",
		L"Aurie.Managed.AurieEntryDelegate, AurieSharp",
		nullptr,
		&dummy
	);

	if (last_error)
	{
		MessageBoxA(
			0,
			"Failed to initialize AurieSharpManaged.\n"
			"Have you placed it in the Managed folder?",
			"AurieSharp Script Runtime Host",
			MB_OK | MB_ICONERROR | MB_SETFOREGROUND
		);

		return AURIE_MODULE_INITIALIZATION_FAILED;
	}

	return last_error == 0 ? AURIE_SUCCESS : AURIE_EXTERNAL_ERROR;
}

Aurie::AurieStatus RuntimeManager::DispatchManagedModule(
	IN const wchar_t* ManagedComponentName,
	IN const wchar_t* Name
)
{
	AurieStatus(CORECLR_DELEGATE_CALLTYPE * module_entry)() = nullptr;

	DWORD last_status = AURIE_SUCCESS;
	last_status = m_LoadAssemblyAndGetFunctionPointer(
		(m_ManagedModDirectory / ManagedComponentName).c_str(),
		L"AurieSharpManaged.AurieSharpManaged, AurieSharpManaged",
		Name,
		L"Aurie.Managed.AurieEntryDelegate, AurieSharp",
		nullptr,
		reinterpret_cast<void**>(&module_entry)
	);

	// AurieFrameworkDispatch ignores nullptr things
	if (!module_entry)
	{
		// Module doesn't have this routine
		return AURIE_SUCCESS;
	}	

	return module_entry();
}

void RuntimeManager::Uninitialize()
{
	// TODO: wtf do I put here
}