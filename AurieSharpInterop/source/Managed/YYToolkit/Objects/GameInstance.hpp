#pragma once
#include <YYToolkit/YYTK_Shared.hpp>
#include "GameObject.hpp"

namespace YYTKInterop
{
	using namespace System::Runtime::InteropServices;

	// CInstance
	public ref class GameInstance
	{
	internal:
		YYTK::CInstance* m_Object;

		void ThrowIfInvalidNativeInstance();
		YYTK::CInstance* GetNativeInstance();

		GameInstance(YYTK::CInstance* Instance);
	public:
		static GameInstance^ FromInstanceID(int InstanceID);

		static GameInstance^ FromObject(GameObject^ Object);

		property System::String^ Name
		{
			System::String^ get();
		}

		property double X
		{
			double get();
			void set(double NewX);
		}

		property double Y
		{
			double get();
			void set(double NewX);
		}

		property int ID
		{
			int get();
		}
	};
}