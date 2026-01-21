bin/prog_principal.out: lib/main.o lib/commande_vocale.o
	gcc lib/main.o lib/commande_vocale.o -o bin/prog_principal.out -lm


lib/main.o: src/main.c
	gcc -c src/main.c -o lib/main.o -Wall -Iinclude -lm


lib/commande_vocale.o: src/commande_vocale.c include/commande_vocale.h
	gcc -c src/commande_vocale.c -o lib/commande_vocale.o -Wall -Iinclude -lm


bin/prog_image.out: lib/main_image.o lib/image.o lib/forme.o lib/objet.o lib/config.o
	gcc lib/main_image.o lib/image.o lib/forme.o lib/objet.o lib/config.o -o bin/prog_image.out -lm

lib/main_image.o: src/main_image.c
	gcc -c src/main_image.c -o lib/main_image.o -Wall -lm

lib/image.o: src/image.c include/image.h
	gcc -c src/image.c -o lib/image.o -Wall -lm

lib/forme.o: src/forme.c include/forme.h
	gcc -c src/forme.c -o lib/forme.o -Wall -lm

lib/objet.o: src/objet.c include/objet.h
	gcc -c src/objet.c -o lib/objet.o -Wall -lm

lib/config.o: src/config.c include/config.h
	gcc -c src/config.c -o lib/config.o -Wall -lm

clean:
	rm -rf lib/*.o