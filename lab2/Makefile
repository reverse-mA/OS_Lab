maincpp:main.cpp my_print.asm
	nasm -f elf32 my_print.asm 
	g++ -m32 -o hello main.cpp my_print.o
	./hello
clear:
	rm -f my_print.o
	rm -f main.o
	rm -f hello
