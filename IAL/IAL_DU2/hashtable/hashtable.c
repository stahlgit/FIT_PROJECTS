/* modify by xstahl01 */
/*
 * Tabulka s rozptýlenými položkami
 *
 * S využitím datových typů ze souboru hashtable.h a připravených koster
 * funkcí implementujte tabulku s rozptýlenými položkami s explicitně
 * zretězenými synonymy.
 *
 * Při implementaci uvažujte velikost tabulky HT_SIZE.
 */

#include "hashtable.h"
#include <stdlib.h>
#include <string.h>

int HT_SIZE = MAX_HT_SIZE;

/*
 * Rozptylovací funkce která přidělí zadanému klíči index z intervalu
 * <0,HT_SIZE-1>. Ideální rozptylovací funkce by měla rozprostírat klíče
 * rovnoměrně po všech indexech. Zamyslete sa nad kvalitou zvolené funkce.
 */
int get_hash(char *key) {
  int result = 1;
  int length = strlen(key);
  for (int i = 0; i < length; i++) {
    result += key[i]; // Zhromažďuje hodnoty znakov
  }
  return (result % HT_SIZE); // Zaisťuje, že index je v rámci veľkosti tabuľky
}

/*
 * Inicializace tabulky — zavolá sa před prvním použitím tabulky.
 */
void ht_init(ht_table_t *table) {
  for(int i = 0; i < HT_SIZE; i++)
  {
    (*table)[i] = malloc(sizeof(ht_item_t));
    (*table)[i] = NULL; // Nastavte každý slot na hodnotu NULL
  }

}

/*
 * Vyhledání prvku v tabulce.
 *
 * V případě úspěchu vrací ukazatel na nalezený prvek; v opačném případě vrací
 * hodnotu NULL.
 */
ht_item_t *ht_search(ht_table_t *table, char *key) {
  for(int i = 0; i<MAX_HT_SIZE; i++)
  {
    if((*table)[i]) //!= NULL
    {
      if((*table)[i]->key == key){
        return (*table[i]);
      }
    }
  }
  return NULL;
}

/*
 * Vložení nového prvku do tabulky.
 *
 * Pokud prvek s daným klíčem už v tabulce existuje, nahraďte jeho hodnotu.
 *
 * Při implementaci využijte funkci ht_search. Pri vkládání prvku do seznamu
 * synonym zvolte nejefektivnější možnost a vložte prvek na začátek seznamu.
 */
void ht_insert(ht_table_t *table, char *key, float value) {
  ht_item_t *item = ht_search(table, key);
  if (item != NULL) 
  {
    // nie nový prvok
    item->value = value;
  }
  else
  { 
    // nový prvok
    ht_item_t *new = (ht_item_t*)malloc(sizeof(ht_item_t));
    if(!new){
      return;
    }
    new->key = key;
    new->value = value;
    new->next = NULL;

    ht_item_t **bucket = &(*table)[get_hash(key)];

    if (*bucket !=  NULL)
    {
      new->next = *bucket; //vloží na začiatok
    }
    *bucket = new;

  }
}

/*
 * Získání hodnoty z tabulky.
 *
 * V případě úspěchu vrací funkce ukazatel na hodnotu prvku, v opačném
 * případě hodnotu NULL.
 *
 * Při implementaci využijte funkci ht_search.
 */
float *ht_get(ht_table_t *table, char *key) {
    ht_item_t *tmp = ht_search(table, key);
    if(tmp != NULL){
      return &(tmp->value); // vráti adresu hodnoty
    }

    return NULL;
}

/*
 * Smazání prvku z tabulky.
 *
 * Funkce korektně uvolní všechny alokované zdroje přiřazené k danému prvku.
 * Pokud prvek neexistuje, funkce nedělá nic.
 *
 * Při implementaci NEPOUŽÍVEJTE funkci ht_search.
 */
void ht_delete(ht_table_t *table, char *key){
  ht_item_t *tmp = (*table)[get_hash(key)];
  
  if (tmp == NULL){
     return;
  }
  else if (tmp->key == key) 
  {
    (*table)[get_hash(key)] = tmp->next; //odstráni zo začiatku
    free(tmp); 
    return;
  }

  ht_item_t *prev = tmp;
  tmp = tmp->next;
  while (tmp != NULL) 
  {
    if (tmp->key == key) 
    {
      prev->next = tmp->next; //odstráni zo stredu/konca
      free(tmp);
      return;
    }
    tmp = tmp->next;
    prev = tmp;
  }
}
/*
 * Smazání všech prvků z tabulky.
 *
 * Funkce korektně uvolní všechny alokované zdroje a uvede tabulku do stavu po 
 * inicializaci.
 */
void ht_delete_all(ht_table_t *table) {
  for (int i = 0; i < HT_SIZE; i++) 
  {
    ht_item_t *tmp = (*table)[i];
    while (tmp != NULL)
    {
      ht_item_t *toDelete = tmp;
      tmp = tmp->next;
      free(toDelete);
    }
    (*table)[i] = NULL;
  }
}
//END OF hashtable.c