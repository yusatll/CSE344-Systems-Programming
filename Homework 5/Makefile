me: bank.c client.c
	gcc bank.c -o bank
	gcc client.c -o client
	
debug: bank.c 
	rm -f clientFifo bankFifo
	gcc -W -Wall -Wextra -pedantic-errors -g bank.c -o bank
	gcc -W -Wall -Wextra -pedantic-errors -g client.c -o client
	
error: bank.c
	gcc -W -Wall -Wextra -ansi -pedantic-errors -Werror -g bank.c -o bank
	
run:
	./bank
	
log:
	cat bank.log

clean:
	rm -f bank client
