#ifndef HCLUS_H
#define HCLUS_H

#include <stdlib.h>
#include "LinkedList.h"
#include "BTree.h"
#include "LinkedList.c"
#include "BTree.c"


struct Hclust_t
{
  BTree *dendro_tree; //pointeur vers la rac de l'arbre 
  int nb_leaves; 
};
struct PairDist_t
{
  char *o1; 
  char *o2; 
  double distance; 
};

Hclust *hclustBuildTree(List *objects, double (*distFn)(const char *, const char *, void *), void *distFnParams)
{
  Hclust *clust = (Hclust *)malloc(sizeof(Hclust)); // alloc mémoire
  if (clust==NULL)
  {
    return NULL; // si l'alloc de la mémoire échoue 
  }
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
    BTree *T1 = btCreate(o1); // T1 est un pointeur vers notre cluster
    if(T1==NULL) // on ne veut pas insérer NULL dans notre dict 
    {
      dictFree(dict);
      free(clust); 
      return NULL; // si l'alloc de la mémoire échoue 
    } 
    clust->nb_leaves++;
    Dict_insert(dict, o1, T1); // on ini le dictionnaire aussi
    current_node=llNext(current_node);
  }
  List *distances = llCreateEmpty(); //pour stocker toutes les paires possibles
  Node *pointeur1 = llHead(objects);
  Node *pointeur2 = llNext(objects);
  while (pointeur1!=NULL)
  {
    const char *obj1 = (const char *)llData(pointeur1); // recup le nom de l'ojt 
    Node *pointeur2 = llNext(objects);
    
    while ( pointeur2 != NULL) 
    {
      const char *obj2 = (const char *)llData(pointeur2); // recup le nom de l'ojt
      PairDist_t *pair = malloc(sizeof(PairDist_t));
      if(pair == NULL ) 
      {
        llFreeData(distances);
        dictFree(dict); 
        free(clust);
        return NULL; 
      } 
      double dist = distFn(obj1, obj2, distFnParams);
      pair->o1 = obj1;
      pair->o2 = obj2; 
      pair->distance = dist; 
      
      llInsertLast(distances, pair);
      pointeur2 = llNext(pointeur2); //avance le pointeur
    }
    pointeur1= llNext(pointeur1);
  }
  
  
  
  return clust;   
}

void hclustFree(Hclust *hc)
{}
int hclustDepth(Hclust *hc)
{}
int hclustNbLeaves(Hclust *hc)
{}
void hclustPrintTree(FILE *fp, Hclust *hc)
{}
List *hclustGetClustersDist(Hclust *hc, double distanceThreshold)
{}
List *hclustGetClustersK(Hclust *hc, int k)
{}
BTree *hclustGetTree(Hclust *hc)
{}
