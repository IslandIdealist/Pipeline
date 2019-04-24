				lw		1	0	mcand
				lw		2	0	mplier
				lw		3	0	sMask	# start mask
				lw		4	0	eMask	# end mask
				nand	5	0	0			# reg5 = -1
loop		beq		3	4	last	# while sMask != eMask
				nand	6	3	2			# reg6 = mplier NAND sMask
				beq		5	6	fi		# if reg6 != -1
				add		7	7	1			# result += mcand
fi			add		1	1	1			# left bitshift mcand
				add		3	3	3			# left bitshift sMask
				beq		0	0	loop	# restart loop
last		nand	6	3	2			# last multiply
				beq		5	6	done	# if reg6 != -1
				add		7	7	1			# result += mcand
done		halt						# halt
mcand		.fill	29562
mplier	.fill	11834
sMask		.fill	1
eMask		.fill	16384
