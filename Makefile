# ================ EN TÊTE ================
# FICHIER: Makefile
# AUTEURS: GRELET Thomas, YAHYAOUI Nidal
# RÔLE: Compiler l'ensemble des fichiers compilables
# MODIF: Ajout de -lpthread pour le thread de surveillance web

# ================= CONFIG =================
CC      = gcc
CFLAGS  = -Wall -Wextra -Iinclude -lm -lpthread
LDFLAGS = -lm -lpthread

# ================= EXECUTION =================
run: bin/prog_principal.out
	@./bin/prog_principal.out

image: bin/prog_image.out
	@./bin/prog_image.out

# ================= LINKING =================
bin/prog_principal.out: lib/main.o lib/commande_vocale.o lib/image.o lib/objet.o lib/config.o lib/utils.o
	@$(CC) lib/main.o lib/commande_vocale.o lib/image.o lib/objet.o lib/config.o lib/utils.o -o bin/prog_principal.out $(CFLAGS)
	@echo "Exécutable généré : bin/prog_principal.out"

bin/prog_image.out: lib/main_image.o lib/image.o lib/objet.o lib/config.o lib/utils.o
	@$(CC) lib/main_image.o lib/image.o lib/objet.o lib/config.o lib/utils.o -o bin/prog_image.out $(CFLAGS)
	@echo "Exécutable généré : bin/prog_image.out"

# ================= COMPILATION =================
lib/main.o: src/main.c
	@$(CC) -c src/main.c -o lib/main.o $(CFLAGS)
	@echo "Compilation : src/main.c -> lib/main.o"

lib/main_image.o: src/main_image.c
	@$(CC) -c src/main_image.c -o lib/main_image.o $(CFLAGS)
	@echo "Compilation : src/main_image.c -> lib/main_image.o"

lib/commande_vocale.o: src/commande_vocale.c
	@$(CC) -c src/commande_vocale.c -o lib/commande_vocale.o $(CFLAGS)
	@echo "Compilation : src/commande_vocale.c -> lib/commande_vocale.o"

lib/image.o: src/image.c
	@$(CC) -c src/image.c -o lib/image.o $(CFLAGS)
	@echo "Compilation : src/image.c -> lib/image.o"

lib/objet.o: src/objet.c
	@$(CC) -c src/objet.c -o lib/objet.o $(CFLAGS)
	@echo "Compilation : src/objet.c -> lib/objet.o"

lib/config.o: src/config.c
	@$(CC) -c src/config.c -o lib/config.o $(CFLAGS)
	@echo "Compilation : src/config.c -> lib/config.o"

lib/utils.o: src/utils.c
	@$(CC) -c src/utils.c -o lib/utils.o $(CFLAGS)
	@echo "Compilation : src/utils.c -> lib/utils.o"

# ================= NETTOYAGE =================
clean:
	@if ls lib/*.o 1> /dev/null 2>&1; then rm lib/*.o; echo "lib nettoyé"; else echo "Rien à supprimer"; fi

fclean: clean
	@rm -f bin/*.out
	@echo "Exécutables supprimés"

re: fclean run
