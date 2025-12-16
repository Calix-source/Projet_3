#ifndef HCLUS_H
#define HCLUS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Dict.h"
#include "BTree.h"
#include "HierarchicalClustering.h"
#include "LinkedList.h"
typedef struct Hclust_t Hclust;
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



struct Hclust_t
{
  BTree *dendro_tree; //pointeur vers la rac de l'arbre 
  int nb_leaves;
  Dict *dict; 
};



typedef struct Triple_t
{
  char *o1; 
  char *o2; 
  double dist; 
} Triple;


Hclust *hclustBuildTree(List *objects, double (*distFn)(const char *, const char *, void *), void *distFnParams)
{
  Triple tab []; // ini le tableau qui contient nos o, p et notre distance
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

    if(hc == NULL){
        return;
    }

    btFree(hc->dendro_tree);

    dictFree(hc->dict);
    free(hc);
    return;
}

static void hclustGetLeaves(BTree *tree, BTNode *n, List *clusterlist)
{
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

int hclustDepth(Hclust *hc) {
    if (hc == NULL || hc->dendro_tree == NULL){
        return 0;
    }
    return hclustDepthAux(hc->dendro_tree, btRoot(hc->dendro_tree));

}

int hclustNbLeaves(Hclust *hc) {
    if (hc == NULL){
        return 0;
    }
    return hc->nb_leaves;
}

void hclustPrintTree(FILE *fp, Hclust *hc)
{

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

BTree *hclustGetTree(Hclust *hc) {

    return;
}

#endif
