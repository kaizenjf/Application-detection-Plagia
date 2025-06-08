#  Détection de Plagiat - Distance de Jaccard (C)

Ce projet implémente la **distance de Jaccard** en C pour comparer des fichiers texte et détecter leur similarité. Il peut être utilisé pour évaluer un potentiel plagiat entre documents.

##  Fonctionnalités

- Calcul de la distance de Jaccard entre plusieurs fichiers `.txt`
- Extraction des mots uniques et stockage via une **hashtable**
- Présentation en pourcentage de similarité
- Gestion mémoire propre et performances optimisées

##  Structure

- `jdis.c` : point d'entrée du programme
- `hashtable.*` : module pour stocker les mots et leur ID
- `holdall.*` : structure utilitaire pour gestion mémoire
- `makefile` : compilation automatique via `make`

##  Utilisation

```bash
make
./jdis fichier1.txt fichier2.txt fichier3.txt
