MOV R0 7
MOV R1 7
CMP R0 R1
JZ equal
MOV R2 -1
JMP end

equal:
MOV R2 42

end:
HALT
