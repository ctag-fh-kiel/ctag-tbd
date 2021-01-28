/* ULP assembly files are passed through C preprocessor first, so include directives
   and C macros may be used in these files 
 */

.macro push rx
	st \rx,r3,0
	sub r3,r3,1
.endm

.macro pop rx
	add r3,r3,1
	ld \rx,r3,0
.endm

.macro psr 
	.set addr,(.+16)
	move r1,addr
	push r1
.endm

.macro ret 
	pop r1
	jump r1
.endm

.macro clear param
	move r1, \param
	move r0, 0
	st r0, r1, 0
.endm
