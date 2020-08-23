.global _start
.extern data_out

.section rodata
    halt
.section text
rodata:
    jmp rodata
_start:
    halt
    
