
.586
.model flat, C
option casemap :none
option prologue:none
option epilogue:none
option language: basic ; invalid language removes leading "_" from output name.


;
;  This file mostly contains the constructors what we need to jump to
;  without any additional code being rolled out.
;

.code

;
;  Implementation macro.
;
ASM_DEFINE_IMPLEMENTATION macro name, address
    name proc
        mov eax, address
        jmp eax
        ; ECHO Warning: MakeName("0x&address", "&name")
    name endp
    align 10h
endm

end