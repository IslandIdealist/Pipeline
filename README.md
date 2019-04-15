## TODO

- [ ] write tests
- [ ] add statistics logic
- [ ] inject noops (bubbling)
- [ ] control hazards (beq)
- [ ] structural hazards (stalls: lw then add)
- [ ] branch prediction
- [ ] make README
- [ ] write overview

### Questions for Dr. Myre

- [ ] Count injected noops as retired?
- [ ] Other data hazards?
- [ ] Questions from code

### Notes

+ Branch prediction
	+ Happens when beq is in decode
	+ Calls 2-bit function
	+ If beq is likely, ?stall, and change PC
	+ (prediction) ? good work : bubble and correct;

+ Tests
	+ Branch into next line
	+ Branch to a branch
