#pragma once
#include "../Aurie/IAurie.hpp"
#include <YYToolkit/Shared.hpp>

namespace YYTK
{
	namespace Managed
	{
		using namespace System::Runtime::InteropServices;
		using namespace System::Collections::Generic;

		public enum class RValueType : uint32_t
		{
			VALUE_REAL = 0,				// Real value
			VALUE_STRING = 1,			// String value
			VALUE_ARRAY = 2,			// Array value
			VALUE_PTR = 3,				// Ptr value
			VALUE_VEC3 = 4,				// Vec3 (x,y,z) value (within the RValue)
			VALUE_UNDEFINED = 5,		// Undefined value
			VALUE_OBJECT = 6,			// YYObjectBase* value 
			VALUE_INT32 = 7,			// Int32 value
			VALUE_VEC4 = 8,				// Vec4 (x,y,z,w) value (allocated from pool)
			VALUE_VEC44 = 9,			// Vec44 (matrix) value (allocated from pool)
			VALUE_INT64 = 10,			// Int64 value
			VALUE_ACCESSOR = 11,		// Actually an accessor
			VALUE_NULL = 12,			// JS Null
			VALUE_BOOL = 13,			// Bool value
			VALUE_ITERATOR = 14,		// JS For-in Iterator
			VALUE_REF = 15,				// Reference value (uses the ptr to point at a RefBase structure)
			VALUE_UNSET = 0x0ffffff		// Unset value (never initialized)
		};

		public ref class RValue
		{
		public:
			[StructLayout(LayoutKind::Explicit, Pack = 4)]
			value class RValueData
			{
			public:
				[FieldOffset(0)] int32_t m_i32;
				[FieldOffset(0)] int64_t m_i64;
				[FieldOffset(0)] double m_Real;
				[FieldOffset(0)] PVOID m_Pointer;
			} m_Data;

			uint32_t m_Flags;
			RValueType m_Kind;

			RValue()
			{
				this->m_Data.m_i64 = 0;
				this->m_Kind = RValueType::VALUE_UNDEFINED;
				this->m_Flags = 0;
			}

			RValue(bool Value)
			{
				this->m_Data.m_i32 = Value;
				this->m_Flags = 0;
				this->m_Kind = RValueType::VALUE_BOOL;
			}
		};

		public ref class IYYToolkit : public Aurie::Managed::AurieInterfaceBase
		{
		private:
			YYTK::YYTKInterface* GetYYTKInterface();
		public:
			IYYToolkit();

			Aurie::Managed::AurieStatus GetNamedRoutineIndex(
				[In] System::String^ FunctionName,
				[Out] int% FunctionIndex
			);

			Aurie::Managed::AurieStatus GetNamedRoutinePointer(
				[In] System::String^ FunctionName,
				[Out] System::UIntPtr% FunctionPointer
			);

			Aurie::Managed::AurieStatus GetGlobalInstance(
				[Out] System::UIntPtr% FunctionPointer
			);

			RValue^ CallBuiltin(
				[In] System::String^ FunctionName,
				[In] List<RValue^>^ Arguments
			);

			void Print(
				[In] System::ConsoleColor Color,
				[In] System::String^ Text
			);

			void PrintInfo(
				[In] System::String^ Text
			);

			void PrintWarning(
				[In] System::String^ Text
			);

			void PrintError(
				[In] System::String^ File,
				[In] int Line,
				[In] System::String^ Text
			);
		};

		inline YYTK::RValue ManagedToUnmanagedRValue(YYTK::Managed::RValue^ Value)
		{
			YYTK::RValue unmanaged_rvalue;

			// This copies the whole 8 bytes without the IEEE bullshit of doubles
			unmanaged_rvalue.m_i64 = Value->m_Data.m_i64;
			unmanaged_rvalue.m_Kind = static_cast<YYTK::RValueType>(Value->m_Kind);
			unmanaged_rvalue.m_Flags = Value->m_Flags;

			return unmanaged_rvalue;
		}

		inline YYTK::Managed::RValue^ UnmanagedToManagedRValue(YYTK::RValue Value)
		{
			YYTK::Managed::RValue^ managed_rvalue = gcnew RValue();

			// This copies the whole 8 bytes without the IEEE bullshit of doubles
			managed_rvalue->m_Data.m_i64 = Value.m_i64;
			managed_rvalue->m_Kind = static_cast<YYTK::Managed::RValueType>(Value.m_Kind);
			managed_rvalue->m_Flags = Value.m_Flags;

			return managed_rvalue;
		}
	}
}