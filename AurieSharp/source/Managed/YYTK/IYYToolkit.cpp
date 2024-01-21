#include "IYYToolkit.hpp"

namespace YYTK
{
	namespace Managed
	{
		YYTK::YYTKInterface* IYYToolkit::GetYYTKInterface()
		{
			return dynamic_cast<YYTK::YYTKInterface*>(this->GetInterface());
		}

		IYYToolkit::IYYToolkit()
		{
			// Try to get the interface
			Aurie::AurieInterfaceBase* interface_ptr = nullptr;
			Aurie::AurieStatus last_status = Aurie::ObGetInterface(
				"YYTK_Main",
				reinterpret_cast<Aurie::AurieInterfaceBase*&>(interface_ptr)
			);

			if (!Aurie::AurieSuccess(last_status))
				throw gcnew System::ArgumentException("YYTK Interface does not exist!", "InterfaceName");

			m_Interface = interface_ptr;
		}

		Aurie::Managed::AurieStatus IYYToolkit::GetNamedRoutineIndex(
			[In] System::String^ FunctionName, 
			[Out] int% FunctionIndex
		)
		{
			return Aurie::Managed::AurieStatus();
		}

		Aurie::Managed::AurieStatus IYYToolkit::GetNamedRoutinePointer(
			[In] System::String^ FunctionName,
			[Out] System::UIntPtr% FunctionPointer
		)
		{
			return Aurie::Managed::AurieStatus();
		}

		Aurie::Managed::AurieStatus IYYToolkit::GetGlobalInstance(
			[Out] System::UIntPtr% GlobalInstance
		)
		{
			return Aurie::Managed::AurieStatus();
		}

		RValue^ IYYToolkit::CallBuiltin(
			[In] System::String^ FunctionName,
			[In] List<RValue^>^ Arguments
		)
		{
			// Get the function name from the System::String
			System::IntPtr function_name = Marshal::StringToHGlobalAnsi(FunctionName);

			// Transfer arguments from List<RValue> to std::vector<RValue>
			// Also convert them from managed RValues to unmanaged ones
			std::vector<YYTK::RValue> unmanaged_arguments;
			for each (RValue ^ argument in Arguments)
			{
				unmanaged_arguments.push_back(ManagedToUnmanagedRValue(argument));
			}

			YYTK::RValue unmanaged_return = this->GetYYTKInterface()->CallBuiltin(
				reinterpret_cast<const char*>(function_name.ToPointer()),
				unmanaged_arguments
			);

			// Free the string
			Marshal::FreeHGlobal(function_name);

			return UnmanagedToManagedRValue(unmanaged_return);
		}

		void IYYToolkit::Print(
			[In] System::ConsoleColor Color,
			[In] System::String^ Text
		)
		{
			System::IntPtr text = Marshal::StringToHGlobalAnsi(Text);

			this->GetYYTKInterface()->Print(
				static_cast<YYTK::CmColor>(Color),
				reinterpret_cast<const char*>(text.ToPointer())
			);

			Marshal::FreeHGlobal(text);
		}

		void IYYToolkit::PrintInfo(
			[In] System::String^ Text
		)
		{
			System::IntPtr text = Marshal::StringToHGlobalAnsi(Text);

			this->GetYYTKInterface()->PrintInfo(
				reinterpret_cast<const char*>(text.ToPointer())
			);

			Marshal::FreeHGlobal(text);
		}

		void IYYToolkit::PrintWarning(
			[In] System::String^ Text
		)
		{
			System::IntPtr text = Marshal::StringToHGlobalAnsi(Text);

			this->GetYYTKInterface()->PrintWarning(
				reinterpret_cast<const char*>(text.ToPointer())
			);

			Marshal::FreeHGlobal(text);
		}

		void IYYToolkit::PrintError(
			System::String^ File, 
			int Line, 
			System::String^ Text
		)
		{
			System::IntPtr file = Marshal::StringToHGlobalAnsi(File);
			System::IntPtr text = Marshal::StringToHGlobalAnsi(Text);

			this->GetYYTKInterface()->PrintError(
				reinterpret_cast<const char*>(file.ToPointer()),
				Line,
				reinterpret_cast<const char*>(text.ToPointer())
			);

			Marshal::FreeHGlobal(file);
			Marshal::FreeHGlobal(text);
		}
	}
}

