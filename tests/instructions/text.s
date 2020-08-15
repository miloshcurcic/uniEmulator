.global _start
.extern data_out
.section text
_start:
    push char_b
    push char_a
    call write_func
    pop var
    pop var

    xchg char_a, char_b
    push char_b
    push char_a
    call write_func
    pop var
    pop var

    mov $0, var
    call write_out

    add $1, var
    call write_out

    sub $-1, var
    call write_out

    mul $4, var
    call write_out

    div $2, var
    call write_out

    not $0xfff8, var
    call write_out

    and $9, var
    call write_out

    or $6, var
    call write_out

    xor $0xe, var
    call write_out

    shr var, $2
    call write_out

    shl $1, var
    call write_out

    halt

.equ char_a_offs, 2
.equ char_b_offs, 4
write_func:
    mov char_a_offs(%r6), data_out
    mov char_b_offs(%r6), data_out
    mov $0xA, data_out
    ret
write_out:
    add $'0', var
    mov var, data_out
    mov $0xA, data_out
    sub $'0', var
    ret
var:
    .word 0
char_a:
    .word 'a'
char_b:
    .word 'b'
.end