.extern data_out
.global addr3
.section text
out3:
    mov $'3', data_out
    halt
.section addrsec
addr3:
    .word out3
.end