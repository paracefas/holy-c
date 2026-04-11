format PE64 CONSOLE
entry start

section '.text' code readable executable
start:
    sub rsp, 40          ; Shadow space + align
    mov rcx, 42          ; Valor de retorno
    call [ExitProcess]

section '.idata' import data readable
    ; Estructura manual de la tabla de importaciones
    dd 0,0,0, RVA kernel_name, RVA kernel_table
    dd 0,0,0,0,0

    kernel_table:
        ExitProcess dq RVA _ExitProcess
        dq 0

    kernel_name db 'KERNEL32.DLL',0
    _ExitProcess dw 0
        db 'ExitProcess',0