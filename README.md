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

- [ ] Spaces in print?
- [ ] readregA == 0 when PC == 2
- [ ] What is EXMEM.readreg for?
- [ ] Setting newPC during EX?
- [ ] When/where should branch prediction happen?
- [ ] Read/write reg/mem ordering and behavior?

### Notes

+ Branch prediction
	+ Happens when beq is in decode
	+ Calls 2-bit function
	+ If beq is likely, ?stall, and change PC
	+ (prediction) ? good work : bubble and correct;

+ Tests
	+ Branch into next line
	+ Branch to a branch
