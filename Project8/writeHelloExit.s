# writeHelloExit.s
#
# Rappaport, Elliot D
# ECE357: Operating Systems
# November 27, 2013
#
# Print to Screen AND exit code 5.

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
        
        movl    $1, %eax
        movl    $5, %ebx
        int     $0x80
