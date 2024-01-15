#include "Script Manager/ScriptManager.hpp"
using namespace Aurie;

EXPORTED AurieStatus ModulePreinitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(Module);
	UNREFERENCED_PARAMETER(ModulePath);

	AurieStatus last_status = AURIE_SUCCESS;
	last_status = g_ScriptManager.Initialize("AurieSharpManaged.dll");;

	// If we failed loading the assembly, no point trying to call it
	if (!AurieSuccess(last_status))
		return last_status;

	// Call ModulePreinitialize in the managed module
	last_status = g_ScriptManager.DispatchManagedModule(
		L"ModulePreinitialize"
	);

	return last_status;
}

EXPORTED AurieStatus ModuleInitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(Module);
	UNREFERENCED_PARAMETER(ModulePath);

	// Call ModuleInitialize in the managed module
	return g_ScriptManager.DispatchManagedModule(
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

	return g_ScriptManager.DispatchManagedModule(
		L"ModuleUnload"
	);
}