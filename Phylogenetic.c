#include "HierarchicalClustering.h"
#include "Phylogenetic.h"

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static double phyloDistFn(const char *o1, const char *o2, void *params);


/*
L’implémentation de Phylogenetic.c vous demandera d’écrire le code pour lire le fichier
contenant les séquences et de préparer l’appel à hclustBuildTree. Pour cette implémentation, inspirez vous et adaptez le fichier main-features.c qui vous est fourni.
*/
/*calcule la distance d(o,o′) décrite dans l’équation (1) entre les deux séquences d’ADN données en argument. Si ces deux séquences n’ont pas la même longueur, on tronquera à droite la plus longue à la longueur
de la plus courte.
*/
double phyloDNADistance(char *dna1, char *dna2){

            int length1= strlen(dna1);
            int length2= strlen(dna2);
            int minlength = (length1>length2 )? length2 : length1;

            int transitions=0;
            int transversions =0;
            for(int i=0; i< minlength; i++){ // on parcourt seulement la longueur du plus petit segment de gauche a droite (le plus long est tronque a droite)
                        char c1 = dna1[i];
                        char c2 = dna2[i];
                        if(c1 != c2) // si les 2 lettres sont différentes
                        {
                                    if((c1 =='A'&& c2 =="G")|| (c1 =='G' && c2 =='A')||(c1 =='C' && c2 =='T')||(c1 =='T' && c2 =='C')){
                                                transitions ++:
                                    }
                                    else{
                                                tansversions ++;
                                    }
                        }
            }
            // P poproportion de transversions, Q de transition
            double P = (double) transversions/minlength; 
            double Q = (double) transitions/minlength;
            double dist = (-(0.5*log(1-2*P -Q)) -(0.25)*log(1-2*Q)); 
            return dist;
}


static double phyloDistFn(const char *o1, const char *o2, void *params) {
    Dict *dict = (Dict *)params;
    char *s1 = (char *)dictSearch(dict, o1);
    char *s2 = (char *)dictSearch(dict, o2);
    return phyloDNADistance(s1, s2);
}

//Hclust *phyloTreeCreate(char *filename) : renvoie le clustering hiérarchique obtenu à partir des espèces dans le fichier filename
Hclust *phyloTreeCreate(char *filename){

            char buffer[MAXLINELENGTH];
            FILE *fp = fopen (filename, "r");

            if (!fgets(buffer, MAXLINELENGTH, fp)){
                        fprintf(stderr, "phyloTreeCreate: the file is empty.\n");
                        exit(EXIT_FAILURE);
            }

            List *names = llCreateEmpty();
            Dict *dicfeatures = dictCreate(1000);

            while (fgets(buffer, MAXLINELENGTH, fp)){
            
                        int lenstr = strlen(buffer) - 1;
                        buffer[lenstr] = '\0'; // replace \n with \0

                        // Extract species name
                        int i = 0;
                        while (buffer[i] != ',')
                                    i++;
                        buffer[i] = '\0';

                        char *objectName = malloc((i + 1) * sizeof(char));
                        char *dnaSequence = malloc((lenstr-i)*sizeof(char));
                        strcpy(dnaSequence, buffer+i+1);
                        strcpy(objectName, buffer);

                        llInsertLast(names, objectName);
                        dictInsert(dicfeatures,objectName, dnaSequence);
                        int pos = 0;
                        i++;

                        }
            
            Hclust *hc = hclustBuildTree(names, phyloDistFn, dicfeatures);

            dictFreeValues(dicfeatures, free);
            llFreeData(names);
            fclose(fp);
            return hc;
}


