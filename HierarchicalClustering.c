#ifndef HCLUS_H
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
Hclust *hclustBuildTree(List *objects, double (*distFn)(const char *, const char *, void *), void *distFnParams)
{
  Node *current_node = llHead(objects);
  while ( current_node!= NULL) 
    {
      
    }
  Dict_t *dict = malloc(sizeof(Dict_t));
  dict->size = 10;       // taille initiale, comme tu veux
  dict->nKeys = 0;       // aucun élément au début

  dict->keys = malloc(dict->size * sizeof(int));
  dict->values = malloc(dict->size * sizeof(void*));
}

