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

- [ ] Do we need to take out error checking (halt & add000)?
- [ ] If branch taken, are bubbled instrs counted as fetched?
- [ ] How to avoid counting .fill as fetched?
- [ ] Your writedata does reset to 0
- [ ] Where is my memory leak?

### Notes

+ Branch prediction
	+ Happens when beq is in decode
	+ Calls 2-bit function
	+ If beq is likely, ?stall, and change PC
	+ (prediction) ? good work : bubble and correct;

+ Tests
	+ Branch into next line
	+ Branch to a branch
