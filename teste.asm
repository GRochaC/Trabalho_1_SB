section text
INPut N 

Load n 
FAT: 
add oNE
jmpz fim
store aux
mult n
sTore N 
load aux 
jmp fat
FIM: output N 
stop
secTion data 
aux: space
n: space 
one: const 0xffff