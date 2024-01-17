#include "Native/NETHost/NetHost.hpp"
using namespace Aurie;

EXPORTED AurieStatus ModulePreinitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(Module);
	UNREFERENCED_PARAMETER(ModulePath);

	AurieStatus last_status = AURIE_SUCCESS;

	// Initializes the managed code host, and loads the AurieSharpManaged.dll file from the managed mods
	// Returning an error code means this native module unloads.
	return g_NetRuntime.Initialize("AurieSharpManaged.dll");
}

EXPORTED AurieStatus ModuleInitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(Module);
	UNREFERENCED_PARAMETER(ModulePath);

	// Call ModuleInitialize in the managed module
	return g_NetRuntime.DispatchManagedModule(
		L"ModuleInitialize"
	);
}

EXPORTED AurieStatus ModuleUnload(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(Module);
	UNREFERENCED_PARAMETER(ModulePath);

	// Call ModuleUnload in the managed module
	return g_NetRuntime.DispatchManagedModule(
		L"ModuleUnload"
	);
}