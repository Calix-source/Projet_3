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
  size_t initial_dict_size = 100; // on met une taille de tab aléatoire
  Dict *dict = dictCreate(initial_dict_size); // on crée notre dictionnaire

  if (dict == NULL) // notre alloc échoue
  {
    return 0;
  } 
}

