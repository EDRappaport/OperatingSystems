# writeHello.s
#
# Rappaport, Elliot D
# ECE357: Operating Systems
# November 27, 2013
#
# Print to Screen.

.text
.globl  main

message:
        .string      "Hello !!!\n"

main:
        movl    $4, %eax
        movl    $1, %ebx
        movl    $message, %ecx
        movl    $10, %edx
        int     $0x80
