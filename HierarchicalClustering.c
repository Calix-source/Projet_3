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
typedef struct 
{
  char *o1; 
  char *o2; 
  double dist; 
} Triple;

Hclust *hclustBuildTree(List *objects, double (*distFn)(const char *, const char *, void *), void *distFnParams)
{
  Hclust *clust = (Hclust *)malloc(sizeof(Hclust)); // alloc mémoire
  if (clust==NULL)
    return NULL; // si l'alloc de la mémoire échoue 
  clust->dendro_tree = NULL ; //ini nos champs 
  clust->nb_leaves = 0; 
  Dict *dict = dictCreate(initial_dict_size); // on crée notre dictionnaire
  if (dict==NULL) // notre alloc échoue
    return 0;
  char *objet_a = "a";
// T_a est un pointeur vers un arbre BTree (votre cluster)
BTree *T_a = btCreate(objet_a);
}



/*Hclust *hclustBuildTree(List *objects, double (*distFn)(const char *, const char *, void *), void *distFnParams)
{
  //Triple tab []; // ini le tableau qui contient nos o, o' et notre distance
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
*/


