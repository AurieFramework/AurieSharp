#include "../Interface/Interface.hpp"
#include <intrin.h>
#include <vector>
#include <MemoryUtils/MemoryUtils.hpp>
#include <ZenHv/ZenHv.hpp>

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

#pragma unmanaged

EXPORTED inline VmxRegisters g_RegisterState = {};

// Handles retarded fucking stupid instructions
bool HandleFuckedInstruction(
	IN ZydisDisassembledInstruction Instruction,
	OUT ZyanU64& AssumedRegisterValue
)
{
	if (Instruction.info.mnemonic == ZYDIS_MNEMONIC_CALL)
	{
		if (Instruction.operands[0].type == ZYDIS_OPERAND_TYPE_REGISTER)
		{
			switch (ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Instruction.operands[0].reg.value))
			{
			case ZYDIS_REGISTER_RAX:
			case ZYDIS_REGISTER_RSP:
			case ZYDIS_REGISTER_RBP:
				return false;
			case ZYDIS_REGISTER_RBX:
				AssumedRegisterValue = g_RegisterState.RBX;
				return true;
			case ZYDIS_REGISTER_RCX:
				AssumedRegisterValue = g_RegisterState.RCX;
				return true;
			case ZYDIS_REGISTER_RDX:
				AssumedRegisterValue = g_RegisterState.RDX;
				return true;
			case ZYDIS_REGISTER_RSI:
				AssumedRegisterValue = g_RegisterState.RSI;
				return true;
			case ZYDIS_REGISTER_RDI:
				AssumedRegisterValue = g_RegisterState.RDI;
				return true;
			case ZYDIS_REGISTER_R8:
				AssumedRegisterValue = g_RegisterState.R8;
				return true;
			case ZYDIS_REGISTER_R9:
				AssumedRegisterValue = g_RegisterState.R9;
				return true;
			case ZYDIS_REGISTER_R10:
				AssumedRegisterValue = g_RegisterState.R10;
				return true;
			case ZYDIS_REGISTER_R11:
				AssumedRegisterValue = g_RegisterState.R11;
				return true;
			case ZYDIS_REGISTER_R12:
				AssumedRegisterValue = g_RegisterState.R12;
				return true;
			case ZYDIS_REGISTER_R13:
				AssumedRegisterValue = g_RegisterState.R13;
				return true;
			case ZYDIS_REGISTER_R14:
				AssumedRegisterValue = g_RegisterState.R14;
				return true;
			case ZYDIS_REGISTER_R15:
				AssumedRegisterValue = g_RegisterState.R15;
				return true;
			}
		}
	}

	return false;
}

extern "C" void YYTKInterop::NativeBuiltinHook(
	OUT YYTK::RValue& Result,
	OPTIONAL IN YYTK::CInstance* Self,
	OPTIONAL IN YYTK::CInstance* Other,
	IN int ArgumentCount,
	OPTIONAL IN YYTK::RValue* Arguments
)
{
	// Terrible hack ahead!
	// Uses return address to act as a disassembly point, then finds original address of calling script.
	uintptr_t my_caller = reinterpret_cast<uintptr_t>(_ReturnAddress());

	// Decode instructions "some number of bytes" before our return address.
	// Experimenting with lower values may yield speedups, at the cost of accurate disassembly.
	constexpr ptrdiff_t size_to_disassemble = 0x20;
	ZyanU64 decoded_last_instruction = 0;
	auto instructions = YYTK::Memory::DmDecodeInstructionByRange(
		reinterpret_cast<PVOID>(my_caller - size_to_disassemble),
		size_to_disassemble,
		&decoded_last_instruction
	);

	if (instructions.empty())
	{
		DbgPrintEx(
			Aurie::LOG_SEVERITY_CRITICAL,
			"Failed to disassemble for %llX",
			my_caller
		);

		exit(1);
	}

	// Parameters to DmDecodeInstructionByRange specify that we end disassembly at the return address.
	// This means that the last instruction in the instructions vector is the one that caused CF transfer to us.
	// We know that because the return address always points to the instruction *after* a CF-transferring instruction.
	//
	// We get complete disassembly of this instruction.
	auto calling_instruction = YYTK::Memory::DmDisassembleInstruction(
		reinterpret_cast<PVOID>(decoded_last_instruction - instructions.back().length)
	);

	ZyanU64 builtin_address = 0;
	ZydisCalcAbsoluteAddress(
		&calling_instruction.info,
		&calling_instruction.operands[0],
		calling_instruction.runtime_address,
		&builtin_address
	);

	if (!builtin_address)
	{
		if (!HandleFuckedInstruction(calling_instruction, builtin_address))
		{
			DbgPrintEx(
				Aurie::LOG_SEVERITY_CRITICAL,
				"Failed to find target address for instruction %s",
				calling_instruction.text
			);

			exit(1);
		}
	}

	std::string hook_name;
	Aurie::AurieStatus last_status = Aurie::Internal::MmpLookupInlineHookBySourceAddress(
		Aurie::g_ArSelfModule,
		reinterpret_cast<PVOID>(builtin_address),
		hook_name
	);

	if (!Aurie::AurieSuccess(last_status))
	{
		DbgPrintEx(
			Aurie::LOG_SEVERITY_CRITICAL,
			"Failed to find hook for address %llX (instruction determined: %s)",
			builtin_address,
			calling_instruction.text
		);

		exit(1);
	}

	auto original = reinterpret_cast<YYTK::TRoutine>(Aurie::MmGetHookTrampoline(Aurie::g_ArSelfModule, hook_name.c_str()));
	if (!original)
	{
		DbgPrintEx(
			Aurie::LOG_SEVERITY_CRITICAL,
			"Failed to find hook for address %llX (instruction determined: %s)",
			builtin_address,
			calling_instruction.text
		);

		exit(1);
	}

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
	// Terrible hack ahead!
	// Uses return address to act as a disassembly point, then finds original address of calling script.
	uintptr_t my_caller = reinterpret_cast<uintptr_t>(_ReturnAddress());

	// Decode instructions "some number of bytes" before our return address.
	// Experimenting with lower values may yield speedups, at the cost of accurate disassembly.
	constexpr ptrdiff_t size_to_disassemble = 0x20;
	ZyanU64 decoded_last_instruction = 0;
	auto instructions = YYTK::Memory::DmDecodeInstructionByRange(
		reinterpret_cast<PVOID>(my_caller - size_to_disassemble),
		size_to_disassemble,
		&decoded_last_instruction
	);

	if (instructions.empty())
	{
		DbgPrintEx(
			Aurie::LOG_SEVERITY_CRITICAL,
			"Failed to disassemble for %llX",
			my_caller
		);

		exit(1);
	}

	// Parameters to DmDecodeInstructionByRange specify that we end disassembly at the return address.
	// This means that the last instruction in the instructions vector is the one that caused CF transfer to us.
	// We know that because the return address always points to the instruction *after* a CF-transferring instruction.
	//
	// We get complete disassembly of this instruction.
	auto calling_instruction = YYTK::Memory::DmDisassembleInstruction(
		reinterpret_cast<PVOID>(decoded_last_instruction - instructions.back().length)
	);

	ZyanU64 script_address = 0;
	ZydisCalcAbsoluteAddress(
		&calling_instruction.info,
		&calling_instruction.operands[0],
		calling_instruction.runtime_address,
		&script_address
	);

	if (!script_address)
	{
		if (!HandleFuckedInstruction(calling_instruction, script_address))
		{
			DbgPrintEx(
				Aurie::LOG_SEVERITY_CRITICAL,
				"Failed to find target address for instruction %s",
				calling_instruction.text
			);

			exit(1);
		}
	}

	std::string hook_name;
	Aurie::AurieStatus last_status = Aurie::Internal::MmpLookupInlineHookBySourceAddress(
		Aurie::g_ArSelfModule,
		reinterpret_cast<PVOID>(script_address),
		hook_name
	);

	if (!Aurie::AurieSuccess(last_status))
	{
		DbgPrintEx(
			Aurie::LOG_SEVERITY_CRITICAL,
			"Failed to find hook for address %llX (instruction determined: %s)",
			script_address,
			calling_instruction.text
		);

		exit(1);
	}

	auto original = reinterpret_cast<YYTK::PFUNC_YYGMLScript>(Aurie::MmGetHookTrampoline(Aurie::g_ArSelfModule, hook_name.c_str()));
	if (!original)
	{
		DbgPrintEx(
			Aurie::LOG_SEVERITY_CRITICAL,
			"Failed to find hook for address %llX (instruction determined: %s)",
			script_address,
			calling_instruction.text
		);

		exit(1);
	}

	bool was_script_cancelled = false;

	RaiseManagedBeforeScriptEvent(hook_name, Result, Self, Other, ArgumentCount, Arguments, was_script_cancelled);

	if (was_script_cancelled)
		return Result;

	original(Self, Other, Result, ArgumentCount, Arguments);

	RaiseManagedAfterScriptEvent(hook_name, Result, Self, Other, ArgumentCount, Arguments);

	return Result;
}