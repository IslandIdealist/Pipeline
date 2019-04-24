	lw 3 0 max
loop	lw 1 0 one
	sw 1 0 1
	lw 2 0 two
	sw 2 0 3
	add 4 4 1
	beq 4 3 end
	beq 0 0 loop
end halt
one .fill 1
two .fill 2
max .fill 15
