	lw 1 0 one
	lw 2 0 one
	beq 1 2 loop
loop lw 3 0 two
	add 2 1 2
	beq 2 3 end
end halt
one .fill 1
two .fill 2
