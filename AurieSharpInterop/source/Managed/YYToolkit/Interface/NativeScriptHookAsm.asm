.const
PTR_SIZE equ 8h

extern g_RegisterState:QWORD
extern NativeScriptHook:PROC

.code
	; https://github.com/Archie-osu/ZenHv/blob/main/source/asm/vm.asm
	; Saves all GPRs in a format expected by VmxRegisters struct.
	; Assumes that RAX contains the address of a VmxRegisters structure.
	PUSHREGS macro
		mov [rax + PTR_SIZE * 0], rax
		mov [rax + PTR_SIZE * 1], rbx
		mov [rax + PTR_SIZE * 2], rcx
		mov [rax + PTR_SIZE * 3], rdx
		mov [rax + PTR_SIZE * 4], rsi
		mov [rax + PTR_SIZE * 5], rdi
		mov [rax + PTR_SIZE * 6], rbp
		; Don't save RSP
		; Don't save RIP
		mov [rax + PTR_SIZE * 9], r8
		mov [rax + PTR_SIZE * 10], r9
		mov [rax + PTR_SIZE * 11], r10
		mov [rax + PTR_SIZE * 12], r11
		mov [rax + PTR_SIZE * 13], r12
		mov [rax + PTR_SIZE * 14], r13
		mov [rax + PTR_SIZE * 15], r14
		mov [rax + PTR_SIZE * 16], r15
	endm

	; Loads all GPRs from a VmxRegisters struct.
	; Assumes that RAX contains the address of a VmxRegisters structure.
	POPREGS macro
		; Don't load RAX
		mov rbx, [rax + PTR_SIZE * 1]
		mov rcx, [rax + PTR_SIZE * 2]
		mov rdx, [rax + PTR_SIZE * 3]
		mov rsi, [rax + PTR_SIZE * 4]
		mov rdi, [rax + PTR_SIZE * 5]
		mov rbp, [rax + PTR_SIZE * 6]
		; Don't load RSP
		; Don't load RIP
		mov r8, [rax + PTR_SIZE * 9]
		mov r9, [rax + PTR_SIZE * 10]
		mov r10, [rax + PTR_SIZE * 11]
		mov r11, [rax + PTR_SIZE * 12]
		mov r12, [rax + PTR_SIZE * 13]
		mov r13, [rax + PTR_SIZE * 14]
		mov r14, [rax + PTR_SIZE * 15]
		mov r15, [rax + PTR_SIZE * 16]
	endm

	NativeScriptHookEntry proc
		lea rax, g_RegisterState
		PUSHREGS
		jmp NativeScriptHook
	NativeScriptHookEntry endp
end