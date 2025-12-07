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
  typedef struct Node_t
{
    char *key;
    void *value;
    struct Node_t *next;
} Node;
  struct Dict_t
{
    Node **array;
    size_t arraySize;
    size_t nbKeys;
};
  // on ini le dictionnaire
  Dict_t *dict = malloc(sizeof(Dict_t));
  dict->arrraySize = ;       
  dict->nbKeys = ;      
  dict->array = malloc(dict->size * sizeof(void));
  array->key= ;
  array->value= ; 
}

