    lw 1 0 one
    lw 2 0 two
    lw 3 0 tres
    lw 4 0 four
    beq 0 0 loop
loop add 5 3 4
    add 5 5 3
    lw 6 0 ten
    beq 6 5 end
    end halt
  one .fill 1
  two .fill 2
  tres .fill 3
  four .fill 4
  
