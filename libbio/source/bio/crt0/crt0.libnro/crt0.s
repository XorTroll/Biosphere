.section .text.jmp, "x"

NORELOC__filestart:
.global _start
_start:
        b start
	.word _mod_header - _start
	
	.section .data.mod0
	.word 0, 8

.local _mod_header
_mod_header:
        .ascii "MOD0"
	.word __dynamic_start - _mod_header
	.word __bss_start - _mod_header
	.word __bss_end - _mod_header
	.word 0, 0 // eh_frame_hdr start/end
	.word 0 // runtime-generated module object offset

.global __bio_bin_type
__bio_bin_type: 
    .word 2
        
.section .text, "x"
.global start
start:
	mov w0, #0xd4dd // LIBTRANSISTOR_ERR_TRNLD_NOT_EXECUTABLE ... change this
	movk w0, #0x5d, lsl #16
	ret

.section .bss
	.lcomm _this_should_exist, 4
