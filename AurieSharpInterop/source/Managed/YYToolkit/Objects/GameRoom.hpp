#pragma once
#include <YYToolkit/YYTK_Shared.hpp>
#include "GameInstance.hpp"
namespace YYTKInterop
{
	using namespace System::Runtime::InteropServices;
	namespace Gen = System::Collections::Generic;

	public ref class GameRoom
	{
	internal:
		YYTK::CRoom* m_RoomObject;
		GameRoom(YYTK::CRoom* Room) : m_RoomObject(Room) {}

	public:
		// name property
		// width, height
		// active instances list
		// inactive instances list

		property int Width
		{
			int get();
			void set(int NewWidth);
		}

		property int Height
		{
			int get();
			void set(int NewHeight);
		}

		property Gen::List<GameInstance^>^ ActiveInstances
		{
			Gen::List<GameInstance^>^ get();
		}

		property Gen::List<GameInstance^>^ InactiveInstances
		{
			Gen::List<GameInstance^>^ get();
		}
	};
}