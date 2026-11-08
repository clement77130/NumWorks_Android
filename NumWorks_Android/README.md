# NumWorks Android OS (EADK, C)

Lanceur d'applications inspire d'Android / One UI pour NumWorks N0120 (320x240, touches uniquement).
Sortie : `output/NumWorks_Android.nwa`

## Etat des tests (honnete)
- Verifie : syntaxe C99 avec un GCC hote et un `eadk.h` factice ; logique de la calculatrice, formatage des nombres, horloge, jour de la semaine.
- **NON verifie** : compilation ARM avec le vrai SDK, format NWA, affichage, touches, memoire, comportement sur Epsilon 26.4.0 / N0120.
  Si le compilateur signale un nom inconnu (ex. `eadk_key_ans`), corrigez-le d'apres `eadk.h` du SDK.

## Contenu reel / a developper
| Appli | Etat |
|---|---|
| Calculatrice | reelle : + - x / ( ) ^ (exposant entier), Ans, 10 chiffres |
| Probabilites | reelle : n!, C(n,k), A(n,k), loi binomiale |
| Suites | reelle : arithmetique / geometrique, u(n), somme |
| Parametres | reels : theme (3), accent (5), barre nav, repetition, heure/date, infos |
| Graphe, Fonctions, Tableur, Stats, Regressions | **modules a developper** (ecran explicite) |
| Python | impossible via l'EADK (appli Epsilon d'origine non lancable) |

Limites : pas d'horloge materielle accessible (heure a regler dans Parametres a chaque lancement) ; reglages non sauvegardes ; le bouton Home est gere par Epsilon (il quitte l'appli).

## Compilation sous Windows
1. Installer **Node.js LTS**, **Arm GNU Toolchain** (`arm-none-eabi-gcc` dans le PATH), **Python 3** + `pip install pillow`.
2. Installer **MSYS2** (https://www.msys2.org), ouvrir le shell MSYS2, puis `pacman -S make`. Ajouter au PATH de ce shell le dossier `bin` de la toolchain ARM et Node.js.
3. Mettre votre photo dans `assets/fond_ecran.jpg`, puis :
```
python tools/prepare_assets.py
make clean
make build
```
4. Le fichier est dans `output/NumWorks_Android.nwa`.

## Installation sur la calculatrice
Brancher la N0120 en USB, ouvrir https://my.numworks.com/apps dans Chrome ou Edge, envoyer le `.nwa`. Ou : `make run`.
Si le fond est trop lourd pour la flash : baisser la resolution dans `prepare_assets.py` ou supprimer le jpg (degrade integre).

## Architecture
`include/` + `src/` : `main.c` (accueil), `interface.c` (dessin, icones), `navigation.c` (touches), `applications.c` (applis + table `g_apps`), `ressources.c` (themes, horloge, utilitaires).
Ajouter une appli : fonction + entree dans `g_apps` + `APP_COUNT` + glyphe dans `ui_icon()`.
Jeux : fichiers `.nwa` independants, installes separement. L'EADK ne permet pas a ce lanceur de les demarrer ; ils se lancent depuis le menu Epsilon.
