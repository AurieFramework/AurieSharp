#include "../IYYToolkit.hpp"

namespace YYTK
{
	namespace Managed
	{
		static void AllocateManagedRValue(YYTK::Managed::RValue^ thisptr)
		{
			// Allocate memory for our unmanaged RValue
			thisptr->m_UnmanagedRValue = reinterpret_cast<YYTK::RValue*>(
				Aurie::MmAllocateMemory(
					Aurie::g_ArSelfModule,
					sizeof(YYTK::RValue)
				)
				);

			if (!thisptr->m_UnmanagedRValue)
			{
				throw gcnew System::OutOfMemoryException("Failed to allocate underlying RValue!");
				return;
			}

			// Initialize the unmanaged RValue
			*thisptr->m_UnmanagedRValue = YYTK::RValue();
		}

		RValue::RValue()
		{
			AllocateManagedRValue(this);
		}

		RValue::RValue(bool Value)
		{
			AllocateManagedRValue(this);
			*this->m_UnmanagedRValue = YYTK::RValue(Value);
		}

		RValue::RValue(int32_t Value)
		{
			AllocateManagedRValue(this);
			*this->m_UnmanagedRValue = YYTK::RValue(Value);
		}

		RValue::RValue(int64_t Value)
		{
			AllocateManagedRValue(this);
			*this->m_UnmanagedRValue = YYTK::RValue(Value);
		}

		RValue::RValue(double Value)
		{
			AllocateManagedRValue(this);
			*this->m_UnmanagedRValue = YYTK::RValue(Value);
		}

		RValue::RValue(System::String^ Value)
		{
			// Strings are pointers to a RefString structure.
			// We can create a dummy RValue on the stack, and copy the pointer.
			System::IntPtr value = Marshal::StringToHGlobalAnsi(Value);

			AllocateManagedRValue(this);
			*this->m_UnmanagedRValue = YYTK::RValue(reinterpret_cast<const char*>(value.ToPointer()));

			// RValue constructor makes a copy of the string, we can safely free ours
			Marshal::FreeHGlobal(value);
		}

		RValue::RValue(YYTK::Managed::CInstance^ Value)
		{
			AllocateManagedRValue(this);
			*this->m_UnmanagedRValue = YYTK::RValue(Value->m_UnmanagedInstance);
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

		bool RValue::IsArray()
		{
			if (!this->m_UnmanagedRValue)
				throw gcnew System::NullReferenceException("Trying to check state of null RValue!");

			return this->m_UnmanagedRValue->m_Kind == VALUE_ARRAY;
		}

		bool RValue::IsStruct()
		{
			if (!this->m_UnmanagedRValue)
				throw gcnew System::NullReferenceException("Trying to check state of null RValue!");

			return this->m_UnmanagedRValue->m_Kind == VALUE_OBJECT;
		}

		System::String^ RValue::AsString()
		{
			if (!this->m_UnmanagedRValue)
				throw gcnew System::NullReferenceException("Trying to convert null RValue to string!");

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

		RValue^ RValue::default::get(System::String^ Key)
		{
			if (!this->IsStruct())
				throw gcnew System::InvalidCastException("Trying to access member variable of non-struct variable!");

			return this->GetMember(Key);
		}

		void RValue::default::set(System::String^ Key, RValue^ Value)
		{
			if (!this->IsStruct())
				throw gcnew System::InvalidCastException("Trying to access member variable of non-struct variable!");

			RValue^ rv = this->GetMember(Key);

			// Copy the RValues
			*rv->m_UnmanagedRValue = *Value->m_UnmanagedRValue;
		}

		RValue^ RValue::default::get(int Key)
		{
			if (!this->IsArray())
				throw gcnew System::InvalidCastException("Trying to index a non-array variable!");

			// Don't copy
			return UnmanagedToManagedRValue(this->m_UnmanagedRValue->at(Key), false);
		}

		void RValue::default::set(int Key, RValue^ Value)
		{
			if (!this->IsArray())
				throw gcnew System::InvalidCastException("Trying to index a non-array variable!");

			RValue^ rv = UnmanagedToManagedRValue(this->m_UnmanagedRValue->at(Key), false);

			// Copy the RValues
			*rv->m_UnmanagedRValue = *Value->m_UnmanagedRValue;
		}

	}
}
