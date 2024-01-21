#include "Native/NETHost/NetHost.hpp"
#include "Managed/IAurie.hpp"
using namespace Aurie;

EXPORTED AurieStatus ModulePreinitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(Module);
	UNREFERENCED_PARAMETER(ModulePath);

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
		L"AurieSharpManaged.dll",
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
		L"AurieSharpManaged.dll",
		L"ModuleUnload"
	);
}