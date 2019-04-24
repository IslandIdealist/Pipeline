		lw 1 0 five
		lw 5 0 nine
		add 2 0 1
		nand 3 2 0
		sw 2 0 nine
		beq 0 0 fi
		sw 0 0 five
fi	noop
		jalr 4 5
		noop
		beq 0 1 fi
		halt
five	.fill 5
nine	.fill 9
