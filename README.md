## TODO

- [x] write tests
- [x] add statistics logic
- [x] inject noops (bubbling)
- [x] control hazards (beq)
- [x] structural hazards (stalls: lw then add)
- [x] make README
- [x] write overview

### Questions for Dr. Myre

- [x] none

### Tests

+ Stalls
	+ executing {ADD,NAND,LW,SW,BEQ}
	+ from LW
+ Data forwarding
	+ executing {ADD,NAND,LW,SW,BEQ}
	+ from {ADD,NAND,LW}
	+	in {EXMEM,MEMWB,WBEND}
+ Others
	+ given
	+ halt
	+ given from Project 1
	+ what happens if we have LW 1 0 8 right after SW 2 0 8
