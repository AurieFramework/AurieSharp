#pragma once
#include <Aurie/shared.hpp>
#pragma warning(disable : 4679)

namespace AurieSharp
{
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
		Unavailable
	};

	public ref class AurieInterface
	{
	public:
		static AurieStatus ElIsProcessSuspended(
			OUT bool% Suspended
		);

		static void MmGetFrameworkVersion(
			OUT short% Major,
			OUT short% Minor,
			OUT short% Patch
		);

		static System::UIntPtr MmSigscanModule(
			IN System::String^ ModuleName,
			IN System::String^ Pattern,
			IN System::String^ PatternMask
		);

		static System::UIntPtr MmSigscanRegion(
			IN System::UIntPtr RegionBase,
			IN int RegionSize,
			IN System::String^ Pattern,
			IN System::String^ PatternMask
		);
	};
}