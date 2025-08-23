#include "../Interface/Interface.hpp"
#include <intrin.h>
#include <vector>
#include <MemoryUtils/MemoryUtils.hpp>
#include <ZenHv/ZenHv.hpp>
#include <msclr/marshal_cppstd.h>

using namespace msclr::interop;

EXPORTED inline VmxRegisters g_RegisterState = {};

#pragma managed
static void RaiseManagedBeforeScriptEvent(
	IN std::string ScriptName,
	OUT YYTK::RValue& Result,
	IN YYTK::CInstance* Self,
	IN YYTK::CInstance* Other,
	IN int ArgumentCount,
	IN YYTK::RValue** Arguments,
	OUT bool& Overridden
)
{
	YYTKInterop::Game::Events->RaiseBeforeScriptEvent(ScriptName, Result, Self, Other, ArgumentCount, Arguments, Overridden);
}

static void RaiseManagedAfterScriptEvent(
	IN std::string ScriptName,
	OUT YYTK::RValue& Result,
	IN YYTK::CInstance* Self,
	IN YYTK::CInstance* Other,
	IN int ArgumentCount,
	IN YYTK::RValue** Arguments
)
{
	YYTKInterop::Game::Events->RaiseAfterScriptEvent(ScriptName, Result, Self, Other, ArgumentCount, Arguments);
}

static void RaiseManagedBeforeBuiltinEvent(
	IN std::string BuiltinName,
	OUT YYTK::RValue& Result,
	OPTIONAL IN YYTK::CInstance* Self,
	OPTIONAL IN YYTK::CInstance* Other,
	IN int ArgumentCount,
	OPTIONAL IN YYTK::RValue* Arguments,
	OUT bool& Overridden
)
{
	YYTKInterop::Game::Events->RaiseBeforeBuiltinEvent(BuiltinName, Result, Self, Other, ArgumentCount, Arguments, Overridden);
}

static void RaiseManagedAfterBuiltinEvent(
	IN std::string BuiltinName,
	OUT YYTK::RValue& Result,
	OPTIONAL IN YYTK::CInstance* Self,
	OPTIONAL IN YYTK::CInstance* Other,
	IN int ArgumentCount,
	OPTIONAL IN YYTK::RValue* Arguments
)
{
	YYTKInterop::Game::Events->RaiseAfterBuiltinEvent(BuiltinName, Result, Self, Other, ArgumentCount, Arguments);
}

std::string GetCurrentHookName(
	IN System::Collections::Generic::List<System::String^>^ HookList
)
{
	using namespace Aurie;

	// Since we store all registers, and the hook stores all registers, they will match exactly for our hook call.
	// So we can just loop all our hooks, and see which matches.
	// 
	// We loop the hooks that we know we placed...
	for each (auto script in HookList)
	{
		// Convert to an unmanaged string
		std::string hook_name = marshal_as<std::string>(script);

		// Get the current hook
		AurieObject* object = Internal::MmpGetHookByName(
			g_ArSelfModule,
			hook_name
		);

		// Check the object type for the hook - I think it's safe to assume 
		// that it's an RP hook since ASI doesn't do any other hooks, but still, better to be safe.
		if (Internal::ObpGetObjectType(object) != AURIE_OBJECT_RP_HOOK)
		{
			continue;
		}

		// Get the captured registers for our hook
		ProcessorContext context = {};
		auto script_hook = reinterpret_cast<AurieRpHook*>(object);
		if (!AurieSuccess(Internal::MmpGetRegistersForRPHook(
			script_hook,
			context
		)))
		{
			continue;
		}

		// We don't compare RSP, RBP, and RIP, since our g_RegisterState doesn't have it.
		if ((context.RAX == g_RegisterState.RAX) &&
			(context.RBX == g_RegisterState.RBX) &&
			(context.RCX == g_RegisterState.RCX) &&
			(context.RDX == g_RegisterState.RDX) &&
			(context.RSI == g_RegisterState.RSI) &&
			(context.RDI == g_RegisterState.RDI) &&
			(context.R8 == g_RegisterState.R8) &&
			(context.R9 == g_RegisterState.R9) &&
			(context.R10 == g_RegisterState.R10) &&
			(context.R11 == g_RegisterState.R11) &&
			(context.R12 == g_RegisterState.R12) &&
			(context.R13 == g_RegisterState.R13) &&
			(context.R14 == g_RegisterState.R14) &&
			(context.R15 == g_RegisterState.R15)
			)
		{
			return hook_name;
		}
	}

	DbgPrintEx(
		Aurie::LOG_SEVERITY_CRITICAL,
		"[ASI] No hook matches current processor state!"
	);

	YYTK::GetInterface()->GetRunnerInterface().YYError("[ASI] Failed to find correct hook.");
	return "";
}

std::string GetCurrentHookNameScript()
{
	return GetCurrentHookName(YYTKInterop::Game::Events->m_AttachedScripts);
}

std::string GetCurrentHookNameBuiltin()
{
	return GetCurrentHookName(YYTKInterop::Game::Events->m_AttachedBuiltins);
}

#pragma unmanaged

extern "C" void YYTKInterop::NativeBuiltinHook(
	OUT YYTK::RValue& Result,
	OPTIONAL IN YYTK::CInstance* Self,
	OPTIONAL IN YYTK::CInstance* Other,
	IN int ArgumentCount,
	OPTIONAL IN YYTK::RValue* Arguments
)
{
	std::string hook_name = GetCurrentHookNameBuiltin();
	auto original = reinterpret_cast<YYTK::TRoutine>(Aurie::MmGetHookTrampoline(Aurie::g_ArSelfModule, hook_name));

	bool was_builtin_cancelled = false;

	RaiseManagedBeforeBuiltinEvent(hook_name, Result, Self, Other, ArgumentCount, Arguments, was_builtin_cancelled);

	if (was_builtin_cancelled)
		return;

	original(Result, Self, Other, ArgumentCount, Arguments);

	RaiseManagedAfterBuiltinEvent(hook_name, Result, Self, Other, ArgumentCount, Arguments);
}

extern "C" YYTK::RValue& YYTKInterop::NativeScriptHook(
	IN YYTK::CInstance* Self,
	IN YYTK::CInstance* Other,
	OUT YYTK::RValue& Result,
	IN int ArgumentCount,
	IN YYTK::RValue** Arguments
)
{
	std::string hook_name = GetCurrentHookNameScript();
	auto original = reinterpret_cast<YYTK::PFUNC_YYGMLScript>(Aurie::MmGetHookTrampoline(Aurie::g_ArSelfModule, hook_name));

	bool was_script_cancelled = false;

	RaiseManagedBeforeScriptEvent(hook_name, Result, Self, Other, ArgumentCount, Arguments, was_script_cancelled);

	if (was_script_cancelled)
		return Result;

	original(Self, Other, Result, ArgumentCount, Arguments);

	RaiseManagedAfterScriptEvent(hook_name, Result, Self, Other, ArgumentCount, Arguments);

	return Result;
}