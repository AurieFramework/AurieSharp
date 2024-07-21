#include "../IYYToolkit.hpp"

namespace YYTK
{
	namespace Managed
	{
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

		RValue^ CInstance::default::get(System::String^ Key)
		{
			return this->GetMember(Key);
		}

		void CInstance::default::set(System::String^ Key, RValue^ Value)
		{
			RValue^ rv = this->GetMember(Key);

			// Copy the RValues
			*rv->m_UnmanagedRValue = *Value->m_UnmanagedRValue;
		}
	}
}