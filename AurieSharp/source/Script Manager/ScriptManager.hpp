#pragma once
#include <Aurie/shared.hpp>
#include <nethost.h>
#include <hostfxr.h>
#include <coreclr_delegates.h>

class SharpScriptManager
{
private:
    HMODULE m_NetHost                                                               = nullptr;
    HMODULE m_HostFXR																= nullptr;
    hostfxr_initialize_for_runtime_config_fn m_HostFXR_InitializeForRuntimeConfig	= nullptr;
    hostfxr_get_runtime_delegate_fn m_HostFXR_GetRuntimeDelegate					= nullptr;
    hostfxr_close_fn m_HostFXR_Close	                                            = nullptr;
    load_assembly_and_get_function_pointer_fn m_LoadAssemblyAndGetFunctionPointer   = nullptr;
    load_assembly_fn m_LoadAssembly                                                 = nullptr;
    get_function_pointer_fn m_GetFunctionPointer                                    = nullptr;

    Aurie::fs::path m_ManagedModDirectory;
    Aurie::fs::path m_HostFXRDirectory;
public:
    Aurie::AurieStatus Initialize(
        IN const char* ManagedComponentName
    );

    Aurie::AurieStatus DispatchManagedModule(
        IN const wchar_t* MethodName
    );

    void Uninitialize();
};

inline SharpScriptManager g_ScriptManager;