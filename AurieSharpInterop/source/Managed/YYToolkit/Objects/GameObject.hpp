#pragma once
#include <YYToolkit/YYTK_Shared.hpp>
namespace YYTKInterop
{
	using namespace System::Runtime::InteropServices;

	// YYObjectBase
	public ref class GameObject
	{
	internal:
		YYTK::YYObjectBase* m_Object;
		GameObject(YYTK::YYObjectBase* Object)
		{
			m_Object = Object;
		}
	public:
		bool IsInstance();

		virtual property System::String^ Name
		{
			System::String^ get();
		}
	};
}