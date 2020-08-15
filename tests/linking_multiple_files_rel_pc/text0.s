.extern data_out, addr3
.global addr2
.section text
out2:
    mov $'2', data_out
    jmp *addr3(%pc)
.section addrsec
addr2:
    .word out2
.end

