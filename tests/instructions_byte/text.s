.global _start
.extern data_out
.section text
_start:
    push char_b
    push char_a
    call write_func
    pop var
    pop var

    xchgb char_a, char_b
    push char_b
    push char_a
    call write_func
    pop var
    pop var

    movb $0, var
    call write_out

    addb $1, var
    call write_out

    subb $-1, var
    call write_out

    mulb $4, var
    call write_out

    divb $2, var
    call write_out

    notb $0xf8, var
    call write_out

    andb $9, var
    call write_out

    orb $6, var
    call write_out

    xorb $0xe, var
    call write_out

    shrb var, $2
    call write_out

    shlb $1, var
    call write_out

    halt

.equ char_a_offs, 2
.equ char_b_offs, 4
write_func:
    movb char_a_offs(%r6), data_out
    movb char_b_offs(%r6), data_out
    movb $0xA, data_out
    ret
write_out:
    addb $'0', var
    movb var, data_out
    movb $0xA, data_out
    subb $'0', var
    ret
var:
    .word 0
char_a:
    .byte 'a'
char_b:
    .byte 'b'
.end