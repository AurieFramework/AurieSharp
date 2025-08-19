#include <cstdint>
struct VmxRegisters
{
	uint64_t RAX; // GPR Number: 0
	uint64_t RBX; // GPR Number: 3
	uint64_t RCX; // GPR Number: 1
	uint64_t RDX; // GPR Number: 2
	uint64_t RSI; // GPR Number: 6
	uint64_t RDI; // GPR Number: 7
	uint64_t RBP; // GPR Number: 5
	uint64_t RSP; // GPR Number: 4
	uint64_t RIP;
	uint64_t R8; // GPR Number: 8
	uint64_t R9; // GPR Number: 9
	uint64_t R10; // GPR Number: 10
	uint64_t R11; // GPR Number: 11
	uint64_t R12; // GPR Number: 12
	uint64_t R13; // GPR Number: 13
	uint64_t R14; // GPR Number: 14
	uint64_t R15; // GPR Number: 15
};
static_assert(sizeof(VmxRegisters) == 0x88);