#include "IAurie.hpp"
using Marshal = System::Runtime::InteropServices::Marshal;

namespace Aurie
{
	namespace Managed
	{
		AurieStatus IAurie::ElIsProcessSuspended(
			[Out] bool% Suspended
		)
		{
			pin_ptr<bool> suspended = &Suspended;
			return static_cast<AurieStatus>(Aurie::ElIsProcessSuspended(*suspended));
		}

		void IAurie::MmGetFrameworkVersion(
			[Out] short% Major,
			[Out] short% Minor,
			[Out] short% Patch
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

		System::UIntPtr IAurie::MmSigscanModule(
			[In] System::String^ ModuleName,
			[In] System::String^ Pattern,
			[In] System::String^ PatternMask
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

		System::UIntPtr IAurie::MmSigscanRegion(
			[In] System::UIntPtr RegionBase,
			[In] int RegionSize,
			[In] System::String^ Pattern,
			[In] System::String^ PatternMask
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

		Aurie::AurieInterfaceBase* AurieInterfaceBase::GetInterface()
		{
			return m_Interface;
		}

		bool AurieInterfaceBase::IsValidPointer()
		{
			return m_Interface != nullptr;
		}

		AurieInterfaceBase::AurieInterfaceBase(System::String^ InterfaceName)
		{
			// Get the string object
			System::IntPtr interface_name = Marshal::StringToHGlobalAnsi(InterfaceName);

			// Try to get the interface
			Aurie::AurieInterfaceBase* interface_ptr = nullptr;
			Aurie::AurieStatus last_status = Aurie::ObGetInterface(
				reinterpret_cast<const char*>(interface_name.ToPointer()),
				interface_ptr
			);

			// Free the string
			Marshal::FreeHGlobal(interface_name);

			if (!Aurie::AurieSuccess(last_status))
				throw gcnew System::ArgumentException("Named interface does not exist!", "InterfaceName");

			m_Interface = interface_ptr;
		}

		AurieInterfaceBase::AurieInterfaceBase(System::UIntPtr InterfacePointer)
		{
			m_Interface = reinterpret_cast<Aurie::AurieInterfaceBase*>(
				static_cast<void*>(InterfacePointer)
			);
		}

		void AurieInterfaceBase::QueryVersion(
			[Out] short% Major, 
			[Out] short% Minor, 
			[Out] short% Patch
		)
		{
			if (!this->IsValidPointer())
				throw gcnew System::NullReferenceException("Tried to query version on null interface!");

			pin_ptr<short> major = &Major;
			pin_ptr<short> minor = &Minor;
			pin_ptr<short> patch = &Patch;

			this->m_Interface->QueryVersion(
				*major,
				*minor,
				*patch
			);
		}
	}
}