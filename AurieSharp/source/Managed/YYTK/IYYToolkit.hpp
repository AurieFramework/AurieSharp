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

		public enum class EventTriggers : uint32_t
		{
			EVENT_OBJECT_CALL = 1,	// The event represents a Code_Execute() call.
			EVENT_FRAME = 2,		// The event represents an IDXGISwapChain::Present() call.
			EVENT_RESIZE = 3,		// The event represents an IDXGISwapChain::ResizeBuffers() call.
			EVENT_SCRIPT_CALL = 4,	// The event represents a DoCallScript() call.
			EVENT_WNDPROC = 5		// The event represents a WndProc() call.
		};

		public ref class RValue
		{
		public:
			YYTK::RValue* m_UnmanagedRValue = nullptr;

			RValue();

			RValue(bool Value);

			RValue(int32_t Value);

			RValue(int64_t Value);

			RValue(double Value);

			RValue(System::String^ Value);

			~RValue();

			double AsReal();

			int32_t AsInt32();

			int64_t AsInt64();

			bool AsBool();

			System::String^ AsString();

			bool IsUndefined();

			bool IsInstance();

			RValue^ GetMember(System::String^ MemberName);

			RValueType GetKind();
		};

		public ref class CInstance
		{
		public:
			YYTK::CInstance* m_UnmanagedInstance = nullptr;
		
			CInstance(System::UIntPtr InstancePointer);

			RValue^ GetMember(System::String^ MemberName);
		};

		// This one specifically has to be inline or else the compiler crashes with an access violation.
		// I don't know what happens behind that mess, nor do I wanna know.
		public ref class CCode
		{
		public:
			YYTK::CCode* m_UnmanagedCode = nullptr;

			CCode(System::UIntPtr ObjectPointer)
			{
				if (!ObjectPointer.ToPointer())
					throw gcnew System::NullReferenceException("Constructing null code object!");

				this->m_UnmanagedCode = reinterpret_cast<YYTK::CCode*>(ObjectPointer.ToPointer());
			}

			System::String^ GetName()
			{
				return gcnew System::String(m_UnmanagedCode->GetName());
			}

			int GetIndex()
			{
				return m_UnmanagedCode->m_CodeIndex;
			}
		};

		// Wraps FWCodeEvent&
		public ref class FWCodeEvent
		{
		private:
			YYTK::FWCodeEvent* m_NativeEvent = nullptr;
		public:
			FWCodeEvent(YYTK::FWCodeEvent& Object);

			bool CalledOriginal();

			bool Call();

			CInstance^ GetSelfInstance();

			CInstance^ GetOtherInstance();

			CCode^ GetCodeObject();

			int GetArgumentCount();

			List<RValue^>^ GetArgumentArray();
		};

		public ref class FWFrame
		{
		private:
			YYTK::FWFrame* m_NativeEvent = nullptr;
		public:
			FWFrame(YYTK::FWFrame& Object);

			bool CalledOriginal();

			HRESULT Call();

			System::UIntPtr GetSwapchainPointer();

			uint32_t GetSyncInterval();

			uint32_t GetFlags();
		};

		public delegate void CodeEventDelegate(FWCodeEvent^ Event);
		public delegate void FrameEventDelegate(FWFrame^ Event);

		// ChatGPT code lmao
		private ref class EventManager {
		public:
			// Using gcroot to create a handle to a managed object
			static List<CodeEventDelegate^>^ m_CodeDelegates = gcnew List<CodeEventDelegate^>(8);
			static List<FrameEventDelegate^>^ m_FrameDelegates = gcnew List<FrameEventDelegate^>(8);
		};

		inline void GlobalCodeCallback(YYTK::FWCodeEvent& FunctionContext)
		{
			// Call each code delegate function with a preconstructed FWCodeEvent
			FWCodeEvent^ event_context = gcnew YYTK::Managed::FWCodeEvent(FunctionContext);
			for each (auto function in EventManager::m_CodeDelegates)
				function->Invoke(event_context);
		}

		inline void GlobalFrameCallback(YYTK::FWFrame& FunctionContext)
		{
			// Call each code delegate function with a preconstructed FWCodeEvent
			FWFrame^ event_context = gcnew YYTK::Managed::FWFrame(FunctionContext);
			for each (auto function in EventManager::m_FrameDelegates)
				function->Invoke(event_context);
		}

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
				[Out] CInstance% Instance
			);

			RValue^ CallBuiltin(
				[In] System::String^ FunctionName,
				[In] List<RValue^>^ Arguments
			);

			Aurie::Managed::AurieStatus CallBuiltinEx(
				[Out] RValue^ Result,
				[In] System::String^ FunctionName,
				[In] CInstance^ SelfInstance,
				[In] CInstance^ OtherInstance,
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

			Aurie::Managed::AurieStatus CreateCodeCallback(
				[In] CodeEventDelegate^ Delegate
			);

			Aurie::Managed::AurieStatus RemoveCodeCallback(
				[In] CodeEventDelegate^ Delegate
			);

			Aurie::Managed::AurieStatus CreateFrameCallback(
				[In] FrameEventDelegate^ Delegate
			);

			Aurie::Managed::AurieStatus RemoveFrameCallback(
				[In] FrameEventDelegate^ Delegate
			);
		};

		inline YYTK::RValue ManagedToUnmanagedRValue(YYTK::Managed::RValue^ Value)
		{
			return *Value->m_UnmanagedRValue;
		}

		inline YYTK::Managed::RValue^ UnmanagedToManagedRValue(YYTK::RValue Value)	
		{
			// Instantiate the new RValue
			YYTK::Managed::RValue^ managed_rvalue = gcnew RValue();

			*managed_rvalue->m_UnmanagedRValue = Value;

			return managed_rvalue;
		}

		inline YYTK::Managed::RValue^ UnmanagedToManagedRValue(YYTK::RValue& Value, bool Copy)
		{
			// Instantiate the new RValue
			YYTK::Managed::RValue^ managed_rvalue = gcnew RValue();

			if (Copy)
			{
				*managed_rvalue->m_UnmanagedRValue = Value;
			}
			else
			{
				managed_rvalue->m_UnmanagedRValue = &Value;
			}

			return managed_rvalue;
		}
	}
}