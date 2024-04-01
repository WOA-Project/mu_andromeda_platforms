	.arch armv8-a
	.file	"PoC.c"
	.text
	.align	2
	.global	PreOslArm64TransferToKernel
	.type	PreOslArm64TransferToKernel, %function
PreOslArm64TransferToKernel:
	.cfi_startproc

				STP 			X19, X20, [SP, #-0X30]!
				STP				X21, X22, [SP, #0X10]
				STR				X30, [SP, #0X20]
                TBZ             X10, #0x28, _no_irm
                AND             X22, X10, #0xFFFFFEFFFFFFFFFF
                MOV             W19, #0
                mrs             X1, MPIDR_EL1
                UBFX            X21, X1, #8, #4
                MOV             X20, #0
_loop:
                CMP             X20, X21
                B.EQ            _skip_current_cpu
                UBFIZ           X8, X19, #8, #0x18
                ORR             X8, X8, X22
                ORR             X10, X8, #1
                MSR             ICC_SGI1R_EL1, X10
                DSB             SY
_skip_current_cpu:
                ADD             W19, W19, #1
                CMP             W19, #8
                ADD             X20, X20, #1
                B.CC            _loop
                B               _exit
_no_irm:
                MSR             ICC_SGI1R_EL1, X10
                DSB             SY
_exit:
				LDR 			X30, [SP, #0X20]
				LDP 			X21, X22, [SP, #0X10]
				LDP 			X19, X20, [SP], #0X30
                RET

	.cfi_endproc
.LFE0:
	.size	PreOslArm64TransferToKernel, .-PreOslArm64TransferToKernel
	.ident	"GCC: (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0"
	.section	.note.GNU-stack,"",@progbits
