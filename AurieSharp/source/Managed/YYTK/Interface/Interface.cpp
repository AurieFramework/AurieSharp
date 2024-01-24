#include "../IYYToolkit.hpp"

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

			// If we fail getting the interface, throw an exception 
			if (!Aurie::AurieSuccess(last_status))
				throw gcnew System::ArgumentException("YYTK Interface does not exist!", "InterfaceName");

			m_Interface = interface_ptr;
		}

		Aurie::Managed::AurieStatus IYYToolkit::GetNamedRoutineIndex(
			[In] System::String^ FunctionName,
			[Out] int% FunctionIndex
		)
		{
			System::IntPtr function_name = Marshal::StringToHGlobalAnsi(FunctionName);

			int function_index = 0;
			Aurie::AurieStatus last_status = this->GetYYTKInterface()->GetNamedRoutineIndex(
				reinterpret_cast<const char*>(function_name.ToPointer()),
				&function_index
			);

			Marshal::FreeHGlobal(function_name);

			if (Aurie::AurieSuccess(last_status))
				FunctionIndex = function_index;

			return static_cast<Aurie::Managed::AurieStatus>(last_status);
		}

		Aurie::Managed::AurieStatus IYYToolkit::GetNamedRoutinePointer(
			[In] System::String^ FunctionName,
			[Out] System::UIntPtr% FunctionPointer
		)
		{
			// Convert the string into an unmanaged one
			System::IntPtr function_name = Marshal::StringToHGlobalAnsi(FunctionName);

			// Do the heavy lifting through YYTK API
			PVOID object_pointer = nullptr;
			Aurie::AurieStatus last_status = this->GetYYTKInterface()->GetNamedRoutinePointer(
				reinterpret_cast<const char*>(function_name.ToPointer()),
				&object_pointer
			);

			// Free the string regardless of success status
			Marshal::FreeHGlobal(function_name);

			// Throw if we failed to get the routine
			if (!Aurie::AurieSuccess(last_status))
				throw gcnew System::ArgumentException("Function does not exist!", "FunctionName");

			FunctionPointer = System::UIntPtr(object_pointer);

			// Return the status code
			return static_cast<Aurie::Managed::AurieStatus>(last_status);
		}

		Aurie::Managed::AurieStatus IYYToolkit::GetGlobalInstance(
			[Out] CInstance% Instance
		)
		{
			// Try to get the global instance from the unmanaged interface
			YYTK::CInstance* instance_ptr = nullptr;
			Aurie::AurieStatus last_status = this->GetYYTKInterface()->GetGlobalInstance(
				&instance_ptr
			);

			// Assign only if successful so to not create a null instance
			if (Aurie::AurieSuccess(last_status))
			{
				Instance.m_UnmanagedInstance = instance_ptr;
			}

			return static_cast<Aurie::Managed::AurieStatus>(last_status);
		}

		RValue^ IYYToolkit::CallBuiltin(
			[In] System::String^ FunctionName,
			[In] List<RValue^>^ Arguments
		)
		{
			// Get the function name
			System::IntPtr function_name = Marshal::StringToHGlobalAnsi(FunctionName);

			// Transfer arguments from List<RValue> to std::vector<RValue>
			// Also convert them from managed RValues to unmanaged ones
			std::vector<YYTK::RValue> unmanaged_arguments;
			for each (RValue ^ argument in Arguments)
			{
				unmanaged_arguments.push_back(ManagedToUnmanagedRValue(argument));
			}

			// Do the actual work
			YYTK::RValue unmanaged_return = this->GetYYTKInterface()->CallBuiltin(
				reinterpret_cast<const char*>(function_name.ToPointer()),
				unmanaged_arguments
			);

			// Free the string
			Marshal::FreeHGlobal(function_name);

			// Convert the unmanaged return value into a managed one, creating a copy
			return UnmanagedToManagedRValue(unmanaged_return);
		}

		Aurie::Managed::AurieStatus IYYToolkit::CallBuiltinEx(
			[Out] RValue^ Result, 
			[In] System::String^ FunctionName, 
			[In] CInstance^ SelfInstance,
			[In] CInstance^ OtherInstance,
			[In] List<RValue^>^ Arguments
		)
		{
			// Get the function name
			System::IntPtr function_name = Marshal::StringToHGlobalAnsi(FunctionName);

			// Transfer arguments from List<RValue> to std::vector<RValue>
			// Also convert them from managed RValues to unmanaged ones
			std::vector<YYTK::RValue> unmanaged_arguments;
			for each (RValue ^ argument in Arguments)
			{
				unmanaged_arguments.push_back(ManagedToUnmanagedRValue(argument));
			}

			// Do the actual work
			YYTK::RValue temp_buffer;
			Aurie::AurieStatus unmanaged_status = this->GetYYTKInterface()->CallBuiltinEx(
				temp_buffer,
				reinterpret_cast<const char*>(function_name.ToPointer()),
				SelfInstance->m_UnmanagedInstance,
				OtherInstance->m_UnmanagedInstance,
				unmanaged_arguments
			);

			// Free the string
			Marshal::FreeHGlobal(function_name);

			// Convert the unmanaged return value into a managed one, creating a copy
			return static_cast<Aurie::Managed::AurieStatus>(unmanaged_status);
		}

		void IYYToolkit::Print(
			[In] System::ConsoleColor Color,
			[In] System::String^ Text
		)
		{
			// Convert the string into an unmanaged one
			System::IntPtr text = Marshal::StringToHGlobalAnsi(Text);

			// Print the unmanaged one
			this->GetYYTKInterface()->Print(
				static_cast<YYTK::CmColor>(Color),
				reinterpret_cast<const char*>(text.ToPointer())
			);

			// Free the unmanaged one
			Marshal::FreeHGlobal(text);
		}

		void IYYToolkit::PrintInfo(
			[In] System::String^ Text
		)
		{
			// Convert the string into an unmanaged one
			System::IntPtr text = Marshal::StringToHGlobalAnsi(Text);

			// Print the unmanaged one
			this->GetYYTKInterface()->PrintInfo(
				reinterpret_cast<const char*>(text.ToPointer())
			);

			// Free the unmanaged one
			Marshal::FreeHGlobal(text);
		}

		void IYYToolkit::PrintWarning(
			[In] System::String^ Text
		)
		{
			// Convert the string into an unmanaged one
			System::IntPtr text = Marshal::StringToHGlobalAnsi(Text);

			// Print the unmanaged one
			this->GetYYTKInterface()->PrintWarning(
				reinterpret_cast<const char*>(text.ToPointer())
			);

			// Free the unmanaged one
			Marshal::FreeHGlobal(text);
		}

		void IYYToolkit::PrintError(
			[In] System::String^ File,
			[In] int Line,
			[In] System::String^ Text
		)
		{
			// Convert the strings into unmanaged ones
			System::IntPtr file = Marshal::StringToHGlobalAnsi(File);
			System::IntPtr text = Marshal::StringToHGlobalAnsi(Text);

			// Print the error
			this->GetYYTKInterface()->PrintError(
				reinterpret_cast<const char*>(file.ToPointer()),
				Line,
				reinterpret_cast<const char*>(text.ToPointer())
			);

			// Free the unmanaged strings
			Marshal::FreeHGlobal(file);
			Marshal::FreeHGlobal(text);
		}

		Aurie::Managed::AurieStatus IYYToolkit::CreateCodeCallback(
			[In] CodeEventDelegate^ Delegate
		)
		{
			// Convert the delegate into an actual function pointer
			System::IntPtr function_pointer = Marshal::GetFunctionPointerForDelegate(Delegate);

			// Create a callback for it, assigning 0 as the priority
			Aurie::AurieStatus unmanaged_status = this->GetYYTKInterface()->CreateCallback(
				Aurie::g_ArSelfModule,
				YYTK::EVENT_OBJECT_CALL,
				GlobalCodeCallback,
				0
			);

			// Push back to our list of delegates if we succeded (first callback registration),
			// or if we failed with AURIE_OBJECT_ALREADY_EXISTS,
			// which just means GlobalCodeCallback is already listed as a native callback.
			if (AurieSuccess(unmanaged_status) || unmanaged_status == Aurie::AURIE_OBJECT_ALREADY_EXISTS)
			{
				EventManager::m_CodeDelegates->Add(Delegate);

				// Return success even if the native function returned AURIE_OBJECT_ALREADY_EXISTS,
				// in order to not confuse managed mods with an error return code on success.
				return Aurie::Managed::AurieStatus::Success;
			}

			// Just convert the status if we failed in any other way
			return static_cast<Aurie::Managed::AurieStatus>(unmanaged_status);
		}

		Aurie::Managed::AurieStatus IYYToolkit::RemoveCodeCallback(
			[In] CodeEventDelegate^ Delegate
		)
		{
			// Check if the callback even exists, if it does remove it
			// If it doesn't, return an error code
			if (!EventManager::m_CodeDelegates->Remove(Delegate))
				return Aurie::Managed::AurieStatus::ObjectNotFound;

			// If there's still callbacks left in our list, just return now
			if (EventManager::m_CodeDelegates->Count != 0)
				return Aurie::Managed::AurieStatus::Success;

			// No callbacks remaining, meaning we can remove our global callback too
			// for a slight performance improvement.
			Aurie::AurieStatus unmanaged_status = GetYYTKInterface()->RemoveCallback(
				Aurie::g_ArSelfModule,
				GlobalCodeCallback
			);

			return static_cast<Aurie::Managed::AurieStatus>(unmanaged_status);
		}

		Aurie::Managed::AurieStatus IYYToolkit::CreateFrameCallback(
			[In] FrameEventDelegate^ Delegate
		)
		{
			// Convert the delegate into an actual function pointer
			System::IntPtr function_pointer = Marshal::GetFunctionPointerForDelegate(Delegate);

			// Create a callback for it, assigning 0 as the priority
			Aurie::AurieStatus unmanaged_status = this->GetYYTKInterface()->CreateCallback(
				Aurie::g_ArSelfModule,
				YYTK::EVENT_FRAME,
				GlobalFrameCallback,
				0
			);

			// Push back to our list of delegates if we succeded (first callback registration),
			// or if we failed with AURIE_OBJECT_ALREADY_EXISTS,
			// which just means GlobalCodeCallback is already listed as a native callback.
			if (AurieSuccess(unmanaged_status) || unmanaged_status == Aurie::AURIE_OBJECT_ALREADY_EXISTS)
			{
				EventManager::m_FrameDelegates->Add(Delegate);

				// Return success even if the native function returned AURIE_OBJECT_ALREADY_EXISTS,
				// in order to not confuse managed mods with an error return code on success.
				return Aurie::Managed::AurieStatus::Success;
			}

			// Just convert the status if we failed in any other way
			return static_cast<Aurie::Managed::AurieStatus>(unmanaged_status);
		}

		Aurie::Managed::AurieStatus IYYToolkit::RemoveFrameCallback(
			[In] FrameEventDelegate^ Delegate
		)
		{
			// Check if the callback even exists, if it does remove it
			// If it doesn't, return an error code
			if (!EventManager::m_FrameDelegates->Remove(Delegate))
				return Aurie::Managed::AurieStatus::ObjectNotFound;

			// If there's still callbacks left in our list, just return now
			if (EventManager::m_FrameDelegates->Count != 0)
				return Aurie::Managed::AurieStatus::Success;

			// No callbacks remaining, meaning we can remove our global callback too
			// for a slight performance improvement.
			Aurie::AurieStatus unmanaged_status = GetYYTKInterface()->RemoveCallback(
				Aurie::g_ArSelfModule,
				GlobalFrameCallback
			);

			return static_cast<Aurie::Managed::AurieStatus>(unmanaged_status);
		}
	}
}