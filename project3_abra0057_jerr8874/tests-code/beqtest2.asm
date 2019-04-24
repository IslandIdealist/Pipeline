        lw 2 0 two
        lw 6 0 max
loop    add 4 4 2
        beq 4 6 end
        beq 0 0 loop
end     halt
two     .fill 2
max     .fill 100
