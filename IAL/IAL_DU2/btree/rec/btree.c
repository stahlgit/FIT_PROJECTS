/*modify by xstahl01*/
/*
 * Binární vyhledávací strom — rekurzivní varianta
 *
 * S využitím datových typů ze souboru btree.h a připravených koster funkcí
 * implementujte binární vyhledávací strom pomocí rekurze.
 */

#include "../btree.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * Inicializace stromu.
 *
 * Uživatel musí zajistit, že inicializace se nebude opakovaně volat nad
 * inicializovaným stromem. V opačném případě může dojít k úniku paměti (memory
 * leak). Protože neinicializovaný ukazatel má nedefinovanou hodnotu, není
 * možné toto detekovat ve funkci. 
 */
void bst_init(bst_node_t **tree) {
  *tree = NULL;
}

/*
 * Vyhledání uzlu v stromu.
 *
 * V případě úspěchu vrátí funkce hodnotu true a do proměnné value zapíše
 * hodnotu daného uzlu. V opačném případě funkce vrátí hodnotu false a proměnná
 * value zůstává nezměněná.
 * 
 * Funkci implementujte rekurzivně bez použité vlastních pomocných funkcí.
 */
bool bst_search(bst_node_t *tree, char key, int *value) {
  bst_node_t *tmp = tree;
  if(!tmp){
    return false;
  }

  if(tmp->key == key){
    *value = tmp->value;
    return true;
    }
  
  else if (tmp->key > key){ //ľavý podstrom
    if(tmp->left != NULL){
      bst_search(tmp->left, key, value);
    }
    else{
      return false;
    }
  }
  else if(tmp->key < key){ //pravý podstrom
    if(tmp->right != NULL){
      bst_search(tmp->right, key, value);
    }
    else{
      return false;
    }
  }
  return false;
}

/*
 * Vložení uzlu do stromu.
 *
 * Pokud uzel se zadaným klíče už ve stromu existuje, nahraďte jeho hodnotu.
 * Jinak vložte nový listový uzel.
 *
 * Výsledný strom musí splňovat podmínku vyhledávacího stromu — levý podstrom
 * uzlu obsahuje jenom menší klíče, pravý větší. 
 *
 * Funkci implementujte rekurzivně bez použití vlastních pomocných funkcí.
 */
void bst_insert(bst_node_t **tree, char key, int value) {
  // Na jednoduchšiu manipuláciu použite dočasný ukazovateľ
    bst_node_t *tmp = *tree;

    if (!tmp) {
        tmp = malloc(sizeof(bst_node_t)); // Vytvorte nový uzol, ak je strom prázdny
        if (!tmp) {
            fprintf(stderr, "Failed to allocate memory for a new node.\n");
            return;
        }
        tmp->key = key;
        tmp->value = value;
        tmp->left = NULL;
        tmp->right = NULL;
        *tree = tmp;  // Nasmerujte pôvodný ukazovateľ stromu na nový uzol
        return;
    }

    if (tmp->key == key) {
        tmp->value = value; //update hodnoty
    } else if (key < tmp->key) {
        //ľavá podstrom
        bst_insert(&(tmp->left), key, value);
    } else {
        // pravý podstrom
        bst_insert(&(tmp->right), key, value);
    }
}



/*
 * Pomocná funkce která nahradí uzel nejpravějším potomkem.
 * 
 * Klíč a hodnota uzlu target budou nahrazeny klíčem a hodnotou nejpravějšího
 * uzlu podstromu tree. Nejpravější potomek bude odstraněný. Funkce korektně
 * uvolní všechny alokované zdroje odstraněného uzlu.
 *
 * Funkce předpokládá, že hodnota tree není NULL.
 * 
 * Tato pomocná funkce bude využitá při implementaci funkce bst_delete.
 *
 * Funkci implementujte rekurzivně bez použití vlastních pomocných funkcí.
 */
void bst_replace_by_rightmost(bst_node_t *target, bst_node_t **tree) {
  if(!tree || !*tree){
    return;
  }

    bst_node_t *tmp = *tree;

  if(tmp->right != NULL){
    bst_replace_by_rightmost(target, &(tmp->right));
    return ;
  }
    target->key = tmp->key;
    target->value = tmp->value;

    // uloženie uzlu na odstranenie
    bst_node_t *to_remove = *tree;

    //reorganizácia  
    *tree = (*tree)->left;

    free(to_remove);
    return;
}

/*
 * Odstranění uzlu ze stromu.
 *
 * Pokud uzel se zadaným klíčem neexistuje, funkce nic nedělá.
 * Pokud má odstraněný uzel jeden podstrom, zdědí ho rodič odstraněného uzlu.
 * Pokud má odstraněný uzel oba podstromy, je nahrazený nejpravějším uzlem
 * levého podstromu. Nejpravější uzel nemusí být listem.
 * 
 * Funkce korektně uvolní všechny alokované zdroje odstraněného uzlu.
 * 
 * Funkci implementujte rekurzivně pomocí bst_replace_by_rightmost a bez
 * použití vlastních pomocných funkcí.
 */
void bst_delete(bst_node_t **tree, char key) {
  if (!*tree) {
    return;
  }

  if (key < (*tree)->key) {
    bst_delete(&((*tree)->left), key);
  } else if (key > (*tree)->key) {
    bst_delete(&((*tree)->right), key);
  } else {
    if ((*tree)->left && (*tree)->right) {
      bst_replace_by_rightmost(*tree, &((*tree)->left));
    } else {
      bst_node_t *to_remove = *tree;
      *tree = (*tree)->left ? (*tree)->left : (*tree)->right;
      free(to_remove);
    }
  }
}

/*
 * Zrušení celého stromu.
 * 
 * Po zrušení se celý strom bude nacházet ve stejném stavu jako po 
 * inicializaci. Funkce korektně uvolní všechny alokované zdroje rušených 
 * uzlů.
 * 
 * Funkci implementujte rekurzivně bez použití vlastních pomocných funkcí.
 */
void bst_dispose(bst_node_t **tree) {
  if(!*tree){
    return;
  }
  bst_node_t *tmp = *tree;
  bst_dispose(&(tmp->left));
  bst_dispose(&(tmp->right));
  free(tmp);
  *tree= NULL;
}

/*
 * Preorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte rekurzivně bez použití vlastních pomocných funkcí.
 */
void bst_preorder(bst_node_t *tree, bst_items_t *items) {
  if(tree){
    bst_add_node_to_items(tree,items);
    bst_preorder(tree->left, items);
    bst_preorder(tree->right, items);
  }
}

/*
 * Inorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte rekurzivně bez použití vlastních pomocných funkcí.
 */
void bst_inorder(bst_node_t *tree, bst_items_t *items) {
  if(tree){
    bst_inorder(tree->left, items);
    bst_add_node_to_items(tree, items);
    bst_inorder(tree->right, items);
  }
}

/*
 * Postorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte rekurzivně bez použití vlastních pomocných funkcí.
 */
void bst_postorder(bst_node_t *tree, bst_items_t *items) {
  if(tree){
    bst_postorder(tree->left, items);
    bst_postorder(tree->right, items);
    bst_add_node_to_items(tree, items);
  }
}
//END OF btree.c
