# =======================EN TÊTE=====================================
# FICHIER: test_voix.py
# AUTEUR: BACHAA Hajar
# RÔLE: Permet de tester la commande vocale
# ===================================================================

import speech_recognition as sr
import subprocess

IP_RASPBERRY = "172.20.10.8"

def main():
    r = sr.Recognizer()
    with sr.Microphone() as source:
        r.adjust_for_ambient_noise(source, duration=1)
        print(">>> Parle maintenant...")
        audio = r.listen(source, timeout=10)
    
    try:
        texte = r.recognize_google(audio, language="fr-FR")
        print("Entendu :", texte)
        cmd = f'ssh pfr3@{IP_RASPBERRY} "echo \'{texte}\' > /home/pfr3/code/PFR_groupe3/data/commande.txt"'
        subprocess.run(cmd, shell=True)
        print("Commande envoyée !")
    except sr.UnknownValueError:
        print(" Je n'ai pas compris, réessaie !")
    except sr.RequestError as e:
        print(f" Erreur Google : {e}")

if __name__ == "__main__":
    while True:
        main()