import os
import time

# Chemin vers le fichier commande (écrit par ton PC via SSH)
FICHIER_COMMANDE = "/home/pfr3/code/PFR_groupe3/data/commande.txt"

def lire_commande():
    """Attend que le PC écrive une commande dans commande.txt"""
    print(">>> En attente de commande vocale depuis le PC...")
    print(">>> Lance test_voix.py sur ton PC maintenant !")
    
    # Vider le fichier d'abord
    open(FICHIER_COMMANDE, "w").close()
    
    # Attendre que le PC écrive dedans (max 30 secondes)
    for _ in range(60):
        time.sleep(0.5)
        if os.path.exists(FICHIER_COMMANDE):
            with open(FICHIER_COMMANDE, "r", encoding="utf-8") as f:
                contenu = f.read().strip()
            if contenu:
                print(f"[VOCAL] Commande reçue : {contenu}")
                return contenu
    
    print("[VOCAL] Timeout : aucune commande reçue.")
    return ""

def main():
    lire_commande()

if __name__ == "__main__":
    main()
