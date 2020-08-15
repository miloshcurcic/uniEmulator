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
    iret
    .equ diff, 'a' - 'A'
_terminal:
    mov data_in, %r0
    mov %r0, data_out

    cmp %r0, $'a'
    jgt exit

    cmp $'z', %r0
    jgt exit

    cmp $'q', %r0
    jeq quit

    sub $diff, data_out
exit:
    iret
quit: 
    halt
.end

