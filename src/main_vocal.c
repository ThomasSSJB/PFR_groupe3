#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void traiter_commande(void);

int main()
{
    printf("=== MODE VOCAL ROBOT ===\n");

    while (1)
    {
        printf("\n[WAIT] attente commande vocale...\n");

        /*
         * Attend que commande.txt contienne quelque chose
         */
        FILE *f = fopen("data/commande.txt", "r");

        if (f)
        {
            char buffer[512];

            if (fgets(buffer, sizeof(buffer), f))
            {
                if (buffer[0] != '\n' && buffer[0] != '\0')
                {
                    printf("[VOCAL] Commande recue : %s\n", buffer);

                    /*
                     * Transforme en action.txt
                     */
                    traiter_commande();

                    /*
                     * Lance le robot
                     */
                    system("python3 python/pilote_robot.py");

                    /*
                     * Nettoyage commande.txt
                     */
                    FILE *clean = fopen("data/commande.txt", "w");
                    if (clean)
                        fclose(clean);
                }
            }

            fclose(f);
        }

        usleep(500000);
    }

    return 0;
}