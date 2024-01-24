#include "../IYYToolkit.hpp"

namespace YYTK
{
	namespace Managed
	{
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
	}
}
