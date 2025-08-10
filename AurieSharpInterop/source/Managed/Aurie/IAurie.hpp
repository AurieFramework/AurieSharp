#pragma once
#pragma unmanaged
#include <Aurie/shared.hpp>
#pragma managed
#pragma warning(disable : 4679)

// A hack to prevent C++/CLI from complaining about missing type references (annoying warnings that cannot be disabled)
namespace Aurie
{
	struct AurieModule {};
};

namespace AurieSharpInterop
{
	using namespace System::Runtime::InteropServices;

	// A status code as returned by low-level Aurie code.
	public enum class AurieStatus : int32_t
	{
		// The operation completed successfully.
		Success = 0,
		// An invalid architecture was specified.
		InvalidArchitecture,
		// An error occured in an external function call.
		ExternalError,
		// The requested file was not found.
		FileNotFound,
		// The requested access to the object was denied.
		AccessDenied,
		// An object with the same identifier / priority is already registered.
		ObjectAlreadyExists,
		// One or more parameters were invalid.
		InvalidParameter,
		// Insufficient memory is available.
		InsufficientMemory,
		// An invalid signature was detected.
		InvalidSignature,
		// The requested operation is not implemented.
		NotImplemented,
		// An internal error occured in the module.
		ModuleInternalError,
		// The module failed to resolve dependencies.
		ModuleDependencyNotResolved,
		// The module failed to initialize.
		ModuleInitializationFailed,
		// The target file header, directory, or RVA could not be found or is invalid.
		FilePartNotFound,
		// The object was not found.
		ObjectNotFound,
		// The requested resource is unavailable.
		Unavailable,
		// The verification failed.
		VerificationFailure,
		// A generic error has occurred.
		UnknownError,
	};
	
	// Log severity. Used in the Debug.Print* function family.
	public enum class AurieLogSeverity
	{
		Trace = 0,
		Debug,
		Info,
		Warning,
		Error,
		Critical
	};

	// Contains methods for interacting directly with the low-level Aurie Framework.
	// Do not touch, unless you know what you're doing!
	public ref class Framework abstract sealed
	{
	public:
		static System::String^ GetGameProcessPath();

		static System::String^ GetGameDirectory();

		static System::IntPtr GetInterface(
			[In] System::String^ Name
		);

		static void GetVersion(
			[Out] short% Major,
			[Out] short% Minor,
			[Out] short% Patch
		);
	};

	// Contains methods for logging data into Aurie.log and the console.
	public ref class Debug abstract sealed
	{
	public:
		static void Print(
			[In] System::String^ Text
		);

		static void PrintEx(
			[In] AurieLogSeverity Severity,
			[In] System::String^ Text
		);
	};	

	public ref class AurieManagedModule sealed
	{
	internal:
		static int m_NextFreeModId = 0;
		int m_ModId;

	public:
		AurieManagedModule()
		{
			this->m_ModId = m_NextFreeModId;
			m_NextFreeModId++;
		}
	};
}