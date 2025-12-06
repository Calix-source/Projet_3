#ifndef HCLUS_H
#define HCLUS_H

#include "LinkedList.h"
#include "BTree.h"

struct Hclust_t
{
};
Hclust *hclustBuildTree(List *objects, double (*distFn)(const char *, const char *, void *), void *distFnParams);
