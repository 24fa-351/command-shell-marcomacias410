myshell: myshell.c func_commands.c parse.c 
	gcc -o myshell myshell.c func_commands.c parse.c 

clean: 
	rm myshell	