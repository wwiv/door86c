; ----------------------------------------------------------------------------
; hello1.asm
;
; Displays a silly message to standard output.  Illustrates user-defined data.
; The easiest way to do this is to put the data in a data segment, separate 
; from the code, and access it via the ds register.  Note that you must have
; ds:0 pointing to your data segment (technically to your segment's GROUP) 
; before you reference your data.  The predefined symbol @data referes to 
; the group containing the segments created by .data, .data?, .const, 
; .fardata, and .fardata?.
;
; Processor: 386 or later
; Assembler: MASM
; OS: DOS 2.0 or later only
; Assemble and link with "ml hello.asm"
; set WATCOM=C:\WATCOM
; wcl hello.asm
; ----------------------------------------------------------------------------

        .model  small

        .stack  128

        .code
start:  mov     ax, @data
        mov     ds, ax
        mov     ah, 9
        mov     bx, 10h
        add     bx, 1
        add     bx, 1
        add     bx, 1
        add     bx, 1
        lea     dx, Msg
        int     21h
        mov     ah, 4ch
        int     21h

        .data
Msg     byte    'Hello, there.', 13, 10, '$'

        end     start
        