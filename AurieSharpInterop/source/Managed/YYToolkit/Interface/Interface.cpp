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

		Game::Events->RaiseObjectEvent(code->GetName(), self, other, code, argc, argv);
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
		String^ Name,
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

	GameVariable^ Game::EngineController::CallScript(String^ Name, ...array<GameVariable^>^ Arguments)
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
		String^ Name, 
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
			throw gcnew InvalidOperationException("Failed to call game script!");

		return GameVariable::CreateFromRValue(result);
	}

	GameRoom^ Game::EngineController::GetStaticRoom(int RoomId)
	{
		YYTK::CRoom* room = nullptr;
		auto last_status = YYTK::GetInterface()->GetRoomData(
			RoomId,
			room
		);

		if (!Aurie::AurieSuccess(last_status))
			throw gcnew ArgumentException("Attempted to fetch room with invalid index.");

		return gcnew GameRoom(room);
	}

	GameRoom^ Game::EngineController::GetStaticRoom(String^ RoomName)
	{
		auto last_status = Aurie::AURIE_SUCCESS;
		std::string room_name = marshal_as<std::string>(RoomName);

		int index = 0;
		do
		{
			YYTK::CRoom* room = nullptr;
			last_status = YYTK::GetInterface()->GetRoomData(
				index,
				room
			);

			if (Aurie::AurieSuccess(last_status))
				if (_stricmp(room_name.c_str(), room->m_Caption))
					return gcnew GameRoom(room);

			index++;
		} while (Aurie::AurieSuccess(last_status));

		throw gcnew ArgumentException("Attempted to fetch room with invalid name.");
	}

	GameRoom^ Game::EngineController::GetRunningRoom()
	{
		YYTK::CRoom* room = nullptr;
		auto last_status = YYTK::GetInterface()->GetCurrentRoomData(room);

		if (!Aurie::AurieSuccess(last_status))
			throw gcnew InvalidOperationException("Failed to grab current room!");

		return gcnew GameRoom(room);
	}

	void Game::EventController::RaiseObjectEvent(
		std::string Name, 
		YYTK::CInstance* Self,
		YYTK::CInstance* Other,
		YYTK::CCode* CodeObject, 
		int ArgumentCount,
		YYTK::RValue* Arguments
	)
	{
		// Some events (eg. GlobalScripts) are not called on real instances.
		// Constructing a GameInstance from that will throw.
		// This is a fault of C++ YYTK using a type it shouldn't.

		CodeExecutionContext^ context = gcnew CodeExecutionContext(
			Name,
			Self,
			Other,
			CodeObject,
			ArgumentCount,
			Arguments
		);

		OnGameEvent(context);
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

		for each (auto kv_pair in m_BeforeScriptHandlers)
		{
			BeforeScriptCallbackHandler^ handler = nullptr;
			if (kv_pair.Value->TryGetValue(context->Name, handler))
				handler->Invoke(context);
		}

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

		for each(auto kv_pair in m_AfterScriptHandlers)
		{
			AfterScriptCallbackHandler^ handler = nullptr;
			if (kv_pair.Value->TryGetValue(context->Name, handler))
				handler->Invoke(context);
		}
	}

	void Game::EventController::RaiseBeforeBuiltinEvent(
		IN std::string BuiltinName,
		OUT YYTK::RValue& Result, 
		OPTIONAL IN YYTK::CInstance* Self,
		OPTIONAL IN YYTK::CInstance* Other,
		IN int ArgumentCount, 
		OPTIONAL IN YYTK::RValue* Arguments,
		OUT bool& Overridden
	)
	{
		BuiltinExecutionContext^ context = gcnew BuiltinExecutionContext(
			BuiltinName,
			Result,
			Self,
			Other,
			ArgumentCount,
			Arguments
		);

		for each (auto kv_pair in m_BeforeBuiltinHandlers)
		{
			BeforeBuiltinCallbackHandler^ handler = nullptr;
			if (kv_pair.Value->TryGetValue(context->Name, handler))
				handler->Invoke(context);
		}

		Overridden = context->m_ResultOverridden;
	}

	void Game::EventController::RaiseAfterBuiltinEvent(
		IN std::string BuiltinName, 
		OUT YYTK::RValue& Result,
		OPTIONAL IN YYTK::CInstance* Self,
		OPTIONAL IN YYTK::CInstance* Other,
		IN int ArgumentCount, 
		OPTIONAL IN YYTK::RValue* Arguments
	)
	{
		BuiltinExecutionContext^ context = gcnew BuiltinExecutionContext(
			BuiltinName,
			Result,
			Self,
			Other,
			ArgumentCount,
			Arguments
		);

		for each (auto kv_pair in m_AfterBuiltinHandlers)
		{
			AfterBuiltinCallbackHandler^ handler = nullptr;
			if (kv_pair.Value->TryGetValue(context->Name, handler))
				handler->Invoke(context);
		}
	}

	Aurie::AurieStatus Game::EventController::AttachTargetBuiltinToNBH(
		std::string BuiltinName
	)
	{
		using namespace Aurie;
		AurieStatus last_status = AURIE_SUCCESS;
		YYTK::YYTKInterface* module_interface = YYTK::GetInterface();

		// Check if the hook exists.
		// If it does, we just return AURIE_SUCCESS, since MmHookExists finds it.
		last_status = MmHookExists(g_ArSelfModule, BuiltinName.c_str());
		if (AurieSuccess(last_status))
			return last_status;

		int function_index = 0;
		last_status = module_interface->GetNamedRoutineIndex(
			BuiltinName.c_str(),
			&function_index
		);

		// Function indices < 100'000 are built-in functions.
		// They also can't have indices below zero.
		if (!AurieSuccess(last_status) || function_index < 0 || function_index >= 100'000)
			return AURIE_OBJECT_NOT_FOUND;

		// Get the CScript object.
		YYTK::TRoutine routine = nullptr;
		last_status = YYTK::GetInterface()->GetNamedRoutinePointer(
			BuiltinName.c_str(),
			reinterpret_cast<PVOID*>(&routine)
		);

		if (!AurieSuccess(last_status))
			return last_status;

		// Create the hook to attach the method to NSH
		last_status = Aurie::MmCreateHook(
			g_ArSelfModule,
			BuiltinName,
			routine,
			NativeBuiltinHookEntry,
			nullptr
		);

		if (AurieSuccess(last_status))
			m_AttachedBuiltins->Add(gcnew System::String(BuiltinName.c_str()));

		return last_status;
	}

	Aurie::AurieStatus Game::EventController::DetachTargetBuiltinFromNBH(
		std::string BuiltinName
	)
	{
		m_AttachedBuiltins->Remove(gcnew System::String(BuiltinName.c_str()));

		return Aurie::MmRemoveHook(
			Aurie::g_ArSelfModule,
			BuiltinName
		);
	}

	Aurie::AurieStatus Game::EventController::AttachTargetScriptToNSH(
		std::string ScriptName
	)
	{
		using namespace Aurie;
		AurieStatus last_status = AURIE_SUCCESS;
		YYTK::YYTKInterface* module_interface = YYTK::GetInterface();

		// Check if the hook exists.
		// If it does, we just return AURIE_SUCCESS, since MmHookExists finds it.
		last_status = MmHookExists(g_ArSelfModule, ScriptName.c_str());
		if (AurieSuccess(last_status))
			return last_status;

		int function_index = 0;
		last_status = module_interface->GetNamedRoutineIndex(
			ScriptName.c_str(),
			&function_index
		);

		// Function indices < 100'000 are built-in functions, not scripts.
		// Above 500'000, there's extension functions - also not scripts.
		if (!AurieSuccess(last_status) || function_index < 100'000 || function_index > 500'000)
			return AURIE_OBJECT_NOT_FOUND;

		// Get the CScript object.
		YYTK::CScript* script_object = nullptr;
		last_status = YYTK::GetInterface()->GetNamedRoutinePointer(
			ScriptName.c_str(),
			reinterpret_cast<PVOID*>(&script_object)
		);

		if (!AurieSuccess(last_status))
			return last_status;

		// Create the hook to attach the method to NSH
		last_status = Aurie::MmCreateHook(
			g_ArSelfModule,
			ScriptName,
			script_object->m_Functions->m_ScriptFunction,
			NativeScriptHookEntry,
			nullptr
		);

		if (AurieSuccess(last_status))
			m_AttachedScripts->Add(gcnew System::String(ScriptName.c_str()));

		return last_status;
	}

	Aurie::AurieStatus Game::EventController::DetachTargetScriptFromNSH(
		std::string ScriptName
	)
	{
		m_AttachedScripts->Remove(gcnew System::String(ScriptName.c_str()));

		return Aurie::MmRemoveHook(
			Aurie::g_ArSelfModule,
			ScriptName
		);
	}

	bool Game::EventController::GetOrCreateModScopedPreScriptCallbackDict(
		AurieSharpInterop::AurieManagedModule^ Module, 
		Gen::Dictionary<String^, BeforeScriptCallbackHandler^>^% Scripts
	)
	{
		if (!m_BeforeScriptHandlers->ContainsKey(Module))
			m_BeforeScriptHandlers->Add(Module, gcnew Gen::Dictionary<String^, BeforeScriptCallbackHandler^>(0));

		return m_BeforeScriptHandlers->TryGetValue(Module, Scripts);
	}

	bool Game::EventController::GetOrCreateModScopedPostScriptCallbackDict(
		AurieSharpInterop::AurieManagedModule^ Module, 
		Gen::Dictionary<String^, AfterScriptCallbackHandler^>^% Scripts
	)
	{
		if (!m_AfterScriptHandlers->ContainsKey(Module))
			m_AfterScriptHandlers->Add(Module, gcnew Gen::Dictionary<String^, AfterScriptCallbackHandler^>(0));

		return m_AfterScriptHandlers->TryGetValue(Module, Scripts);
	}

	bool Game::EventController::GetOrCreateModScopedPreBuiltinCallbackDict(
		AurieSharpInterop::AurieManagedModule^ Module, 
		Gen::Dictionary<System::String^, BeforeBuiltinCallbackHandler^>^% Builtins
	)
	{
		if (!m_BeforeBuiltinHandlers->ContainsKey(Module))
			m_BeforeBuiltinHandlers->Add(Module, gcnew Gen::Dictionary<String^, BeforeBuiltinCallbackHandler^>(0));

		return m_BeforeBuiltinHandlers->TryGetValue(Module, Builtins);
	}

	bool Game::EventController::GetOrCreateModScopedPostBuiltinCallbackDict(
		AurieSharpInterop::AurieManagedModule^ Module,
		Gen::Dictionary<System::String^, AfterBuiltinCallbackHandler^>^% Builtins
	)
	{
		if (!m_AfterBuiltinHandlers->ContainsKey(Module))
			m_AfterBuiltinHandlers->Add(Module, gcnew Gen::Dictionary<String^, AfterBuiltinCallbackHandler^>(0));

		return m_AfterBuiltinHandlers->TryGetValue(Module, Builtins);
	}

	bool Game::EventController::IsScriptHooked(
		System::String^ ScriptName
	)
	{
		bool script_in_use = false;
		for each (auto kv_pair in m_BeforeScriptHandlers)
		{
			if (kv_pair.Value->ContainsKey(ScriptName))
			{
				script_in_use = true;
				break;
			}
		}

		for each (auto kv_pair in m_AfterScriptHandlers)
		{
			if (kv_pair.Value->ContainsKey(ScriptName))
			{
				script_in_use = true;
				break;
			}
		}

		return script_in_use;
	}

	bool Game::EventController::IsBuiltinHooked(
		System::String^ BuiltinName
	)
	{
		bool builtin_in_use = false;
		for each (auto kv_pair in m_BeforeBuiltinHandlers)
		{
			if (kv_pair.Value->ContainsKey(BuiltinName))
			{
				builtin_in_use = true;
				break;
			}
		}

		for each (auto kv_pair in m_BeforeBuiltinHandlers)
		{
			if (kv_pair.Value->ContainsKey(BuiltinName))
			{
				builtin_in_use = true;
				break;
			}
		}

		return builtin_in_use;
	}

	void Game::EventController::DetachScriptIfUnused(
		String^ ScriptName
	)
	{
		if (!IsScriptHooked(ScriptName))
			DetachTargetScriptFromNSH(marshal_as<std::string>(ScriptName));
	}

	void Game::EventController::DetachBuiltinIfUnused(
		System::String^ BuiltinName
	)
	{
		if (!IsScriptHooked(BuiltinName))
			DetachTargetScriptFromNSH(marshal_as<std::string>(BuiltinName));
	}

	void Game::EventController::RemoveAllBuiltinHooksForMod(
		AurieSharpInterop::AurieManagedModule^ Module
	)
	{
		Gen::List<String^>^ before_builtin_dict_keys = gcnew Gen::List<String^>(4);
		Gen::List<String^>^ after_builtin_dict_keys = gcnew Gen::List<String^>(4);

		Gen::Dictionary<String^, BeforeBuiltinCallbackHandler^>^ before_builtin_dict = nullptr;
		Gen::Dictionary<String^, AfterBuiltinCallbackHandler^>^ after_builtin_dict = nullptr;

		if (m_BeforeBuiltinHandlers->TryGetValue(Module, before_builtin_dict))
		{
			// Loop to store all keys in a native vector (and null the entry)
			for each (auto kv_pair in before_builtin_dict)
				before_builtin_dict_keys->Add(kv_pair.Key);

			// Actually remove all the entries
			for each (auto name in before_builtin_dict_keys)
			{
				before_builtin_dict->Remove(name);
				DetachBuiltinIfUnused(name);
			}

			m_BeforeBuiltinHandlers->Remove(Module);
		}

		if (m_AfterBuiltinHandlers->TryGetValue(Module, after_builtin_dict))
		{
			// Loop to store all keys in a native vector (and null the entry)
			for each (auto kv_pair in after_builtin_dict)
				after_builtin_dict_keys->Add(kv_pair.Key);

			// Actually remove all the entries
			for each (auto name in after_builtin_dict_keys)
			{
				after_builtin_dict->Remove(name);
				DetachBuiltinIfUnused(name);
			}

			m_AfterBuiltinHandlers->Remove(Module);
		}
	}

	void Game::EventController::RemoveAllScriptHooksForMod(
		AurieSharpInterop::AurieManagedModule^ Module
	)
	{
		Gen::List<String^>^ before_script_dict_keys = gcnew Gen::List<String^>(4);
		Gen::List<String^>^ after_script_dict_keys = gcnew Gen::List<String^>(4);

		Gen::Dictionary<String^, BeforeScriptCallbackHandler^>^ before_script_dict = nullptr;
		Gen::Dictionary<String^, AfterScriptCallbackHandler^>^ after_script_dict = nullptr;

		if (m_BeforeScriptHandlers->TryGetValue(Module, before_script_dict))
		{
			// Loop to store all keys in a native vector (and null the entry)
			for each (auto kv_pair in before_script_dict)
				before_script_dict_keys->Add(kv_pair.Key);

			// Actually remove all the entries
			for each (auto name in before_script_dict_keys)
			{
				before_script_dict->Remove(name);
				DetachScriptIfUnused(name);
			}

			m_BeforeScriptHandlers->Remove(Module);
		}

		if (m_AfterScriptHandlers->TryGetValue(Module, after_script_dict))
		{
			// Loop to store all keys in a native vector (and null the entry)
			for each (auto kv_pair in after_script_dict)
				after_script_dict_keys->Add(kv_pair.Key);

			// Actually remove all the entries
			for each (auto name in after_script_dict_keys)
			{
				after_script_dict->Remove(name);
				DetachScriptIfUnused(name);
			}
				
			m_AfterScriptHandlers->Remove(Module);
		}
	}
	
	void Game::EventController::AddPreScriptNotification(
		AurieSharpInterop::AurieManagedModule^ CurrentModule,
		String^ ScriptName,
		BeforeScriptCallbackHandler^ NotifyHandler
	)
	{
		std::string script_name = marshal_as<std::string>(ScriptName);

		auto last_status = AttachTargetScriptToNSH(script_name);
		if (!Aurie::AurieSuccess(last_status))
			throw gcnew InvalidOperationException("Failed to attach to script!");

		Gen::Dictionary<String^, BeforeScriptCallbackHandler^>^ script_list = nullptr;
		if (GetOrCreateModScopedPreScriptCallbackDict(CurrentModule, script_list))
		{
			BeforeScriptCallbackHandler^ existing_handler = nullptr;
			script_list->TryGetValue(ScriptName, existing_handler);

			auto combined = static_cast<BeforeScriptCallbackHandler^>(
				Delegate::Combine(existing_handler, NotifyHandler)
			);

			script_list[ScriptName] = combined;
		}
	}

	void Game::EventController::AddPostScriptNotification(
		AurieSharpInterop::AurieManagedModule^ CurrentModule,
		String^ ScriptName, 
		AfterScriptCallbackHandler^ NotifyHandler
	)
	{
		std::string script_name = marshal_as<std::string>(ScriptName);

		auto last_status = AttachTargetScriptToNSH(script_name);
		if (!Aurie::AurieSuccess(last_status))
			throw gcnew InvalidOperationException("Failed to attach to script!");

		Gen::Dictionary<String^, AfterScriptCallbackHandler^>^ script_list = nullptr;
		if (GetOrCreateModScopedPostScriptCallbackDict(CurrentModule, script_list))
		{
			AfterScriptCallbackHandler^ existing_handler = nullptr;
			script_list->TryGetValue(ScriptName, existing_handler);

			auto combined = static_cast<AfterScriptCallbackHandler^>(
				Delegate::Combine(existing_handler, NotifyHandler)
			);

			script_list[ScriptName] = combined;
		}
	}

	void Game::EventController::RemovePreScriptNotification(
		AurieSharpInterop::AurieManagedModule^ CurrentModule,
		String^ ScriptName, 
		BeforeScriptCallbackHandler^ NotifyHandler
	)
	{
		Gen::Dictionary<String^, BeforeScriptCallbackHandler^>^ script_list = nullptr;
		if (!GetOrCreateModScopedPreScriptCallbackDict(CurrentModule, script_list))
			return; // Nothing for this module

		BeforeScriptCallbackHandler^ existing_handler = nullptr;
		if (!script_list->TryGetValue(ScriptName, existing_handler))
			return; // Script not registered for this module

		auto new_handler = static_cast<BeforeScriptCallbackHandler^>(
			Delegate::RemoveAll(existing_handler, NotifyHandler)
		);

		if (new_handler == nullptr)
			script_list->Remove(ScriptName);
		else
			script_list[ScriptName] = new_handler;

		if (script_list->Count == 0)
			m_BeforeScriptHandlers->Remove(CurrentModule);

		DetachScriptIfUnused(ScriptName);
	}

	void Game::EventController::RemovePostScriptNotification(
		AurieSharpInterop::AurieManagedModule^ CurrentModule,
		String^ ScriptName, 
		AfterScriptCallbackHandler^ NotifyHandler
	)
	{
		Gen::Dictionary<String^, AfterScriptCallbackHandler^>^ script_list = nullptr;
		if (!GetOrCreateModScopedPostScriptCallbackDict(CurrentModule, script_list))
			return; // Nothing for this module

		AfterScriptCallbackHandler^ existing_handler = nullptr;
		if (!script_list->TryGetValue(ScriptName, existing_handler))
			return; // Script not registered for this module

		auto new_handler = static_cast<AfterScriptCallbackHandler^>(
			Delegate::RemoveAll(existing_handler, NotifyHandler)
			);

		if (new_handler == nullptr)
			script_list->Remove(ScriptName);
		else
			script_list[ScriptName] = new_handler;

		if (script_list->Count == 0)
			m_BeforeScriptHandlers->Remove(CurrentModule);

		DetachScriptIfUnused(ScriptName);
	}

	void Game::EventController::AddPreBuiltinNotification(
		AurieSharpInterop::AurieManagedModule^ CurrentModule, 
		System::String^ BuiltinName, 
		BeforeBuiltinCallbackHandler^ NotifyHandler
	)
	{
		std::string script_name = marshal_as<std::string>(BuiltinName);

		auto last_status = AttachTargetBuiltinToNBH(script_name);
		if (!Aurie::AurieSuccess(last_status))
			throw gcnew InvalidOperationException("Failed to attach to script!");

		Gen::Dictionary<String^, BeforeBuiltinCallbackHandler^>^ script_list = nullptr;
		if (GetOrCreateModScopedPreBuiltinCallbackDict(CurrentModule, script_list))
		{
			BeforeBuiltinCallbackHandler^ existing_handler = nullptr;
			script_list->TryGetValue(BuiltinName, existing_handler);

			auto combined = static_cast<BeforeBuiltinCallbackHandler^>(
				Delegate::Combine(existing_handler, NotifyHandler)
			);

			script_list[BuiltinName] = combined;
		}
	}

	void Game::EventController::AddPostBuiltinNotification(
		AurieSharpInterop::AurieManagedModule^ CurrentModule,
		System::String^ BuiltinName,
		AfterBuiltinCallbackHandler^ NotifyHandler
	)
	{
		std::string script_name = marshal_as<std::string>(BuiltinName);

		auto last_status = AttachTargetBuiltinToNBH(script_name);
		if (!Aurie::AurieSuccess(last_status))
			throw gcnew InvalidOperationException("Failed to attach to script!");

		Gen::Dictionary<String^, BeforeBuiltinCallbackHandler^>^ builtin_list = nullptr;
		if (GetOrCreateModScopedPreBuiltinCallbackDict(CurrentModule, builtin_list))
		{
			BeforeBuiltinCallbackHandler^ existing_handler = nullptr;
			builtin_list->TryGetValue(BuiltinName, existing_handler);

			auto combined = static_cast<BeforeBuiltinCallbackHandler^>(
				Delegate::Combine(existing_handler, NotifyHandler)
				);

			builtin_list[BuiltinName] = combined;
		}
	}

	void Game::EventController::RemovePreBuiltinNotification(
		AurieSharpInterop::AurieManagedModule^ CurrentModule,
		System::String^ BuiltinName,
		BeforeBuiltinCallbackHandler^ NotifyHandler
	)
	{
		Gen::Dictionary<String^, BeforeBuiltinCallbackHandler^>^ builtin_list = nullptr;
		if (!GetOrCreateModScopedPreBuiltinCallbackDict(CurrentModule, builtin_list))
			return; // Nothing for this module

		BeforeBuiltinCallbackHandler^ existing_handler = nullptr;
		if (!builtin_list->TryGetValue(BuiltinName, existing_handler))
			return; // Script not registered for this module

		auto new_handler = static_cast<BeforeBuiltinCallbackHandler^>(
			Delegate::RemoveAll(existing_handler, NotifyHandler)
			);

		if (new_handler == nullptr)
			builtin_list->Remove(BuiltinName);
		else
			builtin_list[BuiltinName] = new_handler;

		if (builtin_list->Count == 0)
			m_BeforeBuiltinHandlers->Remove(CurrentModule);

		DetachBuiltinIfUnused(BuiltinName);
	}

	void Game::EventController::RemovePostBuiltinNotification(
		AurieSharpInterop::AurieManagedModule^ CurrentModule,
		System::String^ BuiltinName,
		AfterBuiltinCallbackHandler^ NotifyHandler
	)
	{
		Gen::Dictionary<String^, AfterBuiltinCallbackHandler^>^ builtin_list = nullptr;
		if (!GetOrCreateModScopedPostBuiltinCallbackDict(CurrentModule, builtin_list))
			return; // Nothing for this module

		AfterBuiltinCallbackHandler^ existing_handler = nullptr;
		if (!builtin_list->TryGetValue(BuiltinName, existing_handler))
			return; // Script not registered for this module

		auto new_handler = static_cast<AfterBuiltinCallbackHandler^>(
			Delegate::RemoveAll(existing_handler, NotifyHandler)
			);

		if (new_handler == nullptr)
			builtin_list->Remove(BuiltinName);
		else
			builtin_list[BuiltinName] = new_handler;

		if (builtin_list->Count == 0)
			m_AfterBuiltinHandlers->Remove(CurrentModule);

		DetachBuiltinIfUnused(BuiltinName);
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
			list->Add(GameVariable::CreateFromRValue(*(m_Arguments[i])));

		return list->AsReadOnly();
	}

	void ScriptExecutionContext::OverrideArgument(
		int Index, 
		GameVariable^ NewValue
	)
	{
		if (Index < 0 || Index > m_ArgumentCount)
			throw gcnew IndexOutOfRangeException("Invalid index provided to OverrideArgument!");

		*this->m_Arguments[Index] = NewValue->ToRValue();
	}

	void ScriptExecutionContext::OverrideResult(
		GameVariable^ NewValue
	)
	{
		m_Result = NewValue->ToRValue();
		m_ResultOverridden = true;
	}

	GameVariable^ ScriptExecutionContext::GetResult()
	{
		return GameVariable::CreateFromRValue(m_Result);
	}

	String^ ScriptExecutionContext::Name::get()
	{
		return m_Name;
	}

	GameObject^ CodeExecutionContext::Self::get()
	{
		return gcnew GameObject(m_SelfObject);
	}

	GameObject^ CodeExecutionContext::Other::get()
	{
		return gcnew GameObject(m_OtherObject);
	}

	Gen::IReadOnlyList<GameVariable^>^ CodeExecutionContext::Arguments::get()
	{
		auto list = gcnew Gen::List<GameVariable^>(this->m_ArgumentCount);

		for (int i = 0; i < m_ArgumentCount; i++)
			list->Add(GameVariable::CreateFromRValue(m_Arguments[i]));

		return list->AsReadOnly();
	}

	String^ CodeExecutionContext::Name::get()
	{
		return m_Name;
	}

	void CodeExecutionContext::OverrideArgument(
		int Index,
		GameVariable^ NewValue
	)
	{
		if (Index < 0 || Index > m_ArgumentCount)
			throw gcnew IndexOutOfRangeException("Invalid index provided to OverrideArgument!");

		this->m_Arguments[Index] = NewValue->ToRValue();
	}

	String^ BuiltinExecutionContext::Name::get()
	{
		return m_Name;
	}

	GameObject^ BuiltinExecutionContext::Self::get()
	{
		return gcnew GameObject(m_SelfObject);
	}

	GameObject^ BuiltinExecutionContext::Other::get()
	{
		return gcnew GameObject(m_OtherObject);
	}

	Gen::IReadOnlyList<GameVariable^>^ BuiltinExecutionContext::Arguments::get()
	{
		auto list = gcnew Gen::List<GameVariable^>(this->m_ArgumentCount);

		for (int i = 0; i < m_ArgumentCount; i++)
			list->Add(GameVariable::CreateFromRValue(m_Arguments[i]));

		return list->AsReadOnly();
	}

	void BuiltinExecutionContext::OverrideArgument(
		int Index,
		GameVariable^ NewValue
	)
	{
		if (Index < 0 || Index > m_ArgumentCount)
			throw gcnew IndexOutOfRangeException("Invalid index provided to OverrideArgument!");

		this->m_Arguments[Index] = NewValue->ToRValue();
	}

	void BuiltinExecutionContext::OverrideResult(
		GameVariable^ NewValue
	)
	{
		m_Result = NewValue->ToRValue();
		m_ResultOverridden = true;
	}

	GameVariable^ BuiltinExecutionContext::GetResult()
	{
		return GameVariable::CreateFromRValue(m_Result);
	}
}

