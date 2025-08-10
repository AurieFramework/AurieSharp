#include "Managed/Aurie/IAurie.hpp"
#include "Managed/YYToolkit/Interface/Interface.hpp"
#include <msclr/marshal_cppstd.h>
using namespace msclr::interop;
using namespace Aurie;

// This method has to be here such that our module is 
// guaranteed to load before any C# interop is attempted by AurieSharpCore, 
// which tries doing it in ModulePreinitialize.
EXPORTED AurieStatus ModuleEntrypoint(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(Module);
	UNREFERENCED_PARAMETER(ModulePath);

	return AURIE_SUCCESS;
}

void UnhandledExceptionHandler(System::Object^ Sender, System::UnhandledExceptionEventArgs^ Arguments)
{
	auto ex = safe_cast<System::Exception^>(Arguments->ExceptionObject);
	std::string exception_as_string = marshal_as<std::string>(ex->ToString());

	std::string error_message =
		"The game has crashed due to an unhandled exception in a managed mod.\n\n"
		"This is likely an issue with the mod's code, not the game itself.\n"
		"Crash details have been saved to the game directory.\n\n"
		"Please include the 'Aurie.log' file when reporting the issue to the mod developer.\n\n"
		"The runtime provided the following information about the crash:\n";

	error_message += exception_as_string;

	DbgPrintEx(Aurie::LOG_SEVERITY_CRITICAL, "%s", exception_as_string.c_str());

	MessageBoxA(
		0,
		error_message.c_str(),
		"Aurie Framework Script Host",
		MB_OK | MB_ICONERROR | MB_SETFOREGROUND | MB_TOPMOST
	);
}

EXPORTED AurieStatus ModulePreinitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(Module);
	UNREFERENCED_PARAMETER(ModulePath);

	// Setup unhandled exception filter
	System::AppDomain::CurrentDomain->UnhandledException += gcnew System::UnhandledExceptionEventHandler(&UnhandledExceptionHandler);

	// Now set up YYTK hooks
	YYTK::YYTKInterface* module_interface = YYTK::GetInterface();

	if (!module_interface)
	{
		DbgPrintEx(Aurie::LOG_SEVERITY_CRITICAL, "[ASM] Failed to get YYTK interface in ModulePreinitialize! Is it loaded?");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}

	module_interface->CreateCallback(Aurie::g_ArSelfModule, YYTK::EVENT_OBJECT_CALL, YYTKInterop::NativeObjectCallback, 0);
	module_interface->CreateCallback(Aurie::g_ArSelfModule, YYTK::EVENT_FRAME, YYTKInterop::NativeFrameCallback, 0);

	return AURIE_SUCCESS;
}