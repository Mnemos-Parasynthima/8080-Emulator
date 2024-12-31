- Make a small kernel, interfaced by a shell
- In shell, can use the assembler program to assemble a file

```
bash> ./machine

WELCOME TO MACHINE, 8080 INTEL PROCESSOR, ARUEL KERNEL, 64KB RAM

machine-kernel-shell> as add.s -o add
machine-kernel-shell> add
machine-kernel-shell> adb add

# performs add program
# can debug using Aruel DeBugger on add
```

- Basic kernel/machine
	- Uniprogramming
	- Basic I/O
		- Uses C's I/O fxns
	- Basic syscall interrupt system

- Debugger
	- 