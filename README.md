# opcodenator
opcodenator is a tool that generates a decoder for a given list of opcodes.

## Usage
1. Provide a list of opcodes.
2. opcodenator will identify any collisions or duplicates.
3. Merge the colliding/duplicate opcodes into a single opcode.
4. Testing, manual stages if needed and final implementation.

It's not a 1 -> 1 solution, takes a bit of manual work but should save a ton of time, and more importantly, it's less error prone than doing this manually.

See [example](example) of how I used this to implement a decoder for the AVR instruction set.
