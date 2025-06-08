#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "hashtable.h"



#define WORD_LENGTH_MAX 31
#define MAX_WORDS 100000
#define MAX_FILES 64



void print_help(const char *progname);

static size_t str_hashfun(const void *s);

char *safe_strdup(const char *str);

int main(int argc, char *argv[]) {
    // ici on verifie que aumoins deux argument on ete passé  avant de filter les option et fichier 
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file1> <file2> ... <fileN>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Ce block va permettre de gererer les options , dans le cas present uniquement "--help ou -?" est defini
    // si l'utilisateur cherche d'autre option on lui indique que il n'y en pas d'autre
    int arg_start = 1;
    for (; arg_start < argc; arg_start++) {
        if (strcmp(argv[arg_start], "--help") == 0 || strcmp(argv[arg_start], "-?") == 0) {
            print_help(argv[0]);
            return EXIT_SUCCESS;
        } else if (argv[arg_start][0] == '-') {
            fprintf(stderr, "Option non supportée : %s\n", argv[arg_start]);
            return EXIT_FAILURE;
        } else {
            break;
        }
    }
    //On verifie que deux fichier .txt ont été fournis 
    int file_count = argc - arg_start;
    if (file_count < 2) {
        fprintf(stderr, "Veuillez fournir au moins deux fichiers texte.\n");
        return EXIT_FAILURE;
    }
    //On verifie que l'utilisateur n'a pas fournis trop de fichier
    if (file_count > MAX_FILES) {
        fprintf(stderr, "Trop de fichiers. Maximum supporté : %d\n", MAX_FILES);
        return EXIT_FAILURE;
    }
    // On va ensuite cre une nouvelle table de table de hachage et verifier si tout ce passe bien
    hashtable *word_to_id = hashtable_empty((int (*)(const void *, const void *))strcmp, str_hashfun);
    if (!word_to_id) {
        fprintf(stderr, "Erreur lors de la création de la hashtable\n");
        return EXIT_FAILURE;
    }

    // On va ensuite cree word_list: un tableau de pointeur vers chaine de caractere 
    // chaque mot unique rencontrer dans les fichier sera stoker dans word_list[id] -> "mot1"
    char **word_list = malloc(MAX_WORDS * sizeof(char *));
    // On va ensuite cree presence: un tableau 2 demension 
    // un pointeur vers une liste de int presence[id][fichier] vaudra 1 si l'ID "id" est present dans le fichier donner 
    int **presence = malloc(MAX_WORDS * sizeof(int *));

    // Verification que il n'y as pas eu derreur lors de l'allocation memoire 
    if (!word_list || !presence) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        return EXIT_FAILURE;
    }
    // On va initialiser chaque ligne du tableau a 0 avec comme MAX_FILES de cellule
    // chaque ligne va representer un mot par son id et contient un tableau doccurence par fichier
    for (int i = 0; i < MAX_WORDS; i++) {
        presence[i] = calloc(MAX_FILES, sizeof(int));
        if (!presence[i]) {
            fprintf(stderr, "Erreur d'allocation mémoire pour presence[%d]\n", i);
            return EXIT_FAILURE;
        }
    }

    // creation du compteur de mot unique word_count
    int word_count = 0;
    // Boucle principale :on parcourt tous les fichiers donnés en argument
    for (int i = 0; i < file_count; i++) {
        // Ouverture du fichier i
        FILE *file = fopen(argv[arg_start + i], "r");
        // verification des erreur dans le cas ou l'ouverture a echouer 
        if (!file) {
            fprintf(stderr, "Erreur lors de l'ouverture de %s\n", argv[arg_start + i]);
            return EXIT_FAILURE;
        }

        char word[WORD_LENGTH_MAX + 1];
        //lecture de chaque mot du ficheir 
        while (fscanf(file, "%31s", word) == 1) {
            // recherche du mot dans la table de hachage
            int *id_ptr = hashtable_search(word_to_id, word);
            int id;
 
            if (!id_ptr) {
                // on verifie que on a pas depasser la limite 
                if (word_count >= MAX_WORDS) {
                    fprintf(stderr, "Fichier trop complexe (> %d mots uniques).\n", MAX_WORDS);
                    return EXIT_FAILURE;
                }
                //  on va donnen un nouvel id au mot actuelle   word_count  ; 
                id = word_count;
                // on va mettre le nouveau mot   dans   word_list dans la position qui correspond 
                word_list[word_count] = safe_strdup(word);
                // on va   cree un pointeur qui va pointer vers l'id stoker precedement puis le mettre dans la table de hachage
                int *new_id = malloc(sizeof(int));
                *new_id = id;
                hashtable_add(word_to_id, word_list[word_count], new_id);
                // ensuite on increment le compteur de mot unique word_count
                word_count++;
            } else {
                // si le mot existe deja on recupere son id
                id = *id_ptr;
            }
            //On va noter la presente du mot id dans le fichier i 
            presence[id][i] = 1;
        }
        fclose(file);
    }
    // ici on va appliquer jdis  entre toute les pair de fichier en utilisant les deux permeir boucle i et j 
    // donc fichier1 vs fichier2,fichier3,fichierN , puis fichier2 vs , fichier3,fichier4,fichierN 
    for (int i = 0; i < file_count; i++) {
        for (int j = i + 1; j < file_count; j++) {
            int inter = 0; // creation de inter pour intersection  pour les mots en commun 
            int uni = 0;  // creation de uni pour union pour un mot present dans aumoins 1 des deux fichier

            // cette boucle elle va parcourir tout les mot unique trouver 
            for (int k = 0; k < word_count; k++) {
                //pour un id k [0,1,2,3] et un un fichier i et fichier j deux fichier differnet  
                int pi = presence[k][i];
                int pj = presence[k][j];
                if (pi || pj) uni++; // si l'id est present dans aumoins un des deux fichier  
                if (pi && pj) inter++; // si l'id est present dans obligatoirement les deux fichier  
            }
            // application de la formule  pour le calcul  de  la distance de Jaccard
            double distance = 1.0 - ((double)inter / uni);
            // on affiche de maniere conforme au projet les resultat pour chaque fichier 
            printf("%.4f\t%s\t%s\n", distance, argv[arg_start + i], argv[arg_start + j]);
        }
    }
    
    // boucle pour liberer tout les espace memoire alouée
    for (int i = 0; i < word_count; i++) {
        free(word_list[i]);
        free(presence[i]);
        int *id_ptr = hashtable_search(word_to_id, word_list[i]);
        if (id_ptr != NULL) {
            free(id_ptr);
        }
    }
    free(word_list);
    free(presence);
    hashtable_dispose(&word_to_id);

    return EXIT_SUCCESS;
}


char *safe_strdup(const char *str) {
    char *dup = strdup(str);
    if (!dup) {
        fprintf(stderr, "Échec de l'allocation de mémoire pour la duplication de la  chaîne\n");
        exit(EXIT_FAILURE);
    }
    return dup;
}

void print_help(const char *progname) {
    printf("Utilisation: %s [--help|-?] fichier1.txt fichier2.txt [...fichierN.txt]\n", progname);
    printf("Calculez la distance de Jaccard entre les ensembles de mots dans chaque paire de fichiers.\n");
    printf("Options:\n");
    printf("  --help, -?          affiche ce message d'aide et quitte le programme \n");
}

size_t str_hashfun(const void *s) {
    size_t h = 0;
    for (const unsigned char *p = (const unsigned char *) s; *p != '\0'; ++p) {
        h = 37 * h + *p;
    }
    return h;
}
