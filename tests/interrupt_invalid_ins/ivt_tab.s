.extern _start
.global data_in, data_out
.section ivt_tab
    .word _start
    .word _invalid_instruction
    .word _timer
    .word _terminal

.section ivt_tab.text
.equ data_in, 0xFF00
.equ data_out, 0xFF02
_invalid_instruction:
    mov $0, %r0
loop:
    mov msg(%r0), data_out
    add $1, %r0
    cmp msg(%r0), $0
    jne loop
    halt
quit:
    halt
_timer:
    iret
_terminal:
    iret
.section ivt_tab.data
msg:
    .byte 'i'
    .byte 'n'
    .byte 'v'
    .byte 'a'
    .byte 'l'
    .byte 'i'
    .byte 'd'
    .byte ' '
    .byte 'i'
    .byte 'n'
    .byte 's'
    .byte 't'
    .byte 'r'
    .byte 'u'
    .byte 'c'
    .byte 't'
    .byte 'i'
    .byte 'o'
    .byte 'n'
    .byte 0xa
    .byte 0
.end

