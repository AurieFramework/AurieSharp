#pragma once
#pragma unmanaged
#include <YYToolkit/YYTK_Shared.hpp>
#pragma managed
#include "../Objects/GameObject.hpp"
#include "../Variable/GameVariable.hpp"
#include "../Objects/GameRoom.hpp"
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

	extern "C" void NativeBuiltinHook(
		OUT YYTK::RValue& Result,
		OPTIONAL IN YYTK::CInstance* Self,
		OPTIONAL IN YYTK::CInstance* Other,
		IN int ArgumentCount,
		OPTIONAL IN YYTK::RValue* Arguments
	);

	extern "C" void NativeScriptHookEntry();
	extern "C" void NativeBuiltinHookEntry();

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

	public ref class BuiltinExecutionContext sealed
	{
	internal:
		YYTK::RValue& m_Result;
		YYTK::YYObjectBase* m_SelfObject;
		YYTK::YYObjectBase* m_OtherObject;
		int m_ArgumentCount;
		YYTK::RValue* m_Arguments;
		bool m_ResultOverridden;
		System::String^ m_Name;

		BuiltinExecutionContext(std::string Name, YYTK::RValue& Result, YYTK::YYObjectBase* Self, YYTK::YYObjectBase* Other, int ArgumentCount, YYTK::RValue* Arguments) :
			m_Name(gcnew System::String(Name.c_str())), m_SelfObject(Self), m_OtherObject(Other),
			m_ArgumentCount(ArgumentCount), m_Arguments(Arguments), m_Result(Result)
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

	public delegate void GameEventCallbackHandler(CodeExecutionContext^ Context);
	public delegate void FrameCallbackHandler(long FrameNumber, double DeltaTime);
	public delegate void BeforeScriptCallbackHandler(ScriptExecutionContext^ Context);
	public delegate void AfterScriptCallbackHandler(ScriptExecutionContext^ Context);
	public delegate void BeforeBuiltinCallbackHandler(BuiltinExecutionContext^ Context);
	public delegate void AfterBuiltinCallbackHandler(BuiltinExecutionContext^ Context);

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

			// GetRoomData
			GameRoom^ GetStaticRoom(
				int RoomId
			);

			// GetRoomData but wrapped
			GameRoom^ GetStaticRoom(
				System::String^ RoomName
			);

			// GetCurrentRoomData
			GameRoom^ GetRunningRoom();
		};

		ref class EventController sealed
		{
		internal:
			using _BeforeScriptDict = Gen::Dictionary<AurieSharpInterop::AurieManagedModule^, Gen::Dictionary<System::String^, BeforeScriptCallbackHandler^>^>;
			using _AfterScriptDict = Gen::Dictionary<AurieSharpInterop::AurieManagedModule^, Gen::Dictionary<System::String^, AfterScriptCallbackHandler^>^>;
			using _BeforeBuiltinDict = Gen::Dictionary<AurieSharpInterop::AurieManagedModule^, Gen::Dictionary<System::String^, BeforeBuiltinCallbackHandler^>^>;
			using _AfterBuiltinDict = Gen::Dictionary<AurieSharpInterop::AurieManagedModule^, Gen::Dictionary<System::String^, AfterBuiltinCallbackHandler^>^>;

			Gen::List<System::String^>^ m_AttachedScripts;
			Gen::List<System::String^>^ m_AttachedBuiltins;

			_BeforeScriptDict^ m_BeforeScriptHandlers;
			_AfterScriptDict^ m_AfterScriptHandlers;
			_BeforeBuiltinDict^ m_BeforeBuiltinHandlers;
			_AfterBuiltinDict^ m_AfterBuiltinHandlers;

			EventController() 
			{
				m_BeforeScriptHandlers = gcnew _BeforeScriptDict(4);
				m_AfterScriptHandlers = gcnew _AfterScriptDict(4);

				m_BeforeBuiltinHandlers = gcnew _BeforeBuiltinDict(4);
				m_AfterBuiltinHandlers = gcnew _AfterBuiltinDict(4);

				m_AttachedScripts = gcnew Gen::List<System::String^>(4);
				m_AttachedBuiltins = gcnew Gen::List<System::String^>(4);
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

			void RaiseBeforeBuiltinEvent(
				IN std::string BuiltinName,
				OUT YYTK::RValue& Result,
				OPTIONAL IN YYTK::CInstance* Self,
				OPTIONAL IN YYTK::CInstance* Other,
				IN int ArgumentCount,
				OPTIONAL IN YYTK::RValue* Arguments,
				OUT bool& Overridden
			);

			void RaiseAfterBuiltinEvent(
				IN std::string BuiltinName,
				OUT YYTK::RValue& Result,
				OPTIONAL IN YYTK::CInstance* Self,
				OPTIONAL IN YYTK::CInstance* Other,
				IN int ArgumentCount,
				OPTIONAL IN YYTK::RValue* Arguments
			);

			Aurie::AurieStatus AttachTargetBuiltinToNBH(
				std::string BuiltinName
			);

			Aurie::AurieStatus DetachTargetBuiltinFromNBH(
				std::string BuiltinName
			);

			// Redirects a target script to NativeScriptHook
			Aurie::AurieStatus AttachTargetScriptToNSH(
				std::string ScriptName
			);

			Aurie::AurieStatus DetachTargetScriptFromNSH(
				std::string ScriptName
			);

			bool GetOrCreateModScopedPreScriptCallbackDict(
				AurieSharpInterop::AurieManagedModule^ Module,
				Gen::Dictionary<System::String^, BeforeScriptCallbackHandler^>^% Scripts
			);

			bool GetOrCreateModScopedPostScriptCallbackDict(
				AurieSharpInterop::AurieManagedModule^ Module,
				Gen::Dictionary<System::String^, AfterScriptCallbackHandler^>^% Scripts
			);

			bool GetOrCreateModScopedPreBuiltinCallbackDict(
				AurieSharpInterop::AurieManagedModule^ Module,
				Gen::Dictionary<System::String^, BeforeBuiltinCallbackHandler^>^% Builtins
			);

			bool GetOrCreateModScopedPostBuiltinCallbackDict(
				AurieSharpInterop::AurieManagedModule^ Module,
				Gen::Dictionary<System::String^, AfterBuiltinCallbackHandler^>^% Builtins
			);

			bool IsScriptHooked(
				System::String^ ScriptName
			);

			bool IsBuiltinHooked(
				System::String^ BuiltinName
			);

			void DetachScriptIfUnused(
				System::String^ ScriptName
			);

			void DetachBuiltinIfUnused(
				System::String^ BuiltinName
			);

		public:
			void RemoveAllBuiltinHooksForMod(
				AurieSharpInterop::AurieManagedModule^ Module
			);

			void RemoveAllScriptHooksForMod(
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

			void AddPreBuiltinNotification(
				AurieSharpInterop::AurieManagedModule^ CurrentModule,
				System::String^ BuiltinName,
				BeforeBuiltinCallbackHandler^ NotifyHandler
			);

			void AddPostBuiltinNotification(
				AurieSharpInterop::AurieManagedModule^ CurrentModule,
				System::String^ BuiltinName,
				AfterBuiltinCallbackHandler^ NotifyHandler
			);

			void RemovePreBuiltinNotification(
				AurieSharpInterop::AurieManagedModule^ CurrentModule,
				System::String^ BuiltinName,
				BeforeBuiltinCallbackHandler^ NotifyHandler
			);

			void RemovePostBuiltinNotification(
				AurieSharpInterop::AurieManagedModule^ CurrentModule,
				System::String^ BuiltinName,
				AfterBuiltinCallbackHandler^ NotifyHandler
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