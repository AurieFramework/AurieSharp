#pragma once
#include <YYToolkit/YYTK_Shared.hpp>
#include "../Objects/GameInstance.hpp"

namespace YYTKInterop
{
	using namespace System::Runtime::InteropServices;

	// RValue
	public ref class GameVariable sealed
	{
	internal:
		YYTK::RValue* m_Value;

		static GameVariable^ CreateFromRValue(const YYTK::RValue& Value);

		void InitializeFromRValue(const YYTK::RValue& Value);
		YYTK::RValue& ToRValue();
	public:
		// Creates a VALUE_UNDEFINED RValue
		GameVariable();

		// Creates a VALUE_INT64 RValue
		GameVariable(System::Int32 Value);

		// Creates a VALUE_INT64 RValue
		GameVariable(System::Int64 Value);

		// Create a VALUE_REAL RValue
		GameVariable(double Value);

		// Creates a VALUE_OBJECT RValue
		GameVariable(GameObject^ Value);
		GameVariable(GameInstance^ Value);

		// Creates a VALUE_STRING RValue
		GameVariable(System::String^ Value);

		// Implicit conversions such that C# can do GameVariable var = "string" for example
		static operator GameVariable ^ (System::Int32 Value);
		static operator GameVariable ^ (System::Int64 Value);
		static operator GameVariable ^ (GameObject^ Value);
		static operator GameVariable ^ (GameInstance^ Value);
		static operator GameVariable ^ (System::String^ Value);

		System::Int32 ToInt32();
		System::Int64 ToInt64();
		double ToDouble();
		virtual System::String^ ToString() override;
		GameObject^ ToGameObject();
		GameInstance^ ToGameInstance();
		Gen::IReadOnlyList<GameVariable^>^ ToArrayView();

		static operator double(GameVariable^ Variable);
		static operator System::Int32(GameVariable^ Variable);
		static operator System::Int64(GameVariable^ Variable);
		static operator System::String^(GameVariable^ Variable);
		static operator GameObject^(GameVariable^ Variable);
		static operator GameInstance^ (GameVariable^ Variable);
		static operator Gen::IReadOnlyList<GameVariable^>^ (GameVariable^ Variable);

		bool TryGetInt32([Out] System::Int32% Value);
		bool TryGetInt64([Out] System::Int64% Value);
		bool TryGetDouble([Out] double% Value);
		bool TryGetGameObject([Out] GameObject^% Value);
		bool TryGetGameInstance([Out] GameInstance^% Value);
		bool TryGetString([Out] System::String^% Value);
		bool TryGetArrayView([Out] Gen::IReadOnlyList<GameVariable^>^% Value);

		// Destructor - called only when modders use Dispose() or instance goes out of scope of a "using" block
		~GameVariable();
		// Finalizer - called by GC
		!GameVariable();

		static property GameVariable^ Undefined
		{
			GameVariable ^ get();
		}

		property System::String^ Type
		{
			System::String^ get();
		}

		property GameVariable^ default[System::String^]
		{
			GameVariable^ get(System::String^ Name);
			void set(System::String^ Name, GameVariable^ Value);
		}

		property GameVariable^ default[int]
		{
			GameVariable^ get(int Index);
			void set(int Index, GameVariable^ Value);
		}
	};
}