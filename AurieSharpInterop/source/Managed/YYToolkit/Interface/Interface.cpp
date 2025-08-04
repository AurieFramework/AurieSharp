#include "Interface.hpp"
#include <msclr/marshal_cppstd.h>
#include <vector>
#include <intrin.h>
#include <MemoryUtils/MemoryUtils.hpp>
using namespace msclr::interop;
using namespace System;

namespace YYTKInterop
{
	void NativeObjectCallback(
		IN YYTK::FWCodeEvent& CodeEvent
	)
	{
		// Unpack native arguments
		auto& [self, other, code, argc, argv] = CodeEvent.Arguments();

		Game::Events->RaiseObjectEvent(self, other);
	}

	void NativeFrameCallback(
		IN YYTK::FWFrame& FrameEvent
	)
	{
		static long frame_count = 0;
		YYTK::RValue delta_time;
		YYTK::GetInterface()->GetBuiltin("delta_time", nullptr, NULL_INDEX, delta_time);

		Game::Events->RaiseFrameEvent(frame_count, delta_time.ToDouble() / 1000.0);

		frame_count++;
	}

	GameVariable^ Game::EngineController::CallFunction(
		String^ Name,
		... array<GameVariable^>^ Arguments
	)
	{
		Aurie::AurieStatus last_status = Aurie::AURIE_SUCCESS;
		YYTK::CInstance* global_instance = nullptr;

		last_status = YYTK::GetInterface()->GetGlobalInstance(&global_instance);

		if (!Aurie::AurieSuccess(last_status))
			throw gcnew InvalidOperationException("Failed to acquire global instance!");

		return CallFunctionEx(
			Name,
			gcnew GameObject(global_instance),
			gcnew GameObject(global_instance),
			Arguments
		);
	}

	GameVariable^ Game::EngineController::CallFunctionEx(
		System::String^ Name,
		GameObject^ Self,
		GameObject^ Other,
		...array<GameVariable^>^ Arguments
	)
	{
		std::string function_name = marshal_as<std::string>(Name);
		std::vector<YYTK::RValue> function_arguments;

		for each (GameVariable ^ argument in Arguments)
			function_arguments.push_back(argument->ToRValue());

		Aurie::AurieStatus last_status = Aurie::AURIE_SUCCESS;

		// Note: The global context is not an instance. Trying to construct a 
		// GameInstance object from it will cause a crash with InvalidCastException.
		//
		// This is a fault of C++ YYTK accepting a type it shouldn't.
		YYTK::RValue result;
		last_status = YYTK::GetInterface()->CallBuiltinEx(
			result,
			function_name.c_str(),
			reinterpret_cast<YYTK::CInstance*>(Self->m_Object),
			reinterpret_cast<YYTK::CInstance*>(Self->m_Object),
			function_arguments
		);

		if (!Aurie::AurieSuccess(last_status))
			throw gcnew InvalidOperationException("Failed to call built-in function!");

		return GameVariable::CreateFromRValue(result);
	}

	GameObject^ Game::EngineController::GetGlobalObject()
	{
		Aurie::AurieStatus last_status = Aurie::AURIE_SUCCESS;
		YYTK::CInstance* global_instance = nullptr;

		last_status = YYTK::GetInterface()->GetGlobalInstance(&global_instance);

		if (!Aurie::AurieSuccess(last_status))
			throw gcnew InvalidOperationException("Failed to acquire global instance!");

		return gcnew GameObject(global_instance);
	}

	GameVariable^ Game::EngineController::CallScript(System::String^ Name, ...array<GameVariable^>^ Arguments)
	{
		Aurie::AurieStatus last_status = Aurie::AURIE_SUCCESS;
		YYTK::CInstance* global_instance = nullptr;

		last_status = YYTK::GetInterface()->GetGlobalInstance(&global_instance);

		if (!Aurie::AurieSuccess(last_status))
			throw gcnew InvalidOperationException("Failed to acquire global instance!");

		return CallScriptEx(
			Name,
			gcnew GameObject(global_instance),
			gcnew GameObject(global_instance),
			Arguments
		);
	}

	GameVariable^ Game::EngineController::CallScriptEx(
		System::String^ Name, 
		GameObject^ Self, 
		GameObject^ Other, 
		...array<GameVariable^>^ Arguments
	)
	{
		std::string function_name = marshal_as<std::string>(Name);
		std::vector<YYTK::RValue> function_arguments;

		for each (GameVariable ^ argument in Arguments)
			function_arguments.push_back(argument->ToRValue());

		Aurie::AurieStatus last_status = Aurie::AURIE_SUCCESS;

		// Note: The global context is not an instance. Trying to construct a 
		// GameInstance object from it will cause a crash with InvalidCastException.
		//
		// This is a fault of C++ YYTK accepting a type it shouldn't.
		YYTK::RValue result;
		last_status = YYTK::GetInterface()->CallGameScriptEx(
			result,
			function_name.c_str(),
			reinterpret_cast<YYTK::CInstance*>(Self->m_Object),
			reinterpret_cast<YYTK::CInstance*>(Self->m_Object),
			function_arguments
		);

		if (!Aurie::AurieSuccess(last_status))
			throw gcnew InvalidOperationException("Failed to call built-in function!");

		return GameVariable::CreateFromRValue(result);
	}

	void Game::EventController::RaiseObjectEvent(
		YYTK::CInstance* Self, 
		YYTK::CInstance* Other
	)
	{
		// Some events (eg. GlobalScripts) are not called on real instances.
		// Constructing a GameInstance from that will throw.
		// This is a fault of C++ YYTK using a type it shouldn't.
		OnGameEvent(gcnew GameObject(Self), gcnew GameObject(Other));
	}

	void Game::EventController::RaiseFrameEvent(
		long FrameNumber,
		double DeltaTime
	)
	{
		OnFrame(FrameNumber, DeltaTime);
	}

	void Game::EventController::RaiseBeforeScriptEvent(
		std::string Name, 
		YYTK::RValue& Result, 
		YYTK::CInstance* Self, 
		YYTK::CInstance* Other,
		int ArgumentCount,
		YYTK::RValue** Arguments,
		bool& outOverridden
	)
	{
		ScriptExecutionContext^ context = gcnew ScriptExecutionContext(
			Name,
			Result,
			Self,
			Other,
			ArgumentCount,
			Arguments
		);

		BeforeScriptCallbackHandler^ value = nullptr;
		if (m_BeforeScriptHandlers->TryGetValue(gcnew System::String(Name.c_str()), value))
			value->Invoke(context);

		outOverridden = context->m_ResultOverridden;
	}

	void Game::EventController::RaiseAfterScriptEvent(
		std::string Name, 
		YYTK::RValue& Result, 
		YYTK::CInstance* Self,
		YYTK::CInstance* Other, 
		int ArgumentCount,
		YYTK::RValue** Arguments
	)
	{
		ScriptExecutionContext^ context = gcnew ScriptExecutionContext(
			Name,
			Result,
			Self,
			Other,
			ArgumentCount,
			Arguments
		);

		AfterScriptCallbackHandler^ value = nullptr;
		if (m_AfterScriptHandlers->TryGetValue(gcnew System::String(Name.c_str()), value))
			value->Invoke(context);
	}
	
	void Game::EventController::AddPreScriptNotification(
		System::String^ ScriptName,
		BeforeScriptCallbackHandler^ NotifyHandler
	)
	{
		std::string script_name = marshal_as<std::string>(ScriptName);
		if (!Aurie::AurieSuccess(Aurie::MmHookExists(Aurie::g_ArSelfModule, script_name.c_str())))
		{
			YYTK::CScript* script_object = nullptr;
			Aurie::AurieStatus last_status = Aurie::AURIE_SUCCESS;

			int function_index = 0;
			last_status = YYTK::GetInterface()->GetNamedRoutineIndex(
				script_name.c_str(),
				&function_index
			);
			
			// Function indices < 100'000 are built-in functions, not scripts.
			// Above 500'000, there's extension functions - also not scripts.
			if (!Aurie::AurieSuccess(last_status) || function_index < 100'000 || function_index > 500'000)
				throw gcnew System::InvalidOperationException("Script does not exist!");

			// Get the CScript object.
			last_status = YYTK::GetInterface()->GetNamedRoutinePointer(
				script_name.c_str(),
				reinterpret_cast<PVOID*>(&script_object)
			);

			if (!Aurie::AurieSuccess(last_status))
				throw gcnew System::InvalidOperationException("Script does not exist!");

			last_status = Aurie::MmCreateHook(
				Aurie::g_ArSelfModule,
				script_name,
				script_object->m_Functions->m_ScriptFunction,
				NativeScriptHookEntry,
				nullptr
			);

			if (!Aurie::AurieSuccess(last_status))
				throw gcnew System::InvalidOperationException("Failed to hook script!");
		}

		if (m_BeforeScriptHandlers->ContainsKey(ScriptName))
		{
			m_BeforeScriptHandlers[ScriptName] = static_cast<BeforeScriptCallbackHandler^>(Delegate::Combine(m_BeforeScriptHandlers[ScriptName], NotifyHandler));
			return;
		}

		return m_BeforeScriptHandlers->Add(ScriptName, NotifyHandler);
	}

	void Game::EventController::AddPostScriptNotification(
		System::String^ ScriptName, 
		AfterScriptCallbackHandler^ NotifyHandler
	)
	{
		std::string script_name = marshal_as<std::string>(ScriptName);
		if (!Aurie::AurieSuccess(Aurie::MmHookExists(Aurie::g_ArSelfModule, script_name.c_str())))
		{
			YYTK::CScript* script_object = nullptr;
			Aurie::AurieStatus last_status = Aurie::AURIE_SUCCESS;

			int function_index = 0;
			last_status = YYTK::GetInterface()->GetNamedRoutineIndex(
				script_name.c_str(),
				&function_index
			);

			// Function indices < 100'000 are built-in functions, not scripts.
			// Above 500'000, there's extension functions - also not scripts.
			if (!Aurie::AurieSuccess(last_status) || function_index < 100'000 || function_index > 500'000)
				throw gcnew System::InvalidOperationException("Script does not exist!");

			// Get the CScript object.
			last_status = YYTK::GetInterface()->GetNamedRoutinePointer(
				script_name.c_str(),
				reinterpret_cast<PVOID*>(&script_object)
			);

			if (!Aurie::AurieSuccess(last_status))
				throw gcnew System::InvalidOperationException("Script does not exist!");

			last_status = Aurie::MmCreateHook(
				Aurie::g_ArSelfModule,
				script_name,
				script_object->m_Functions->m_ScriptFunction,
				NativeScriptHookEntry,
				nullptr
			);

			if (!Aurie::AurieSuccess(last_status))
				throw gcnew System::InvalidOperationException("Failed to hook script!");
		}

		if (m_AfterScriptHandlers->ContainsKey(ScriptName))
		{
			m_AfterScriptHandlers[ScriptName] = static_cast<AfterScriptCallbackHandler^>(Delegate::Combine(m_AfterScriptHandlers[ScriptName], NotifyHandler));
			return;
		}

		return m_AfterScriptHandlers->Add(ScriptName, NotifyHandler);
	}

	void Game::EventController::RemovePreScriptNotification(
		System::String^ ScriptName, 
		BeforeScriptCallbackHandler^ NotifyHandler
	)
	{
		if (m_BeforeScriptHandlers->ContainsKey(ScriptName))
		{
			BeforeScriptCallbackHandler^ new_handler = static_cast<BeforeScriptCallbackHandler^>(Delegate::RemoveAll(m_BeforeScriptHandlers[ScriptName], NotifyHandler));
			if (!new_handler)
				m_BeforeScriptHandlers->Remove(ScriptName);
			else
				m_BeforeScriptHandlers[ScriptName] = new_handler;
		}
	}

	void Game::EventController::RemovePostScriptNotification(
		System::String^ ScriptName, 
		AfterScriptCallbackHandler^ NotifyHandler
	)
	{
		if (m_AfterScriptHandlers->ContainsKey(ScriptName))
		{
			AfterScriptCallbackHandler^ new_handler = static_cast<AfterScriptCallbackHandler^>(Delegate::RemoveAll(m_AfterScriptHandlers[ScriptName], NotifyHandler));
			if (!new_handler)
				m_AfterScriptHandlers->Remove(ScriptName);
			else
				m_AfterScriptHandlers[ScriptName] = new_handler;
		}
	}

	GameObject^ ScriptExecutionContext::Self::get()
	{
		return gcnew GameObject(m_SelfObject);
	}

	GameObject^ ScriptExecutionContext::Other::get()
	{
		return gcnew GameObject(m_OtherObject);
	}

	Gen::IReadOnlyList<GameVariable^>^ ScriptExecutionContext::Arguments::get()
	{
		auto list = gcnew Gen::List<GameVariable^>(this->m_ArgumentCount);

		for (int i = 0; i < m_ArgumentCount; i++)
			list->Add(GameVariable::CreateFromRValue(*m_Arguments[i]));

		return list->AsReadOnly();
	}

	void ScriptExecutionContext::OverrideArgument(
		int Index, 
		GameVariable^ NewValue
	)
	{
		if (Index < 0 || Index > m_ArgumentCount)
			throw gcnew System::IndexOutOfRangeException("Invalid index provided to OverrideArgument!");

		*this->m_Arguments[Index] = NewValue->ToRValue();
	}

	void ScriptExecutionContext::OverrideResult(
		GameVariable^ NewValue
	)
	{
		m_Result = NewValue->ToRValue();
		m_ResultOverridden = true;
	}

	System::String^ ScriptExecutionContext::Name::get()
	{
		return m_Name;
	}
}

