.global _start
.extern data_out, addr2
.section text
addr0:
    .word out0
out0:
    mov $'0', data_out
    jmp *addr1(%pc)
_start:
    mov $'s', data_out
    jmp *addr0(%pc)
out1:
    mov $'1', data_out
    jmp *addr2(%pc)
addr1:
    .word out1
.end