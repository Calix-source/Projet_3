#ifndef HCLUS_H
#define HCLUS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Dict.h"
#include "BTree.h"
#include "HierarchicalClustering.h"
#include "LinkedList.h"

struct Hclust_t
{
  BTree *dendro_tree; //pointeur vers la rac de l'arbre 
  int nb_leaves;
  Dict *dict; 
};
typedef struct Hclust_t Hclust;

struct PairDist_t
{
    char *o1; 
    char *o2; 
    double distance; 
};
typedef struct PairDist_t PairDist;

Hclust *hclustBuildTree(List *objects, double (*distFn)(const char *, const char *, void *), void *distFnParams);
void hclustFree(Hclust *hc);
static void hclustGetLeaves(BTree *tree, BTNode *n, List *clusterlist);
int hclustDepthAux(BTree *tree, BTNode *n);
int hclustDepth(Hclust *hc);
int hclustNbLeaves(Hclust *hc);
void hclustPrintTree(FILE *fp, Hclust *hc);
static void GetClustersDistAux(List *clusters, BTree *tree, BTNode *n, double distanceThreshold);
List *hclustGetClustersDist(Hclust *hc, double distanceThreshold);
List *GetClustersAux(Hclust *hc, int k);
List *hclustGetClustersK(Hclust *hc, int k);
BTree *hclustGetTree(Hclust *hc) ;
static void update_dict_entry(void *data, void *fparams);
void printNode(FILE *fp, BTree *tree, BTNode *node,double dist_parent_fus);

// Fonction auxi pour mettre à jour le dictionnaire des feuilles 
static void update_dict_entry(void *data, void *fparams)
{  
    Hclust *params = (Hclust *)fparams;
    dictInsert(params->dict, (char *)data, params-> dendro_tree); //met a jour l'appartenance

}
static int compare_distance(void *a, void *b){ // on en a besoin pour trier nos distances apres
    PairDist *pA = (PairDist *)a;
    PairDist *pB = (PairDist *)b;
    double distA = pA->distance; 
    double distB = pB->distance;
    if (distA < distB) 
        return -1;
    if (distA > distB) 
        return 1;
    return 0;
}

Hclust *hclustBuildTree(List *objects, double (*distFn)(const char *, const char *, void *), void *distFnParams)
{
    if (objects == NULL || llLength(objects) == 0) return NULL;

    Hclust *clust = (Hclust *)malloc(sizeof(Hclust)); // alloc mémoire
    if (clust==NULL)
        return NULL; // si l'alloc de la mémoire échoue 
    clust->dendro_tree = NULL ; //ini nos champs 
    clust->nb_leaves = 0; 
    size_t ini_dict_size = llLength(objects) * 2; // on ini une taille 
    Dict *dict = dictCreate(ini_dict_size); // on crée notre dictionnaire
  
    if (dict==NULL) // notre alloc échoue
    {
        free(clust); 
        return NULL; // si l'alloc de la mémoire échoue 
    }
    BTree *T_fin = NULL; // stocker dernier arbre fusio 
    Node *current_node = llHead(objects); // premier noeud de la liste d'objects 

    while (current_node!=NULL) 
    {
        char *o1 = (char *)llData(current_node); 
        BTree *T1 = btCreate(); // T1 est un pointeur vers notre cluster 
        if (T1==NULL)
            {
                dictFree(dict); 
                free(clust); 
                return NULL; 
            }
        btCreateRoot(T1, o1); //o1 va dans notre arbre
        clust->nb_leaves++; 
        dictInsert(dict, o1, T1); // on ini le dictionnaire aussi
        T_fin = T1; 
        current_node=llNext(current_node);
    }
    List *distances = llCreateEmpty(); //pour stocker toutes les paires possibles
    Node *pointeur1 = llHead(objects);

    while (pointeur1!=NULL)
    {
        const char *obj1 = (const char *)llData(pointeur1); // recup le nom de l'ojt 
        Node *pointeur2 = llNext(pointeur1);
    
        while ( pointeur2 != NULL) 
        {
            const char *obj2 = (const char *)llData(pointeur2); // recup le nom de l'ojt
            PairDist *pair = malloc(sizeof(PairDist));
            if(pair == NULL ) 
            {
                llFreeData(distances);
                dictFree(dict); 
                free(clust);
                return NULL; 
            } 
            double dist = distFn(obj1, obj2, distFnParams);
            pair->o1 = (char*)obj1;
            pair->o2 = (char*)obj2; 
            pair->distance = dist; 
      
            llInsertLast(distances, pair);
            pointeur2 = llNext(pointeur2); //avance le pointeur
        }
        pointeur1= llNext(pointeur1);
    }
    llSort(distances, compare_distance); // on trie nos distances
  
    int nbr_clust_rest = clust->nb_leaves; // nbr de clusters qu'il nous reste 

    while(nbr_clust_rest >1)// on fusionne pour qu'il nous reste un seul cluster 
    {
        PairDist *best_pair = llPopFirst(distances); // on trouve un pointeur vers la struct avec la plus petite distance
        if (best_pair == NULL) 
            break;
        BTree *T1 = (BTree *) dictSearch(dict, best_pair->o1); // on reg si nos objts ont deja fusio 
        BTree *T2 = (BTree *)dictSearch(dict, best_pair->o2);
        // si les objts ont déjà été fusio indirect dans un cluster plus grand
        if( T1==T2 || T1==NULL || T2== NULL)
        {
            free(best_pair);
            continue; 
        }
        else // on a trouvé les deux clusters les plus proches et distincts
        {
            double *dist_fus = malloc (sizeof(double*));
            if (dist_fus==NULL)
                return NULL; 
            *dist_fus =best_pair-> distance;

            Hclust params;
            params.dendro_tree =T1;
            params.dict = dict; 
            btMapLeaves(T2, btRoot(T2), update_dict_entry, &params); // mise à jour pour que les feuilles de T2 point vers T1 dans le dict
            btMergeTrees(T1, T2, dist_fus);
  
            T_fin = T1;   
            free(best_pair);
            nbr_clust_rest --; 
        }            
 
    }
    clust->dendro_tree = T_fin; 
    clust->dict = dict;
    llFreeData(distances); 
    return clust;   
}

void hclustFree(Hclust *hc){

    if(hc == NULL){
        return;
    }

    btFree(hc->dendro_tree);

    dictFree(hc->dict);
    free(hc);
    return;
}

static void hclustGetLeaves(BTree *tree, BTNode *n, List *clusterlist){
    if (n == NULL){
        return;
    }
    
    if (btIsExternal(tree, n)){ // si le noeud est une feuille
        llInsertLast(clusterlist, btGetData(tree, n)); // ajout de l objet a la liste
        return;
    } 
    
    // recursion pour les nœuds internes
    hclustGetLeaves(tree, btLeft(tree, n), clusterlist);
    hclustGetLeaves(tree, btRight(tree, n), clusterlist);
}

int hclustDepthAux(BTree *tree, BTNode *n){
    if(n == NULL){
        return 0;
    }

    if (btIsExternal(tree, n)){
        return 0; // car on veut le nombre d arretes et pas de noeuds
    }

    int leftDepth = hclustDepthAux(tree, btLeft(tree, n)); 
    int rightDepth = hclustDepthAux(tree, btRight(tree, n));
    return 1 + (leftDepth > rightDepth ? leftDepth : rightDepth); //on ajoute 1 pour le noeud courant a la profondeur du sous arbre le + profond

}

int hclustDepth(Hclust *hc){
    if (hc == NULL || hc->dendro_tree == NULL)
    {
        return 0;
    }
    return hclustDepthAux(hc->dendro_tree, btRoot(hc->dendro_tree));

}
int hclustNbLeaves(Hclust *hc){
    if (hc == NULL)
    {
        return 0;
    }
    return hc->nb_leaves;
}
void printNode(FILE *fp, BTree *tree, BTNode *node, double dist_parent_fus){
    

   
    double dist_noeud_fus; 
    double length; 

    if (btIsExternal(tree, node)) // ca veut dire qu'on a une feuille 
    {
        dist_noeud_fus =0.0;
        length = dist_parent_fus ; // calcule la long de la branche
        char *obj_name = (char*)btGetData(tree, node); 

        fprintf(fp, "%s:%f", obj_name, length);
    }
        
    else // cas récursif de base pour tous les autres noeuds ( càd s'il est à l'intérieur)
    {
        dist_noeud_fus = *(double *)btGetData(tree, node);
        length = dist_parent_fus - dist_noeud_fus;
    
        fprintf(fp, "(");
        BTNode *left = btLeft(tree, node);
        BTNode *right = btRight(tree, node);
        if(left != NULL){
            printNode(fp, tree, left, dist_noeud_fus);
        }
        fprintf(fp, ",");
        if(right != NULL){
            printNode(fp, tree, right, dist_noeud_fus);
            
        }

        fprintf(fp, "):%f", length);

    }
    
   return ; 
}

void hclustPrintTree(FILE *fp, Hclust *hc)
{
    if (hc == NULL || hc->dendro_tree == NULL || fp == NULL){
        return;
    } 
    BTNode *root = btRoot(hc->dendro_tree);
    if (root == NULL){
        return;
    }
    if (btIsExternal(hc->dendro_tree, root)){
        double dist_root_fus =0.0;
        printNode(fp,hc->dendro_tree, root, dist_root_fus);
    
    }
    else{
        double dist_root_fus = *(double*)btGetData(hc->dendro_tree, root);
        printNode(fp, hc->dendro_tree, root, dist_root_fus);

    }
    fprintf(fp, ";\n");
    return;
}


static void GetClustersDistAux(List *clusters, BTree *tree, BTNode *n, double distanceThreshold){

    if(n == NULL){
        return;
    }

    if (btIsExternal(tree, n)){ // si n est une feuille
        List *clusterlist = llCreateEmpty(); // on cree une liste pour stocker le cluster
        llInsertLast(clusterlist, btGetData(tree, n)); //on insere l'objet a la fin de la liste
        llInsertLast(clusters, clusterlist);
        return;
    }
    double *distptr = (double *)btGetData(tree, n);
    double dist = *distptr;

    // si n est un noeud interne
    if( dist >distanceThreshold){ // la distance entre les clusters est superieure au seuil,
        List *clusterlist = llCreateEmpty();//
        hclustGetLeaves(tree, n, clusterlist);
        llInsertLast(clusters, clusterlist); //


    }

    else{ // le seuil n'est pas atteint, on continue la recherche dans les sous-arbres
        
        GetClustersDistAux(clusters, tree, btLeft(tree, n), distanceThreshold);
        GetClustersDistAux(clusters, tree, btRight(tree, n), distanceThreshold);
    }
}

List *hclustGetClustersDist(Hclust *hc, double distanceThreshold){ // i do

    if (hc == NULL || hc->dendro_tree == NULL){ 
        return llCreateEmpty();
    }

    List *resultclusters = llCreateEmpty();

    //un seul onjet
    if (hc->nb_leaves == 1){
        List *clusterlist = llCreateEmpty();
        llInsertLast(clusterlist, btGetData(hc->dendro_tree, btRoot(hc->dendro_tree)));
        llInsertLast(resultclusters, clusterlist);
        return resultclusters;
    }

    GetClustersDistAux(resultclusters, hc->dendro_tree, btRoot(hc->dendro_tree), distanceThreshold);
    return resultclusters;

} 


List *GetClustersAux(Hclust *hc, int k)
{
    (void) hc; 
    (void)k;
    return   NULL; 
}

List *hclustGetClustersK(Hclust *hc, int k)
{ 
    (void) hc; 
    (void)k;
    return NULL;
}


BTree *hclustGetTree(Hclust *hc)
{
    return hc->dendro_tree;
}

#endif


/*#ifndef HCLUS_H
#define HCLUS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Dict.h"
#include "BTree.h"
#include "HierarchicalClustering.h"
#include "LinkedList.h"

struct Hclust_t
{
  BTree *dendro_tree; //pointeur vers la rac de l'arbre 
  int nb_leaves;
  Dict *dict; 
};
typedef struct Hclust_t Hclust;

struct PairDist_t
{
    char *o1; 
    char *o2; 
    double distance; 
};
typedef struct PairDist_t PairDist;

Hclust *hclustBuildTree(List *objects, double (*distFn)(const char *, const char *, void *), void *distFnParams);
void hclustFree(Hclust *hc);
static void hclustGetLeaves(BTree *tree, BTNode *n, List *clusterlist);
int hclustDepthAux(BTree *tree, BTNode *n);
int hclustDepth(Hclust *hc);
int hclustNbLeaves(Hclust *hc);
void printNode(FILE *fp, BTree *tree, BTNode *node, double dist_parent_fus);
void hclustPrintTree(FILE *fp, Hclust *hc);
static void GetClustersDistAux(List *clusters, BTree *tree, BTNode *n, double distanceThreshold);
List *hclustGetClustersDist(Hclust *hc, double distanceThreshold);
List *GetClustersAux(Hclust *hc, int k);
List *hclustGetClustersK(Hclust *hc, int k);
BTree *hclustGetTree(Hclust *hc) ;
static void update_dict_entry(void *data, void *fparams);
void printNode(FILE *fp, BTNode *node,double dist_parent_fus);

// Fonction auxiliaire pour mettre à jour le dictionnaire des feuilles 
static void update_dict_entry(void *data, void *fparams){    // data est le nom de l'objet (char*), fparams est le dictionnaire et le nouvel arbre
    void **params = (void **)fparams;//tranfo point en tab
    Dict *d = (Dict *)params[0]; // dict est à l'indidce 0
    BTree *new_tree = (BTree *)params[1]; // nouv arbre est à l'indice 1 
    
    dictInsert(d, (char *)data, new_tree); // Met à jour l'appartenance 
}

static int compare_distance(void *a, void *b){ // on en a besoin pour trier nos distances apres
    PairDist *pA = (PairDist *)a;
    PairDist *pB = (PairDist *)b;
    double distA = pA->distance; 
    double distB = pB->distance;
    if (distA < distB) 
        return -1;
    if (distA > distB) 
        return 1;
    return 0;
}


Hclust *hclustBuildTree(List *obje
cts, double (*distFn)(const char *, const char *, void *), void *distFnParams){
    if (objects == NULL || llLength(objects) == 0) return NULL;

    Hclust *clust = (Hclust *)malloc(sizeof(Hclust)); // alloc mémoire
    if (clust==NULL)
        return NULL; // si l'alloc de la mémoire échoue 
    clust->dendro_tree = NULL ; //ini nos champs 
    clust->nb_leaves = 0; 
    size_t ini_dict_size = llLength(objects) * 2; // on ini une taille 
    Dict *dict = dictCreate(ini_dict_size); // on crée notre dictionnaire
  
    if (dict==NULL) // notre alloc échoue
    {
        free(clust); 
        return NULL; // si l'alloc de la mémoire échoue 
    }
    Node *current_node = llHead(objects); // premier noeud de la liste d'objects 

    while (current_node!=NULL) 
    {
        char *o1 = (char *)llData(current_node); 
        BTree *T1 = btCreate(); // T1 est un pointeur vers notre cluster 
        if (T1==NULL)
            {
                dictFree(dict); 
                free(clust); 
                return NULL; 
            }
        btCreateRoot(T1, o1); //o1 va dans notre arbre
        clust->nb_leaves++; 
        dictInsert(dict, o1, T1); // on ini le dictionnaire aussi
        current_node=llNext(current_node);
    }
    List *distances = llCreateEmpty(); //pour stocker toutes les paires possibles
    Node *pointeur1 = llHead(objects);

    while (pointeur1!=NULL)
    {
        const char *obj1 = (const char *)llData(pointeur1); // recup le nom de l'ojt 
        Node *pointeur2 = llNext(pointeur1);
    
        while ( pointeur2 != NULL) 
        {
            const char *obj2 = (const char *)llData(pointeur2); // recup le nom de l'ojt
            PairDist *pair = malloc(sizeof(PairDist));
            if(pair == NULL ) 
            {
                llFreeData(distances);
                dictFree(dict); 
                free(clust);
                return NULL; 
            } 
            double dist = distFn(obj1, obj2, distFnParams);
            pair->o1 = (char*)obj1;
            pair->o2 = (char*)obj2; 
            pair->distance = dist; 
      
            llInsertLast(distances, pair);
            pointeur2 = llNext(pointeur2); //avance le pointeur
        }
        pointeur1= llNext(pointeur1);
    }
    llSort(distances, compare_distance); // on trie nos distances
  
    int nbr_clust_rest = clust->nb_leaves; // nbr de clusters qu'il nous reste 
    BTree *T_fin = NULL; // stocker dernier arbre fusio 
    while(nbr_clust_rest >1)
    {
        PairDist *best_pair = llPopFirst(distances); // on trouve un pointeur vers la struct avec la plus petite distance
        if (best_pair == NULL) 
            break;
        BTree *T1 = (BTree *) dictSearch(dict, best_pair->o1); // on reg si nos objts ont deja fusio 
        BTree *T2 = (BTree *)dictSearch(dict, best_pair->o2);
        // si les objts ont déjà été fusio indirect dans un cluster plus grand
        if( T1==T2)
        {
            free(best_pair);
            continue; 
        }
        else // on a trouvé les deux clusters les plus proches et distincts
        {
            double *dist_fus = malloc (sizeof(double));
            *dist_fus =best_pair-> distance; 
            void *fparams[2]={(void*)dict, (void*)T1};
            btMapLeaves(T2, btRoot(T2), update_dict_entry, fparams);
            btMergeTrees(T1, T2, dist_fus);
  
            T_fin = T1;   
            free(best_pair);
            nbr_clust_rest --;  
        }
    }
    clust->dendro_tree = T_fin; 
    dictFree(dict); 
    llFreeData(distances); 
    return clust;   
}

void hclustFree(Hclust *hc){

    if(hc == NULL){
        return;
    }

    btFree(hc->dendro_tree);

    dictFree(hc->dict);
    free(hc);
    return;
}

static void hclustGetLeaves(BTree *tree, BTNode *n, List *clusterlist){
    if (n == NULL){
        return;
    }
    
    if (btIsExternal(tree, n)){ // si le noeud est une feuille
        llInsertLast(clusterlist, btGetData(tree, n)); // ajout de l objet a la liste
        return;
    } 
    
    // recursion pour les nœuds internes
    hclustGetLeaves(tree, btLeft(tree, n), clusterlist);
    hclustGetLeaves(tree, btRight(tree, n), clusterlist);
}

int hclustDepthAux(BTree *tree, BTNode *n){
    if(n == NULL){
        return 0;
    }

    if (btIsExternal(tree, n)){
        return 1;
    }

    int leftDepth = hclustDepthAux(tree, btLeft(tree, n)); 
    int rightDepth = hclustDepthAux(tree, btRight(tree, n));
    return 1 + (leftDepth > rightDepth ? leftDepth : rightDepth); //on ajoute 1 pour le noeud courant a la profondeur du sous arbre le + profond

}

int hclustDepth(Hclust *hc){
    if (hc == NULL || hc->dendro_tree == NULL)
    {
        return 0;
    }
    return hclustDepthAux(hc->dendro_tree, btRoot(hc->dendro_tree));

}
int hclustNbLeaves(Hclust *hc){
    if (hc == NULL)
    {
        return 0;
    }
    return hc->nb_leaves;
}
void printNode(FILE *fp, BTree *tree, BTNode *node, double dist_parent_fus){
    
   
    double dist_noeud_fus; 
    double length; 

    if (btIsExternal(tree, node)) // ca veut dire qu'on a une feuille 
    {
        dist_noeud_fus =0.0;
        length = dist_parent_fus ; // calcule la long de la branche
        char *obj_name = (char*)btGetData(tree, node); 

        fprintf(fp, "%s:%f", obj_name, length);
    }
        
    else // cas récursif de base pour tous les autres noeuds ( càd s'il est à l'intérieur)
    {
        dist_noeud_fus = *(double *)btGetData(tree, node);
        length = dist_parent_fus - dist_noeud_fus;
    
        fprintf(fp, "(");
        BTNode *left = btLeft(tree, node);
        BTNode *right = btRight(tree, node);
        if(left != NULL){
            printNode(fp, tree, left, dist_noeud_fus);
        }
        fprintf(fp, ",");
        if(right != NULL){
            printNode(fp, tree, right, dist_noeud_fus);
            
        }

        fprintf(fp, "):%f", length);

    }
    
   return ; 
}

void hclustPrintTree(FILE *fp, Hclust *hc)
{
    if (hc == NULL || hc->dendro_tree == NULL || fp == NULL){
        return;
    } 
    BTNode *root = btRoot(hc->dendro_tree);
    if (root == NULL){
        return;
    }
    if (btIsExternal(hc->dendro_tree, root)){
        double dist_root_fus =0.0;
        printNode(fp,hc->dendro_tree, root, dist_root_fus);
    
    }
    else{
        double dist_root_fus = *(double*)btGetData(hc->dendro_tree, root);
        printNode(fp, hc->dendro_tree, root, dist_root_fus);

    }
    fprintf(fp, ";\n");
    return;
}

static void GetClustersDistAux(List *clusters, BTree *tree, BTNode *n, double distanceThreshold){

    if(n == NULL){
        return;
    }

    if (btIsExternal(tree, n)){ // si n est une feuille
        List *clusterlist = llCreateEmpty(); // on cree une liste pour stocker le cluster
        llInsertLast(clusterlist, btGetData(tree, n)); //on insere l'objet a la fin de la liste
        llInsertLast(clusters, clusterlist);
        return;
    }
    double *distptr = (double *)btGetData(tree, n);
    double dist = *distptr;

    // si n est un noeud interne
    if( dist >distanceThreshold){ // la distance entre les clusters est superieure au seuil,
        List *clusterlist = llCreateEmpty();//
        hclustGetLeaves(tree, n, clusterlist);
        llInsertLast(clusters, clusterlist); //


    }

    else{ // le seuil n'est pas atteint, on continue la recherche dans les sous-arbres
        
        GetClustersDistAux(clusters, tree, btLeft(tree, n), distanceThreshold);
        GetClustersDistAux(clusters, tree, btRight(tree, n), distanceThreshold);
    }
}

List *hclustGetClustersDist(Hclust *hc, double distanceThreshold){ // i do

    if (hc == NULL || hc->dendro_tree == NULL){ 
        return llCreateEmpty();
    }

    List *resultclusters = llCreateEmpty();

    //un seul onjet
    if (hc->nb_leaves == 1){
        List *clusterlist = llCreateEmpty();
        llInsertLast(clusterlist, btGetData(hc->dendro_tree, btRoot(hc->dendro_tree)));
        llInsertLast(resultclusters, clusterlist);
        return resultclusters;
    }

    GetClustersDistAux(resultclusters, hc->dendro_tree, btRoot(hc->dendro_tree), distanceThreshold);
    return resultclusters;

} 


List *GetClustersAux(Hclust *hc, int k){
    if (hc == NULL || hc->dendro_tree == NULL || k <= 0){
        return llCreateEmpty(); // car il faut return une liste
    }
    if (k >= hc->nb_leaves){ // k plus grand que le nb d objets, donc on le fixe au nb max d objets
        k = hc->nb_leaves; 
    }
    BTree *tree = hc->dendro_tree;
    BTNode *root = btRoot(tree);
    if (k == 1 || hc->nb_leaves <= 1){
        List *clusterlist = llCreateEmpty();
        hclustGetLeaves(tree, root, clusterlist);
        List *clusters = llCreateEmpty();
        llInsertLast(clusters, clusterlist);
        return clusters;
    }
    return;
}

List *hclustGetClustersK(Hclust *hc, int k){ // I do

    return GetClustersAux(hc, k);
}

BTree *hclustGetTree(Hclust *hc){
   if( hc == NULL){
        return NULL;
    }

    return hc->dendro_tree; // ca me parait un peu simple lol mais je vois pas quoi faire d'autre...
}
    

#endif

*/

/*#ifndef HCLUS_H
#define HCLUS_H

#include "LinkedList.h"
#include "BTree.h"
#include "LinkedList.c"
#include "BTree.c"

struct Hclust_t
{
  BTree *dendro_tree; //pointeur vers la rac de l'arbre 
  int nb_leaves; 
};
typedef struct 
{
  char *o; 
  char *o'; 
  double dist; 
} Triple;

Hclust *hclustBuildTree(List *objects, double (*distFn)(const char *, const char *, void *), void *distFnParams)
{
  Triple tab []; // ini le tableau qui contient nos o, o' et notre distance
  Node *current_node = llHead(objects);
  size_t initial_dict_size = llLength(objects) * 2; // on ini une taille 
  Dict *dict = dictCreate(initial_dict_size); // on crée notre dictionnaire
  if (dict == NULL) // notre alloc échoue
  {
    return 0;
  }   
  while ( current_node!= NULL) 
    {
      char *object = (char *)llData(current_node);
      
      BTree *T_o = btCreateLeaf(object); 
      dictInsert(dict, object, T_o);
      current_node = llNext(current_node);
    }


}

void hclustFree(Hclust *hc){


    return;
}


int hclustDepth(Hclust *hc) {

    return;
}

int hclustNbLeaves(Hclust *hc) {

    return;
}

void hclustPrintTree(FILE *fp, Hclust *hc)
{

    return;
}

List *hclustGetClustersDist(Hclust *hc, double distanceThreshold){

    return;
} 

List *hclustGetClustersK(Hclust *hc, int k) {

    return;
}

BTree *hclustGetTree(Hclust *hc) {

    return;
}
*/
