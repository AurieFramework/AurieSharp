#pragma once
#include <Zydis/Zydis.h>
#include <Aurie/shared.hpp>

namespace YYTK
{
	namespace Memory
	{

		/**
		 * \brief Fully disassembles an instruction using the current architecture.
		 * \param InstructionBase The address of the instruction.
		 * \return The disassembled instruction.
		 */
		ZydisDisassembledInstruction DmDisassembleInstruction(
			IN PVOID InstructionBase
		);

		/**
		 * \brief Decodes an instruction using the current architecture.
		 * \param InstructionBase The address of the instruction.
		 * \return The decoded instruction.
		 */
		ZydisDecodedInstruction DmDecodeInstruction(
			IN PVOID InstructionBase
		);

		/**
		 * \brief Decodes a set number of instructions using the current architecture.
		 * \param InstructionBase The address of the initial instruction.
		 * \param InstructionRange The maximum offset from InstructionBase.
		 * \param LastInstruction An optional pointer to a buffer which receives the address at which disassembly ended.
		 * \return The decoded instructions.
		 */
		std::vector<ZydisDecodedInstruction> DmDecodeInstructionByRange(
			IN PVOID InstructionBase,
			IN SIZE_T InstructionRange,
			OPTIONAL OUT ZyanU64* LastInstruction
		);

		/**
		 * \brief Disassembles a set number of instructions using the current architecture.
		 * \param InstructionBase The address of the initial instruction.
		 * \param InstructionCount The number of instructions to disassemble.
		 * \return The disassembled instructions.
		 */
		std::vector<ZydisDisassembledInstruction> DmDisassembleInstructionByCount(
			IN PVOID InstructionBase,
			IN SIZE_T InstructionCount
		);

		/**
		 * \brief Disassembles a set number of instructions using the current architecture.
		 * \param InstructionBase The address of the initial instruction.
		 * \param InstructionRange The maximum offset from InstructionBase.
		 * \return The disassembled instructions.
		 */
		std::vector<ZydisDisassembledInstruction> DmDisassembleInstructionByRange(
			IN PVOID InstructionBase,
			IN SIZE_T InstructionRange
		);

		/**
		 * \brief Retrieves the start and end addresses for a given section inside the game's executable.
		 * \param SectionName The name of the section.
		 * \param SectionStart A buffer which will get populated with the starting address of the section.
		 * \param SectionEnd A buffer which will get populated with the ending address of the section.
		 * \return A status code.
		 */
		Aurie::AurieStatus DmGetSectionBounds(
			IN const char* SectionName,
			OUT uint64_t* SectionStart,
			OUT uint64_t* SectionEnd
		);

		/**
		 * \brief Calculates the address of a decoded instruction given the IP past all the other instructions.
		 * \param Instructions The decoded instructions.
		 * \param Index The index of the instruction whose address is to be computed.
		 * \param PostLastInstructionAddress The instruction pointer past the final instruction.
		 * \return The address of the instruction.
		 */
		uintptr_t DmCalculateInstructionAddress(
			IN const std::vector<ZydisDecodedInstruction>& Instructions,
			IN int64_t Index,
			IN uintptr_t PostLastInstructionAddress
		);

		SIZE_T DmFindMnemonicPattern(
			IN const std::vector<ZydisDecodedInstruction>& Instructions,
			IN const std::vector<ZydisMnemonic>& Mnemonics,
			OPTIONAL IN SIZE_T LoopStartIndex
		);

		SIZE_T DmFindMnemonicPattern(
			IN const std::vector<ZydisDisassembledInstruction>& Instructions,
			IN const std::vector<ZydisMnemonic>& Mnemonics,
			OPTIONAL IN SIZE_T LoopStartIndex
		);

		size_t DmSigscanGame(
			IN const unsigned char* Pattern,
			IN const char* Mask
		);

		std::vector<size_t> DmSigscanGameEx(
			IN const unsigned char* Pattern,
			IN const char* Mask
		);
	}
}