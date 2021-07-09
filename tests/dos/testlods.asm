; ----------------------------------------------------------------------------
; lods.asm
; ----------------------------------------------------------------------------

        .model  small

        .stack  128

        .code
start:  mov     ax, @data
        mov     ds, ax

        xor ax, ax
        mov ah, 0Eh
        mov si, offset Msg
        cld

printmsg:
        lodsb
        cmp al, 0
        jz done
        int 10h
        jmp printmsg
done:

        mov     ah, 9
        lea     dx, DoneMsg
        int     21h
        mov     ah, 4ch
        int     21h

        .data
Msg     byte    'Hello world.', 13, 10, 0
DoneMsg byte    'Done!', 13, 10, '$'

        end     start
        