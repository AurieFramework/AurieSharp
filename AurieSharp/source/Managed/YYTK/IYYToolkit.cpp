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

		Aurie::Managed::AurieStatus IYYToolkit::CreateCallback(
			[In] EventTriggers Trigger,
			[In] CallbackDelegate^ Delegate,
			[In] int32_t Priority
		)
		{
			// Convert the delegate into an actual function pointer
			System::IntPtr function_pointer = Marshal::GetFunctionPointerForDelegate(Delegate);
			
			// Create a callback for it, converting the EventTriggers enum and return status in the process
			return static_cast<Aurie::Managed::AurieStatus>(
				this->GetYYTKInterface()->CreateCallback(
					Aurie::g_ArSelfModule,
					static_cast<YYTK::EventTriggers>(Trigger),
					function_pointer.ToPointer(),
					Priority
				)
			);
		}

		RValue::RValue()
		{
			// Allocate memory for our unmanaged RValue
			this->m_UnmanagedRValue = reinterpret_cast<YYTK::RValue*>(
				Aurie::MmAllocateMemory(
					Aurie::g_ArSelfModule,
					sizeof(YYTK::RValue)
				)
			);

			if (!this->m_UnmanagedRValue)
			{
				throw gcnew System::OutOfMemoryException("Failed to allocate underlying RValue!");
				return;
			}

			// Initialize the unmanaged RValue
			*this->m_UnmanagedRValue = YYTK::RValue();
		}

		RValue::RValue(bool Value)
		{
			RValue();
			*this->m_UnmanagedRValue = YYTK::RValue(Value);
		}

		RValue::RValue(int32_t Value)
		{
			RValue();
			*this->m_UnmanagedRValue = YYTK::RValue(Value);
		}

		RValue::RValue(int64_t Value)
		{
			RValue();
			*this->m_UnmanagedRValue = YYTK::RValue(Value);
		}

		RValue::RValue(double Value)
		{
			RValue();
			*this->m_UnmanagedRValue = YYTK::RValue(Value);
		}

		RValue::RValue(System::String^ Value)
		{
			// Strings are pointers to a RefString structure.
			// We can create a dummy RValue on the stack, and copy the pointer.
			System::IntPtr value = Marshal::StringToHGlobalAnsi(Value);

			RValue();
			*this->m_UnmanagedRValue = YYTK::RValue(reinterpret_cast<const char*>(value.ToPointer()));

			// RValue constructor makes a copy of the string, we can safely free ours
			Marshal::FreeHGlobal(value);
		}

		RValue::~RValue()
		{
			Aurie::MmFreeMemory(Aurie::g_ArSelfModule, m_UnmanagedRValue);
		}

		double RValue::AsReal()
		{
			if (!this->m_UnmanagedRValue)
				throw gcnew System::NullReferenceException("Trying to convert null RValue to real value!");

			return this->m_UnmanagedRValue->AsReal();
		}

		int32_t RValue::AsInt32()
		{
			if (!this->m_UnmanagedRValue)
				throw gcnew System::NullReferenceException("Trying to convert null RValue to Int32!");

			return static_cast<int32_t>(this->m_UnmanagedRValue->AsReal());
		}

		int64_t RValue::AsInt64()
		{
			if (!this->m_UnmanagedRValue)
				throw gcnew System::NullReferenceException("Trying to convert null RValue to Int64!");

			return static_cast<int64_t>(this->m_UnmanagedRValue->AsReal());
		}

		bool RValue::AsBool()
		{
			if (!this->m_UnmanagedRValue)
				throw gcnew System::NullReferenceException("Trying to convert null RValue to bool!");

			return this->m_UnmanagedRValue->AsBool();
		}

		System::String^ RValue::AsString()
		{
			if (!this->m_UnmanagedRValue)
				throw gcnew System::NullReferenceException("Trying to convert null RValue to bool!");

			return gcnew System::String(this->m_UnmanagedRValue->AsString().data());
		}

		bool RValue::IsUndefined()
		{
			if (!this->m_UnmanagedRValue)
				throw gcnew System::NullReferenceException("Trying to check state of null RValue!");

			return this->m_UnmanagedRValue->m_Kind == VALUE_UNDEFINED || this->m_UnmanagedRValue->m_Kind == VALUE_UNSET;
		}

		bool RValue::IsInstance()
		{
			if (!this->m_UnmanagedRValue)
				throw gcnew System::NullReferenceException("Trying to check state of null RValue!");

			return this->m_UnmanagedRValue->m_Kind == VALUE_OBJECT && this->m_UnmanagedRValue->m_Object;
		}

		RValue^ RValue::GetMember(System::String^ MemberName)
		{
			if (!this->IsInstance())
				throw gcnew System::InvalidOperationException("Trying to access members of non-instance RValue!");

			System::IntPtr member_name = Marshal::StringToHGlobalAnsi(MemberName);

			YYTK::RValue& object = this->m_UnmanagedRValue->at(reinterpret_cast<const char*>(member_name.ToPointer()));

			Marshal::FreeHGlobal(member_name);

			// This directly references the unmanaged RValue, we don't copy in this case!
			return UnmanagedToManagedRValue(object, false);
		}

		RValueType RValue::GetKind()
		{
			if (!this->m_UnmanagedRValue)
				throw gcnew System::NullReferenceException("Trying to access kind of null RValue!");

			return static_cast<RValueType>(this->m_UnmanagedRValue->m_Kind);
		}

		CInstance::CInstance(System::UIntPtr InstancePointer)
		{
			if (!InstancePointer.ToPointer())
				throw gcnew System::NullReferenceException("Constructing null instance!");

			this->m_UnmanagedInstance = reinterpret_cast<YYTK::CInstance*>(InstancePointer.ToPointer());
		}

		RValue^ CInstance::GetMember(System::String^ MemberName)
		{
			System::IntPtr member_name = Marshal::StringToHGlobalAnsi(MemberName);

			// We don't copy the RValue, instead store a pointer to the real one
			RValue^ result_value = UnmanagedToManagedRValue(
				this->m_UnmanagedInstance->at(
					reinterpret_cast<const char*>(member_name.ToPointer())
				),
				false
			);

			Marshal::FreeHGlobal(member_name);

			return result_value;
		}

		CCode::CCode(System::UIntPtr ObjectPointer)
		{
			if (!ObjectPointer.ToPointer())
				throw gcnew System::NullReferenceException("Constructing null code object!");

			this->m_UnmanagedCode = reinterpret_cast<YYTK::CCode*>(ObjectPointer.ToPointer());
		}

		System::String^ CCode::GetName()
		{
			return gcnew System::String(m_UnmanagedCode->GetName());
		}

		FWCodeEvent::FWCodeEvent(System::UIntPtr Object)
		{
			if (!Object.ToPointer())
				throw gcnew System::NullReferenceException("Cannot instantiate null code event!");

			this->m_NativeEvent = reinterpret_cast<YYTK::FWCodeEvent*>(Object.ToPointer());
		}

		bool FWCodeEvent::CalledOriginal()
		{
			return m_NativeEvent->CalledOriginal();
		}

		bool FWCodeEvent::Call()
		{
			return m_NativeEvent->Call();
		}

		CInstance^ FWCodeEvent::GetSelfInstance()
		{
			return gcnew CInstance(System::UIntPtr(std::get<0>(this->m_NativeEvent->Arguments())));
		}

		CInstance^ FWCodeEvent::GetOtherInstance()
		{
			return gcnew CInstance(System::UIntPtr(std::get<1>(this->m_NativeEvent->Arguments())));
		}

		CCode^ FWCodeEvent::GetCodeObject()
		{
			return gcnew CCode(System::UIntPtr(std::get<2>(this->m_NativeEvent->Arguments())));
		}

		int FWCodeEvent::GetArgumentCount()
		{
			return std::get<3>(this->m_NativeEvent->Arguments());
		}

		List<RValue^>^ FWCodeEvent::GetArgumentArray()
		{
			// Create a new list
			List<RValue^>^ argument_array = gcnew List<RValue^>(this->GetArgumentCount());

			// Push all elements of the array into the list
			for (int i = 0; i < this->GetArgumentCount(); i++)
			{
				argument_array->Add(UnmanagedToManagedRValue(std::get<4>(this->m_NativeEvent->Arguments())[i]));
			}

			// Return the list
			return argument_array;
		}

		FWFrame::FWFrame(System::UIntPtr Object)
		{
			if (!Object.ToPointer())
				throw gcnew System::NullReferenceException("Cannot instantiate null frame event!");

			this->m_NativeEvent = reinterpret_cast<YYTK::FWFrame*>(Object.ToPointer());
		}

		bool FWFrame::CalledOriginal()
		{
			return m_NativeEvent->CalledOriginal();
		}

		HRESULT FWFrame::Call()
		{
			return this->m_NativeEvent->Call();
		}

		System::UIntPtr FWFrame::GetSwapchainPointer()
		{
			return System::UIntPtr(std::get<0>(this->m_NativeEvent->Arguments()));
		}

		uint32_t FWFrame::GetSyncInterval()
		{
			return std::get<1>(this->m_NativeEvent->Arguments());
		}

		uint32_t FWFrame::GetFlags()
		{
			return std::get<2>(this->m_NativeEvent->Arguments());
		}
	}
}

