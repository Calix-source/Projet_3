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

