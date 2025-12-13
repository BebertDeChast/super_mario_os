# NES Level Collision Generator

Ce script Python permet de transformer une image de niveau de jeu (type NES / Super Mario Bros) en un tableau de données **C++** utilisable dans un moteur de jeu.

Il utilise une approche hybride combinant la **détection de couleur** (pour le fond) et le **Pattern Matching** (comparaison d'images) pour identifier précisément les obstacles, les pièces et les blocs spéciaux à partir d'un fichier de "Tileset".

## 📋 Fonctionnalités

  * **Conversion Image vers Code :** Génère un fichier header `.h` contenant un tableau 1D représentant la carte.
  * **Détection par Tuiles (Tiles) :** Analyse le niveau par blocs de 16x16 pixels.
  * **Reconnaissance d'Assets :** Identifie des objets complexes (Tuyaux, Pièces, Blocs "?", Sol) en les comparant à une planche de sprites (Tileset).
  * **Visualisation Debug :** Affiche une fenêtre interactive (Matplotlib) superposant la grille de collision détectée sur l'image originale.
  * **Mode Vérification :** Permet de valider que les assets sont bien découpés avant de lancer l'analyse complète.


## 🚀 Installation et Configuration

1.  Placez votre script (`level_generator.py`) dans le dossier de votre projet.
2.  Assurez-vous d'avoir vos deux images sources (format PNG recommandé) :
      * L'image du niveau complet (ex: `World 1-1.png`).
      * L'image du Tileset contenant les sprites (ex: `Tileset.png`).
3.  Ouvrez le script et modifiez les chemins en haut du fichier :

<!-- end list -->

```python
LEVEL_IMAGE_PATH = r"chemin/vers/votre/niveau.png"
TILESET_IMAGE_PATH = r"chemin/vers/votre/tileset.png"
```

## 🛠️ Comment définir les Assets (Tileset)

C'est l'étape la plus importante. Le script doit savoir où trouver les modèles (le "moule") des objets dans votre image `Tileset.png`.

1.  Ouvrez `Tileset.png` dans un éditeur d'image (Paint, Gimp, Photoshop).
2.  Relevez les coordonnées **X, Y** (en pixels) du coin **supérieur gauche** de chaque bloc de 16x16 que vous voulez détecter.
3.  Remplissez le dictionnaire `ASSETS_CONFIG` dans le script :

<!-- end list -->

```python
ASSETS_CONFIG = {
    # Format : "Nom": { "x": pixel_x, "y": pixel_y, "type": ID_TYPE }
    "Sol_Marron":   {"x": 0,   "y": 16,  "type": 1},
    "Piece_Or":     {"x": 298, "y": 95,  "type": 2},
    "Bloc_Interro": {"x": 298, "y": 78,  "type": 3},
}
```

### Les Types d'Interaction (IDs)

Par défaut, le script utilise ces IDs pour le tableau C++ :

  * `0` : **PASSTHROUGH** (Air / Vide / Traversable)
  * `1` : **NOT\_PASSTHROUGH** (Mur / Sol / Obstacle)
  * `2` : **COIN** (Pièce à ramasser)
  * `3` : **QUESTION\_BOX** (Bloc mystère interactif)

## 🎮 Utilisation

### Étape 1 : Vérifier les découpes (Important)

Avant d'analyser tout le niveau, vérifiez que vos coordonnées X/Y sont bonnes.

1.  Dans le script, réglez : `CHECK_ASSETS_MODE = True`
2.  Lancez le script : `python level_generator.py`
3.  Une fenêtre s'ouvre montrant chaque petit bloc découpé. S'ils sont coupés en deux ou mal cadrés, ajustez les X/Y dans `ASSETS_CONFIG`.

### Étape 2 : Générer le niveau

1.  Réglez : `CHECK_ASSETS_MODE = False`
2.  Lancez le script.
3.  Une fenêtre de **Debug** s'ouvre :
      * 🟥 **Rouge** : Obstacles solides (Type 1)
      * 🟨 **Jaune** : Pièces (Type 2)
      * 🟪 **Magenta** : Blocs Interactifs (Type 3)
      * (Rien) : Vide (Type 0)
4.  Le fichier `LevelCollision.h` est créé à côté du script.

## 📄 Format de sortie (C++)

Le fichier généré ressemble à ceci :

```cpp
#ifndef LEVEL_COLLISION_H
#define LEVEL_COLLISION_H

const int MAP_WIDTH = 212;  // Largeur en tuiles
const int MAP_HEIGHT = 15;  // Hauteur en tuiles

// Tableau 1D (Row-major order)
const unsigned char collision_map[] = {
    0, 0, 0, 2, 0, ... // 0=Air, 2=Piece
    1, 1, 1, 1, 1, ... // 1=Sol
};

#endif
```

Pour lire ce tableau dans votre code C++ :

```cpp
// Pour obtenir la tuile à la position (x, y)
int tileID = collision_map[y * MAP_WIDTH + x];
```

## ⚠️ Dépannage

  * **Problème :** Les tuyaux ou le sol ne sont pas détectés (restent vides).
      * **Solution :** Vérifiez que `MATCH_THRESHOLD` n'est pas trop bas. Essayez de le monter à `150` ou `200`. Assurez-vous aussi que l'image du tileset correspond *exactement* aux graphismes du niveau (mêmes couleurs).
  * **Problème :** Tout est rouge dans le debug.
      * **Solution :** Vérifiez que vos couleurs de fond (`BACKGROUND_COLORS`) sont bien définies pour exclure le ciel et les décors non-solides.
  * **Problème :** Le script plante avec une erreur de taille.
      * **Solution :** Vérifiez que les coordonnées X/Y dans `ASSETS_CONFIG` ne sortent pas des dimensions de l'image `Tileset.png`.