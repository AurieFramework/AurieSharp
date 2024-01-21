#include "IAurie.hpp"
using Marshal = System::Runtime::InteropServices::Marshal;

namespace AurieSharp
{
	AurieStatus AurieInterface::ElIsProcessSuspended(
		OUT bool% Suspended
	)
	{
		printf("ElProcessSuspended called!!!");
		Beep(1000, 100);

		pin_ptr<bool> suspended = &Suspended;
		return static_cast<AurieStatus>(Aurie::ElIsProcessSuspended(*suspended));
	}

	void AurieInterface::MmGetFrameworkVersion(
		OUT short% Major,
		OUT short% Minor,
		OUT short% Patch
	)
	{
		pin_ptr<short> major = &Major;
		pin_ptr<short> minor = &Minor;
		pin_ptr<short> patch = &Patch;

		return Aurie::MmGetFrameworkVersion(
			major,
			minor,
			patch
		);
	}

	System::UIntPtr AurieInterface::MmSigscanModule(
		IN System::String^ ModuleName,
		IN System::String^ Pattern,
		IN System::String^ PatternMask
	)
	{
		// Allocates memory, we have to free it with FreeHGlobal after we're done
		System::IntPtr module_name = Marshal::StringToHGlobalUni(ModuleName);
		System::IntPtr pattern = Marshal::StringToHGlobalUni(Pattern);
		System::IntPtr pattern_mask = Marshal::StringToHGlobalAnsi(PatternMask);

		size_t result = Aurie::MmSigscanModule(
			reinterpret_cast<const wchar_t*>(module_name.ToPointer()),
			reinterpret_cast<const unsigned char*>(pattern.ToPointer()),
			reinterpret_cast<const char*>(pattern.ToPointer())
		);

		// Free all allocated memory
		Marshal::FreeHGlobal(module_name);
		Marshal::FreeHGlobal(pattern);
		Marshal::FreeHGlobal(pattern_mask);

		return System::UIntPtr(result);
	}

	System::UIntPtr AurieInterface::MmSigscanRegion(
		IN System::UIntPtr RegionBase,
		IN int RegionSize,
		IN System::String^ Pattern,
		IN System::String^ PatternMask
	)
	{
		// Allocates memory, we have to free it with FreeHGlobal after we're done
		System::IntPtr pattern = Marshal::StringToHGlobalUni(Pattern);
		System::IntPtr pattern_mask = Marshal::StringToHGlobalAnsi(PatternMask);

		size_t result = Aurie::MmSigscanRegion(
			reinterpret_cast<const unsigned char*>(RegionBase.ToPointer()),
			static_cast<size_t>(RegionSize),
			reinterpret_cast<const unsigned char*>(pattern.ToPointer()),
			reinterpret_cast<const char*>(pattern.ToPointer())
		);

		// Free all allocated memory
		Marshal::FreeHGlobal(pattern);
		Marshal::FreeHGlobal(pattern_mask);

		return System::UIntPtr(result);
	}
}

