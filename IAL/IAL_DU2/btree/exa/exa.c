/*modify by xstahl01*/
/*
 * Použití binárních vyhledávacích stromů.
 *
 * S využitím Vámi implementovaného binárního vyhledávacího stromu (soubory ../iter/btree.c a ../rec/btree.c)
 * implementujte funkci letter_count. Výstupní strom může být značně degradovaný (až na úroveň lineárního seznamu) 
 * a tedy implementujte i druhou funkci (bst_balance), která strom, na požadavek uživatele, vybalancuje.
 * Funkce jsou na sobě nezávislé a tedy automaticky NEVOLEJTE bst_balance v letter_count.
 * 
 */

#include "../btree.h"
#include <stdio.h>
#include <stdlib.h>


/**
 * Vypočítání frekvence výskytů znaků ve vstupním řetězci.
 * 
 * Funkce inicilializuje strom a následně zjistí počet výskytů znaků a-z (case insensitive), znaku 
 * mezery ' ', a ostatních znaků (ve stromu reprezentováno znakem podtržítka '_'). Výstup je v 
 * uložen ve stromu.
 * 
 * Například pro vstupní řetězec: "abBccc_ 123 *" bude strom po běhu funkce obsahovat:
 * 
 * key | value
 * 'a'     1
 * 'b'     2
 * 'c'     3
 * ' '     2
 * '_'     5
 * 
 * Pro implementaci si můžete v tomto souboru nadefinovat vlastní pomocné funkce.
*/
void letter_count(bst_node_t **tree, char *input) {
    if (!input) return;

    while (*input) {
        char key;

        // is alphabet funkcia 
        if ((*input >= 'A' && *input <= 'Z') || (*input >= 'a' && *input <= 'z')) {
            key = (*input >= 'A' && *input <= 'Z') ? *input - 'A' + 'a' : *input;
        } else if (*input == ' ') {
            key = ' ';
        } else {
            key = '_';
        }

        int count = 0;
        bst_search(*tree, key, &count); // kuk na aktutálny count
        bst_insert(tree, key, count + 1); // Insert/increment count

        input++; //ide ďalej
    }
}




/**
 * Vyvážení stromu.
 * 
 * Vyvážený binární vyhledávací strom je takový binární strom, kde hloubka podstromů libovolného uzlu se od sebe liší maximálně o jedna.
 * 
 * Předpokládejte, že strom je alespoň inicializován. K získání uzlů stromu využijte vhodnou verzi vámi naimplmentovaného průchodu stromem.
 * Následně můžete například vytvořit nový strom, kde pořadím vkládaných prvků zajistíte vyváženost.
 *  
 * Pro implementaci si můžete v tomto souboru nadefinovat vlastní pomocné funkce. Není nutné, aby funkce fungovala *in situ* (in-place).
*/

/** Funckia bst_count_nodes
 * @param tree ukazatel na strom
 * @return počet uzlov v strome
 * 
*/
int bst_count_nodes(bst_node_t *tree){
    if(!tree){
        return 0;
    }
    return 1 + bst_count_nodes(tree->left) + bst_count_nodes(tree->right);
}
/** Fukncia store_inorder
 * @param tree ukazateľ na strom 
 * @param nodes pole ukazateľov na uzly stromu
 * @param index ukazateľ na index do poľa 
 * 
*/
void store_inorder(bst_node_t *tree, bst_node_t **nodes, int *index) {
    if (!tree) return;

    store_inorder(tree->left, nodes, index); //ľavý
    nodes[(*index)++] = tree; //current
    store_inorder(tree->right, nodes, index); //pravý
}

/** Funkcia build_balanced_tree
 * @param nodes pole ukazateľov na uzly stromu
 * @param start index začiatku pod poľa pre rekurívne vytváranie stromu
 * @param end konečný index podpoľa
 * @return ukazateľ na root novo vyváženého stromu
*/
bst_node_t *build_balanced_tree(bst_node_t **nodes, int start, int end) {
    if (start > end) return NULL;

    int mid = (start + end) / 2; 
    bst_node_t *node = nodes[mid]; //middle == root

    //postav podstromy
    node->left = build_balanced_tree(nodes, start, mid - 1);
    node->right = build_balanced_tree(nodes, mid + 1, end);

    return node; //nová root 
}

void bst_balance(bst_node_t **tree) {
    int count = bst_count_nodes(*tree);  
    bst_node_t **nodes = malloc(sizeof(bst_node_t*) * count);
    if (!nodes) return;

    int index = 0;
    store_inorder(*tree, nodes, &index); 
    *tree = build_balanced_tree(nodes, 0, count - 1);
    free(nodes);
}
//END OF exa.c
