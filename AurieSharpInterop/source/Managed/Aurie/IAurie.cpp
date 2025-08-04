#include "IAurie.hpp"
#include <msclr/marshal_cppstd.h>
using namespace msclr::interop;

namespace AurieSharpInterop
{
	System::IntPtr Framework::GetInterface(
		[In] System::String^ Name
	)
	{
		std::string interface_name = marshal_as<std::string>(Name);
		
		Aurie::AurieInterfaceBase* interface_ptr = nullptr;
		auto last_status = Aurie::ObGetInterface(interface_name.c_str(), interface_ptr);

		if (!Aurie::AurieSuccess(last_status))
		{
			throw gcnew System::ArgumentException("Attempted to get non-existing interface!", "Name");
		}

		return System::IntPtr(interface_ptr);
	}

	void Framework::GetVersion(
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

	void Debug::Print(
		[In] System::String^ Text
	)
	{
		std::string native_string = marshal_as<std::string>(Text);
		return Aurie::DbgPrint(native_string.c_str());
	}

	void Debug::PrintEx(
		[In] AurieLogSeverity Severity, 
		[In] System::String^ Text
	)
	{
		std::string native_string = marshal_as<std::string>(Text);
		return Aurie::DbgPrintEx(static_cast<Aurie::AurieLogSeverity>(Severity), native_string.c_str());
	}
}