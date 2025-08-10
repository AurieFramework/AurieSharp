#pragma once
#pragma unmanaged
#include <YYToolkit/YYTK_Shared.hpp>
#pragma managed
#include "../Objects/GameObject.hpp"
#include "../Variable/GameVariable.hpp"
#include "../../Aurie/IAurie.hpp"

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

		GameVariable^ GetResult();
	};
	public ref class CodeExecutionContext sealed
	{
	internal:
		YYTK::YYObjectBase* m_SelfObject;
		YYTK::YYObjectBase* m_OtherObject;
		YYTK::CCode* m_Code;
		int m_ArgumentCount;
		YYTK::RValue* m_Arguments;
		System::String^ m_Name;

		CodeExecutionContext(std::string Name, YYTK::YYObjectBase* Self, YYTK::YYObjectBase* Other, YYTK::CCode* Code, int ArgumentCount, YYTK::RValue* Arguments) :
			m_Name(gcnew System::String(Name.c_str())), m_SelfObject(Self), m_OtherObject(Other),
			m_ArgumentCount(ArgumentCount), m_Arguments(Arguments)
		{

		}

	public:
		property GameObject^ Self
		{
			GameObject ^ get();
		}

		property GameObject^ Other
		{
			GameObject ^ get();
		}

		property Gen::IReadOnlyList<GameVariable^>^ Arguments
		{
			Gen::IReadOnlyList<GameVariable^> ^ get();
		}

		property System::String^ Name
		{
			System::String ^ get();
		}

		void OverrideArgument(int Index, GameVariable^ NewValue);
	};

	public delegate void GameEventCallbackHandler(CodeExecutionContext^ Context);
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
			using _BeforeScriptDict = Gen::Dictionary<AurieSharpInterop::AurieManagedModule^, Gen::Dictionary<System::String^, BeforeScriptCallbackHandler^>^>;
			using _AfterScriptDict = Gen::Dictionary<AurieSharpInterop::AurieManagedModule^, Gen::Dictionary<System::String^, AfterScriptCallbackHandler^>^>;
			
			_BeforeScriptDict^ m_BeforeScriptHandlers;
			_AfterScriptDict^ m_AfterScriptHandlers;

			EventController() 
			{
				m_BeforeScriptHandlers = gcnew _BeforeScriptDict(4);
				m_AfterScriptHandlers = gcnew _AfterScriptDict(4);
			}

			void RaiseObjectEvent(
				std::string Name,
				YYTK::CInstance* Self,
				YYTK::CInstance* Other,
				YYTK::CCode* CodeObject,
				int ArgumentCount,
				YYTK::RValue* Arguments
			);

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

			// Redirects a target script to NativeScriptHook
			Aurie::AurieStatus AttachTargetScriptToNSH(
				std::string ScriptName
			);

			Aurie::AurieStatus DetachTargetScriptFromNSH(
				std::string ScriptName
			);

			bool GetOrCreateModScopedPreCallbackDict(
				AurieSharpInterop::AurieManagedModule^ Module,
				Gen::Dictionary<System::String^, BeforeScriptCallbackHandler^>^% Scripts
			);

			bool GetOrCreateModScopedPostCallbackDict(
				AurieSharpInterop::AurieManagedModule^ Module,
				Gen::Dictionary<System::String^, AfterScriptCallbackHandler^>^% Scripts
			);

			void CheckRemoveUnusedPreScriptHooks(
				System::String^ ScriptName
			);

			void CheckRemoveUnusedPostScriptHooks(
				System::String^ ScriptName
			);

		public:

			void RemoveAllScriptsForMod(
				AurieSharpInterop::AurieManagedModule^ Module
			);

			event GameEventCallbackHandler^ OnGameEvent;
			event FrameCallbackHandler^ OnFrame;

			void AddPreScriptNotification(
				AurieSharpInterop::AurieManagedModule^ CurrentModule,
				System::String^ ScriptName,
				BeforeScriptCallbackHandler^ NotifyHandler
			);

			void AddPostScriptNotification(
				AurieSharpInterop::AurieManagedModule^ CurrentModule,
				System::String^ ScriptName,
				AfterScriptCallbackHandler^ NotifyHandler
			);

			void RemovePreScriptNotification(
				AurieSharpInterop::AurieManagedModule^ CurrentModule,
				System::String^ ScriptName,
				BeforeScriptCallbackHandler^ NotifyHandler
			);

			void RemovePostScriptNotification(
				AurieSharpInterop::AurieManagedModule^ CurrentModule,
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