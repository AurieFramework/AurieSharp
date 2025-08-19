#include "IAurie.hpp"
#include <msclr/marshal_cppstd.h>
using namespace msclr::interop;

namespace AurieSharpInterop
{
	System::String^ Framework::GetGameProcessPath()
	{
		auto path = Aurie::Internal::MdpGetImagePath(Aurie::g_ArInitialImage);
		return gcnew System::String(path.string().c_str());
	}

	System::String^ Framework::GetGameDirectory()
	{
		Aurie::fs::path path;
		Aurie::AurieStatus last_status = Aurie::Internal::MdpGetImageFolder(Aurie::g_ArInitialImage, path);

		if (!Aurie::AurieSuccess(last_status))
			throw gcnew System::InvalidOperationException("Failed to get game folder!");

		return gcnew System::String(path.string().c_str());
	}

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

	void Framework::GetAurieVersion(
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

	void Framework::GetInteropVersion(short% Major, short% Minor, short% Patch)
	{
		Major = 1;
		Minor = 0;
		Patch = 0;
	}

	void Framework::Print(
		[In] System::String^ Text
	)
	{
		std::string native_string = marshal_as<std::string>(Text);
		return Aurie::DbgPrint(native_string.c_str());
	}
	
	void Framework::PrintEx(
		[In] AurieLogSeverity Severity, 
		[In] System::String^ Text
	)
	{
		std::string native_string = marshal_as<std::string>(Text);
		return Aurie::DbgPrintEx(static_cast<Aurie::AurieLogSeverity>(Severity), native_string.c_str());
	}

	AurieStatus Framework::LoadNativeModule(System::String^ ModulePath)
	{
		std::string native_path = marshal_as<std::string>(ModulePath);
		Aurie::AurieModule* module = nullptr;

		return static_cast<AurieStatus>(Aurie::MdMapImage(native_path, module));
	}
}