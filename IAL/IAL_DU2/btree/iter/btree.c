/*modify by xstahl01*/
/*
 * Binární vyhledávací strom — iterativní varianta
 *
 * S využitím datových typů ze souboru btree.h, zásobníku ze souboru stack.h 
 * a připravených koster funkcí implementujte binární vyhledávací 
 * strom bez použití rekurze.
 */

#include "../btree.h"
#include "stack.h"
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
 * Funkci implementujte iterativně bez použité vlastních pomocných funkcí.
 */
bool bst_search(bst_node_t *tree, char key, int *value) {
  bst_node_t *tmp = tree;

  while (tmp){
    if(tmp->key == key){
      *value = tmp->value; //najdený kľúč
      return true;
    } //prechod z oboch strán
    else if (tmp->key > key){
      if(tmp->left != NULL){
        tmp = tmp->left;
      }
      else{
        return false;
      }
    }
    else{
      if(tmp->right != NULL){
        tmp = tmp->right;
      }
      else{
        return false;
      }
    }
  }
  return false; //not found
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
 * Funkci implementujte iterativně bez použití vlastních pomocných funkcí.
 */
void bst_insert(bst_node_t **tree, char key, int value) {
  bst_node_t *tmp = *tree;
  //prázdny strom
  if(!tmp){
    tmp = malloc(sizeof(bst_node_t));
    if(!tmp){
      fprintf(stderr, "Failed to allocate memopry for a new node.\n");
      return;
    }
    tmp->key = key;
    tmp->value = value;
    tmp->left = NULL;
    tmp->right = NULL;
    *tree = tmp;
    return;
  }
  //hľadá správnu pozíciu
  while(tmp){
    if(tmp->key == key){
      tmp->value = value;
      return;
    }
    else if(tmp->key > key){
      if(tmp->left == NULL){
        tmp->left = malloc(sizeof(bst_node_t));
        if(!tmp->left){
          fprintf(stderr, "Failed to allocate memory for a new node.\n");
          return;
        }
        tmp->left->key = key;
        tmp->left->value = value;
        tmp->left->left = NULL;
        tmp->left->right = NULL;
        return;
      }
      else{
        tmp=tmp->left;
      }
    }
    else{
      if(tmp->right == NULL){
        tmp->right = malloc(sizeof(bst_node_t));
        if(!tmp->right){
          fprintf(stderr, "Failed to allocate memory for a new node. \n");
        }
        tmp->right->key = key;
        tmp->right->value = value;
        tmp->right->left = NULL;
        tmp->right->right = NULL;
        return;
      }
      else{
        tmp=tmp->right;
      }
    }
  }
}

/*
 * Pomocná funkce která nahradí uzel nejpravějším potomkem.
 * 
 * Klíč a hodnota uzlu target budou nahrazené klíčem a hodnotou nejpravějšího
 * uzlu podstromu tree. Nejpravější potomek bude odstraněný. Funkce korektně
 * uvolní všechny alokované zdroje odstraněného uzlu.
 *
 * Funkce předpokládá, že hodnota tree není NULL.
 * 
 * Tato pomocná funkce bude využita při implementaci funkce bst_delete.
 *
 * Funkci implementujte iterativně bez použití vlastních pomocných funkcí.
 */
void bst_replace_by_rightmost(bst_node_t *target, bst_node_t **tree) {
  if(!tree || !*tree){
    return;
  }

  bst_node_t *parent = NULL;
  bst_node_t *current = *tree;

  while(current->right != NULL){
    parent = current;
    current = current->right;
  }
  
  target->key = current->key;
  target->value = current->value;

  if(parent){
    parent->right = current->left;
  }
  else{
    *tree = current->left;
  }
  free(current);
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
 * Funkci implementujte iterativně pomocí bst_replace_by_rightmost a bez
 * použití vlastních pomocných funkcí.
 */
void bst_delete(bst_node_t **tree, char key) {
    if (!*tree) {
        return;
    }

    bst_node_t *parent = NULL;
    bst_node_t *current = *tree;

    // Nájdite uzol, ktorý chcete odstrániť
    while (current && current->key != key) {
        parent = current;
        if (key < current->key) {
            current = current->left;
        } else {
            current = current->right;
        }
    }

    if (!current) {
        // nenájdený 
        return;
    }

    // 2 deti
    if (current->left && current->right) {
        bst_replace_by_rightmost(current, &(current->left));
    } else {
        // 0 alebo 1 dieťa 
        bst_node_t *child = (current->left) ? current->left : current->right;

        if (!parent) {
            // mazanie
            *tree = child;
        } else if (parent->left == current) {
            parent->left = child;
        } else {
            parent->right = child;
        }

        free(current);
    }
}

/*
 * Zrušení celého stromu.
 * 
 * Po zrušení se celý strom bude nacházet ve stejném stavu jako po 
 * inicializaci. Funkce korektně uvolní všechny alokované zdroje rušených 
 * uzlů.
 * 
 * Funkci implementujte iterativně s pomocí zásobníku a bez použití 
 * vlastních pomocných funkcí.
 */
void bst_dispose(bst_node_t **tree) {
  stack_bst_t stack;
  stack_bst_init(&stack);

  stack_bst_push(&stack, *tree);
  while (!stack_bst_empty(&stack))
  {
    bst_node_t *tmp = stack_bst_pop(&stack);
    if(!tmp){
      continue;
    }
    stack_bst_push(&stack, tmp->left);
    stack_bst_push(&stack, tmp->right);
    free(tmp);
  }
  *tree = NULL;
}

/*
 * Pomocná funkce pro iterativní preorder.
 *
 * Prochází po levé větvi k nejlevějšímu uzlu podstromu.
 * Nad zpracovanými uzly zavolá bst_add_node_to_items a uloží je do zásobníku uzlů.
 *
 * Funkci implementujte iterativně s pomocí zásobníku a bez použití 
 * vlastních pomocných funkcí.
 */
void bst_leftmost_preorder(bst_node_t *tree, stack_bst_t *to_visit, bst_items_t *items) {
  bst_node_t *tmp = tree;
  while (tmp) {
    stack_bst_push(to_visit, tmp);
    bst_add_node_to_items(tmp, items);  
    tmp = tmp->left;
  }
}


/*
 * Preorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte iterativně pomocí funkce bst_leftmost_preorder a
 * zásobníku uzlů a bez použití vlastních pomocných funkcí.
 */
void bst_preorder(bst_node_t *tree, bst_items_t *items) {
    if (!tree) return;

    stack_bst_t stack;
    stack_bst_init(&stack);

    stack_bst_push(&stack, tree);
    while (!stack_bst_empty(&stack)) {
        bst_node_t *node = stack_bst_pop(&stack);
        bst_add_node_to_items(node, items);

        if (node->right) stack_bst_push(&stack, node->right);
        if (node->left) stack_bst_push(&stack, node->left);
    }
}


/*
 * Pomocná funkce pro iterativní inorder.
 * 
 * Prochází po levé větvi k nejlevějšímu uzlu podstromu a ukládá uzly do
 * zásobníku uzlů.
 *
 * Funkci implementujte iterativně s pomocí zásobníku a bez použití 
 * vlastních pomocných funkcí.
 */
void bst_leftmost_inorder(bst_node_t *tree, stack_bst_t *to_visit) {
  bst_node_t *tmp = tree;
  
  while(tmp){
    stack_bst_push(to_visit,tmp);
    tmp=tmp->left;
  }
}

/*
 * Inorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte iterativně pomocí funkce bst_leftmost_inorder a
 * zásobníku uzlů a bez použití vlastních pomocných funkcí.
 */
void bst_inorder(bst_node_t *tree, bst_items_t *items) {
  stack_bst_t stack;
  bst_node_t *tmp = tree;
  stack_bst_init(&stack);

  bst_leftmost_inorder(tmp, &stack);
  while (!stack_bst_empty(&stack)) {
    tmp = stack_bst_pop(&stack);
    bst_add_node_to_items(tmp, items);  
    bst_leftmost_inorder(tmp->right, &stack);
  }
}

/*
 * Pomocná funkce pro iterativní postorder.
 *
 * Prochází po levé větvi k nejlevějšímu uzlu podstromu a ukládá uzly do
 * zásobníku uzlů. Do zásobníku bool hodnot ukládá informaci, že uzel
 * byl navštíven poprvé.
 *
 * Funkci implementujte iterativně pomocí zásobníku uzlů a bool hodnot a bez použití
 * vlastních pomocných funkcí.
 */
void bst_leftmost_postorder(bst_node_t *tree, stack_bst_t *to_visit, stack_bool_t *first_visit) {
  bst_node_t *tmp = tree;
  
  while(tmp){
    stack_bst_push(to_visit, tmp);
    stack_bool_push(first_visit, true);
    tmp = tmp->left;
  }                          
}

/*
 * Postorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte iterativně pomocí funkce bst_leftmost_postorder a
 * zásobníku uzlů a bool hodnot a bez použití vlastních pomocných funkcí.
 */
void bst_postorder(bst_node_t *tree, bst_items_t *items) {
  stack_bst_t stack;
  bst_node_t *tmp = tree;
  stack_bst_init(&stack);
  stack_bool_t BoolStack;
  stack_bool_init(&BoolStack);
  bool left;

  bst_leftmost_postorder(tree, &stack, &BoolStack);

  while (!stack_bst_empty(&stack)) {
    tmp = stack_bst_pop(&stack);
    left = stack_bool_pop(&BoolStack);

    if (left) {
      stack_bst_push(&stack, tmp);
      stack_bool_push(&BoolStack, false);
      bst_leftmost_postorder(tmp->right, &stack, &BoolStack);
    } else {
      bst_add_node_to_items(tmp, items);  
    }
  }
}
//END OF btree.c