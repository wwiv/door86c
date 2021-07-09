; ----------------------------------------------------------------------------
; lods.asm
; ----------------------------------------------------------------------------

        .model  small

        .stack  128

        .code
start:  mov     ax, @data
        mov     ds, ax

testbody:
        mov si, 0feh
        lea ax, [si]
        cmp ax, 0feh
        je success
        jmp failed

failed:
        mov     ah, 9
        lea     dx, FailMsg
        int     21h
        jmp done

success:
        mov     ah, 9
        lea     dx, SuccessMsg
        int     21h
        jmp done

done:
        mov     ah, 9
        lea     dx, DoneMsg
        int     21h
        mov     ah, 4ch
        int     21h

.data

FailMsg byte            'Failed.', 13, 10, '$'
SuccessMsg byte         'Pass.', 13, 10,'$'
DoneMsg byte            'Done!', 13, 10, '$'

end     start
        
