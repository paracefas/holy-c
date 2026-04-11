entry start

section '.text' code readable executable
start:
    sub rsp, 8          ; Alineación mínima de pila (16-byte boundary)
    sub rsp, 32         ; Shadow space para la API de Windows
    mov rcx, 42         ; El primer argumento en Win64 va en RCX, no en RAX
    call [ExitProcess]  ; Llamada directa a la tabla de importación

section '.idata' import data readable
    library kernel32,'KERNEL32.DLL'
    import kernel32,\
           ExitProcess,'ExitProcess'