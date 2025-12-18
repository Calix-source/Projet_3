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
//void printNode(FILE *fp, BTNode *node,double dist_parent_fus);

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
    while(nbr_clust_rest >1)// on fusionne pour qu'il nous reste un seul cluster 
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
            if (dist_fus==NULL)
                return NULL; 
            *dist_fus =best_pair-> distance;

            Hclust params;
            params.dendro_tree =T1;
            params.dict = dict; 
            btMapLeaves(T2, btRoot(T2), update_dict_entry, &params);
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
/*
void printNode(FILE *fp, BTNode *node, double dist_parent_fus)
{
    
    BTree *tree;
    double dist_noeud_fus; 
    double length; 
    if (btIsInternal(tree, node)) // ca veut dire qu'on a une feuille 
    {
        dist_noeud_fus =0;
        length = dist_parent_fus; // calcule la long de la branche
        char *obj_name = (char*)node->data; 
    }
        
    else // cas récursif de base pour tous les autres noeuds ( càd s'il est à l'intérieur)
    {
        dist_noeud_fus = *(double *)node->data;
        double length = dist_parent_fus - dist_noeud_fus;
    }
    
   return ; 
}
*/
void hclustPrintTree(FILE *fp, Hclust *hc){ //FILE *fp c'est la où on va envoyer la sortie de notre arbre
    (void)fp; 
    (void)hc;

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
