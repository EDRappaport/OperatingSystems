#include "jmpbuf-offsets.h"

	.text
	.global	savectx
	.global restorectx
/* This looks like a function to C, but we skip the usual stack */
/* frame stuff since there are no local variables */
savectx:
	movl	4(%esp),%eax	 	/* Move jump buffer addr into eax */
	movl	%ebx, (0*4)(%eax)
	movl	%esi, (1*4)(%eax)
	movl	%edi, (2*4)(%eax)
	movl	%ebp, (3*4)(%eax)
	leal	4(%esp), %ecx			/*SP when we return */
	movl	%ecx,(4*4)(%eax)
	movl	0(%esp), %ecx			/*return address */
	movl	%ecx, (5*4)(%eax)		/*return address */
	xorl	%eax,%eax			/*return val will be 0 */
	ret

/* Call restorectx(void *jmpbuf, int arg)		*/
restorectx:
	movl	4(%esp),%ecx		/*Move jump buffer addr to ecx */
	movl	8(%esp),%eax		/*Longjmp return value */
	movl	(5*4)(%ecx),%edx	/*Save return addr in edx */
	/* Restore all registers */
	movl	(0*4)(%ecx),%ebx
	movl	(1*4)(%ecx),%esi
	movl	(2*4)(%ecx),%edi
	movl	(3*4)(%ecx),%ebp
	movl	(4*4)(%ecx),%esp
	/* Jump to setjmp point */
	jmp	*%edx
