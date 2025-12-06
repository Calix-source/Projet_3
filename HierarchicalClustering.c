#ifndef HCLUS_H
#define HCLUS_H

#include "LinkedList.h"
#include "BTree.h"

struct Hclust_t
{
  BTree *dendro_tree; //pointeur vers la rac de l'arbre 
  int nb_leaves; 
};
Hclust *hclustBuildTree(List *objects, double (*distFn)(const char *, const char *, void *), void *distFnParams)
{
  
}

