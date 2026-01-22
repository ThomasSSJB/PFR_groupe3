import speech_recognition as sr
from gtts import gTTS

FICHIER_COMMANDE = "commande.txt"
LANGUE = "fr-FR"

recognizer = sr.Recognizer()
microphone = sr.Microphone()

def normaliser_transcription(texte):
    """
    Nettoyage simple de la transcription
    conforme au PFR
    """
    texte = texte.lower().strip()
    tokens = texte.split()
    print("Tokens :", tokens)
    return " ".join(tokens)   # STRING pour le C

def envoyer_texte_au_C(texte):
    """
    Envoi de la commande vers le module C
    """
    with open(FICHIER_COMMANDE, "w", encoding="utf-8") as f:
        f.write(texte)

def text_to_speech(message):
    """
    Synthèse vocale (optionnelle PFR)
    """
    tts = gTTS(message, lang="fr")
    tts.save("reponse.mp3")

def main():
    with microphone as source:
        print("Speak!")
        audio = recognizer.listen(source)
        print("End!")

    try:
        transcription = recognizer.recognize_google(audio, language=LANGUE)
        print("Vous avez dit :", transcription)
    except sr.UnknownValueError:
        print("Erreur : parole non reconnue")
        return
    except sr.RequestError:
        print("Erreur : service indisponible")
        return

    commande = normaliser_transcription(transcription)
    envoyer_texte_au_C(commande)

    text_to_speech("Commande envoyée.")


if __name__ == "__main__":
    main()
