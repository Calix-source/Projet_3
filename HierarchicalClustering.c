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

static int compare_distance(void *a, void *b) // on en a besoin pour trier nos distances apres
{
    PairDist_t *pA = (PairDist_t *)a;
    PairDist_t *pB = (PairDist_t *)b;
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

    while (pointeur1!=NULL)
    {
        const char *obj1 = (const char *)llData(pointeur1); // recup le nom de l'ojt 
        Node *pointeur2 = llNext(pointeur1);
    
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
    BTree *T_new = NULL; // decla pour 
    while(nbr_clust_rest >1)
    {
        PairDist_t *best_pair = llPopFirst(distances); // on trouve un pointeur vers la struct avec la plus petite distance
        if (best_pair == NULL) 
            break;
        BTree *T1 = Dict_search(dict, best_pair->o1); // on reg si nos objts ont deja fusio 
        BTree *T2 = Dict_search(dict, best_pair->o2);
        // si les objts ont déjà été fusio indirect dans un cluster plus grand
        if( T1==T2)
        {
            free(best_pair);
            continue; 
        }
        else // on a trouvé les deux clusters les plus proches et distincts
        {
            double dist_fus = best_pair-> distance; 
            BTree *T_new_tempo = btMergeTrees(T1, T2, dist_fus); 
            if(T_new_tempo==NULL)
            {   
                llFreeData(distances);
                dictFree(dict);
                free(clust); 
                return NULL; // si l'alloc de la mémoire échoue 
            }
            mapLeaves(dict,T1,T_new_tempo); 
            mapLeaves(dict,T2,T_new_tempo);   
            T_new = T_new_tempo;   
            free(best_pair);
            nbr_clust_rest --;  
        }
    }
    clust->dendro_tree = T_new; 
    dictFree(dict); 
    llFreeData(distances); 
    return clust;   
}

void hclustFree(Hclust *hc)
{
    if(hc==NULL)
        return; 
    btFree(hc->dendro_tree); // libere les elem de l'arbre et l'arbre 
    free(hc); 
}
int hclustDepth(Hclust *hc)
{
    
}
int hclustNbLeaves(Hclust *hc)
{

}
void hclustPrintTree(FILE *fp, Hclust *hc)
{

}
List *hclustGetClustersDist(Hclust *hc, double distanceThreshold)
{

}
List *hclustGetClustersK(Hclust *hc, int k)
{

}
BTree *hclustGetTree(Hclust *hc)
{

}
