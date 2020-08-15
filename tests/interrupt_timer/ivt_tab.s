.extern _start
.section ivt_tab
    .word _start
    .word _invalid_instruction
    .word _timer
    .word _terminal

.section ivt_tab.text
.equ data_in, 0xFF00
.equ data_out, 0xFF02
_invalid_instruction:
    iret
_timer:
    add $1, cnt
    mov cnt, %r0
    add $'0', %r0
    mov %r0, data_out
    cmp cnt, $9
    jne exit
    mov $0, cnt
exit:
    iret
cnt:
    .word 0
_terminal:
    cmp data_in, $'q'
    jeq quit
    iret
quit:
    halt
.end

