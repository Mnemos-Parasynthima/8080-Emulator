- Make a small kernel, interfaced by a shell
- In shell, can use the assembler program to assemble a file

```
bash> ./machine

WELCOME TO MACHINE, 8080 INTEL PROCESSOR, ARUEL KERNEL, 64KB RAM

machine-kernel-shell> as add.s -o add
machine-kernel-shell> add
machine-kernel-shell> adb add
machine-kernel-shell> objdump -D add
machine-kernel-shell> objdump -t add

# assembles add.s, outputting to add
# performs add program
# can debug using Aruel DeBugger on add
# can objdump entire program
# can objdump the symbol table
```

- Option to set total stack space. Defaults to 30 KB
- 30KB of stack, 4KB of reserved?, 30KB of code+data
- Loader:
	- Checks size of program, not allowing if more than 30KB

- Basic kernel/machine
	- Uniprogramming
	- Basic I/O
		- Uses C's I/O fxns
	- Basic syscall interrupt system

- Debugger
	- 