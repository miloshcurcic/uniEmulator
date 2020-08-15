.extern d, data_out
.global b, _start
.equ a, d - '0'
.equ b, '2'
.equ sym0, a + b - d
.equ sym1, a + b - d + c
.section text
_start:
    mov $b, data_out
    mov $0, %r0
    mov sym1(%r0), data_out
    jmp *sym_quit(%pc)
.section data
c:
    .word 'a'
    .word 'b'
.section end
.equ sym_quit, quit
quit:
    halt
.end