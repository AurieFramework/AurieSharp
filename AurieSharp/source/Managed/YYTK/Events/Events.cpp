#include "../IYYToolkit.hpp"

namespace YYTK
{
	namespace Managed
	{
		FWCodeEvent::FWCodeEvent(YYTK::FWCodeEvent& Object)
		{
			this->m_NativeEvent = &Object;
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

		FWFrame::FWFrame(YYTK::FWFrame& Object)
		{
			this->m_NativeEvent = &Object;
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