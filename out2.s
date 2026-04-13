;out.s
format PE64 CONSOLE
include 'win64a.inc'

entry start
section '.text' code readable executable
start:
    invoke ExitProcess, 69

section '.idata' import data readable
	library kernel32,'KERNEL32.DLL'
	import kernel32,ExitProcess,'ExitProcess'