#pragma once
#include <YYToolkit/YYTK_Shared.hpp>
#include "../Objects/GameObject.hpp"
#include "../Variable/GameVariable.hpp"


namespace YYTKInterop
{
	using namespace ::System::Runtime::InteropServices;
	namespace Gen = ::System::Collections::Generic;

	void NativeObjectCallback(IN YYTK::FWCodeEvent& CodeEvent);
	void NativeFrameCallback(IN YYTK::FWFrame& FrameEvent);
	extern "C" YYTK::RValue& NativeScriptHook(
		IN YYTK::CInstance* Self,
		IN YYTK::CInstance* Other,
		OUT YYTK::RValue& Result,
		IN int ArgumentCount,
		IN YYTK::RValue** Arguments
	);
	extern "C" void NativeScriptHookEntry();

	public ref class ScriptExecutionContext sealed
	{
	internal:
		YYTK::RValue& m_Result;
		YYTK::YYObjectBase* m_SelfObject;
		YYTK::YYObjectBase* m_OtherObject;
		int m_ArgumentCount;
		YYTK::RValue** m_Arguments;
		bool m_ResultOverridden;
		System::String^ m_Name;

		ScriptExecutionContext(std::string Name, YYTK::RValue& Result, YYTK::YYObjectBase* Self, YYTK::YYObjectBase* Other, int ArgumentCount, YYTK::RValue** Arguments) :
			m_Name(gcnew System::String(Name.c_str())), m_Result(Result), m_SelfObject(Self), 
			m_OtherObject(Other), m_ArgumentCount(ArgumentCount), m_Arguments(Arguments), m_ResultOverridden(false)
		{

		}

	public:
		property GameObject^ Self
		{
			GameObject^ get();
		}

		property GameObject^ Other
		{
			GameObject^ get();
		}

		property Gen::IReadOnlyList<GameVariable^>^ Arguments
		{
			Gen::IReadOnlyList<GameVariable^>^ get();
		}

		property System::String^ Name
		{
			System::String^ get();
		}

		void OverrideArgument(int Index, GameVariable^ NewValue);

		void OverrideResult(GameVariable^ NewValue);
	};

	public delegate void GameEventCallbackHandler(GameObject^ Self, GameObject^ Other);
	public delegate void FrameCallbackHandler(long FrameNumber, double DeltaTime);
	public delegate void BeforeScriptCallbackHandler(ScriptExecutionContext^ Context);
	public delegate void AfterScriptCallbackHandler(ScriptExecutionContext^ Context);

	// YYTKInterface
	public ref class Game abstract sealed
	{
	public:
		ref class EngineController sealed
		{
		internal:
			EngineController() {}
		public:
			// CallBuiltin
			GameVariable^ CallFunction(
				System::String^ Name,
				... array<GameVariable^>^ Arguments
			);

			// CallBuiltinEx
			GameVariable^ CallFunctionEx(
				System::String^ Name,
				GameObject^ Self,
				GameObject^ Other,
				... array<GameVariable^>^ Arguments
			);

			// GetGlobalInstance
			GameObject^ GetGlobalObject();

			// CallGameScript
			GameVariable^ CallScript(
				System::String^ Name,
				... array<GameVariable^>^ Arguments
			);

			// CallBuiltinEx
			GameVariable^ CallScriptEx(
				System::String^ Name,
				GameObject^ Self,
				GameObject^ Other,
				... array<GameVariable^>^ Arguments
			);
		};

		ref class EventController sealed
		{
		internal:
			Gen::Dictionary<System::String^, BeforeScriptCallbackHandler^>^ m_BeforeScriptHandlers;
			Gen::Dictionary<System::String^, AfterScriptCallbackHandler^>^ m_AfterScriptHandlers;

			EventController() 
			{
				m_BeforeScriptHandlers = gcnew Gen::Dictionary<System::String^, BeforeScriptCallbackHandler^>(4);
				m_AfterScriptHandlers = gcnew Gen::Dictionary<System::String^, AfterScriptCallbackHandler^>(4);
			}

			void RaiseObjectEvent(YYTK::CInstance* Self, YYTK::CInstance* Other);
			void RaiseFrameEvent(long FrameNumber, double DeltaTime);
			void RaiseBeforeScriptEvent(
				std::string Name, 
				YYTK::RValue& Result, 
				YYTK::CInstance* Self, 
				YYTK::CInstance* Other, 
				int ArgumentCount, 
				YYTK::RValue** Arguments, 
				bool& outOverriden
			);

			void RaiseAfterScriptEvent(
				std::string Name,
				YYTK::RValue& Result,
				YYTK::CInstance* Self, 
				YYTK::CInstance* Other,
				int ArgumentCount,
				YYTK::RValue** Arguments
			);

		public:
			event GameEventCallbackHandler^ OnGameEvent;
			event FrameCallbackHandler^ OnFrame;

			void AddPreScriptNotification(
				System::String^ ScriptName,
				BeforeScriptCallbackHandler^ NotifyHandler
			);

			void AddPostScriptNotification(
				System::String^ ScriptName,
				AfterScriptCallbackHandler^ NotifyHandler
			);

			void RemovePreScriptNotification(
				System::String^ ScriptName,
				BeforeScriptCallbackHandler^ NotifyHandler
			);

			void RemovePostScriptNotification(
				System::String^ ScriptName,
				AfterScriptCallbackHandler^ NotifyHandler
			);
		};

	private:
		static EngineController^ s_Engine = gcnew EngineController();
		static EventController^ s_Events = gcnew EventController();

	public:
		static property EngineController^ Engine
		{
			EngineController^ get() { return s_Engine; }
		}

		static property EventController^ Events
		{
			EventController^ get() { return s_Events; }
		}
	};
}