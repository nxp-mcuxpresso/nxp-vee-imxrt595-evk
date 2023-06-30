target extended-remote :2331

set print asm-demangle on
set backtrace limit 32
break DefaultHandler
break HardFault
break main
monitor reset 2

load

set language c
set $sp = *0x0
# start the process but immediately halt the processor
stepi
