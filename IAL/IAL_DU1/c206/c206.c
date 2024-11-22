/* ******************************* c206.c *********************************** */
/*  Předmět: Algoritmy (IAL) - FIT VUT v Brně                                 */
/*  Úkol: c206 - Dvousměrně vázaný lineární seznam                            */
/*  Návrh a referenční implementace: Bohuslav Křena, říjen 2001               */
/*  Vytvořil: Martin Tuček, říjen 2004                                        */
/*  Upravil: Kamil Jeřábek, září 2020                                         */
/*           Daniel Dolejška, září 2021                                       */
/*           Daniel Dolejška, září 2022                                       */
/* ************************************************************************** */
/*
** Implementujte abstraktní datový typ dvousměrně vázaný lineární seznam.
** Užitečným obsahem prvku seznamu je hodnota typu int. Seznam bude jako datová
** abstrakce reprezentován proměnnou typu DLList (DL znamená Doubly-Linked
** a slouží pro odlišení jmen konstant, typů a funkcí od jmen u jednosměrně
** vázaného lineárního seznamu). Definici konstant a typů naleznete
** v hlavičkovém souboru c206.h.
**
** Vaším úkolem je implementovat následující operace, které spolu s výše
** uvedenou datovou částí abstrakce tvoří abstraktní datový typ obousměrně
** vázaný lineární seznam:
**
**      DLL_Init ........... inicializace seznamu před prvním použitím,
**      DLL_Dispose ........ zrušení všech prvků seznamu,
**      DLL_InsertFirst .... vložení prvku na začátek seznamu,
**      DLL_InsertLast ..... vložení prvku na konec seznamu,
**      DLL_First .......... nastavení aktivity na první prvek,
**      DLL_Last ........... nastavení aktivity na poslední prvek,
**      DLL_GetFirst ....... vrací hodnotu prvního prvku,
**      DLL_GetLast ........ vrací hodnotu posledního prvku,
**      DLL_DeleteFirst .... zruší první prvek seznamu,
**      DLL_DeleteLast ..... zruší poslední prvek seznamu,
**      DLL_DeleteAfter .... ruší prvek za aktivním prvkem,
**      DLL_DeleteBefore ... ruší prvek před aktivním prvkem,
**      DLL_InsertAfter .... vloží nový prvek za aktivní prvek seznamu,
**      DLL_InsertBefore ... vloží nový prvek před aktivní prvek seznamu,
**      DLL_GetValue ....... vrací hodnotu aktivního prvku,
**      DLL_SetValue ....... přepíše obsah aktivního prvku novou hodnotou,
**      DLL_Previous ....... posune aktivitu na předchozí prvek seznamu,
**      DLL_Next ........... posune aktivitu na další prvek seznamu,
**      DLL_IsActive ....... zjišťuje aktivitu seznamu.
**
** Při implementaci jednotlivých funkcí nevolejte žádnou z funkcí
** implementovaných v rámci tohoto příkladu, není-li u funkce explicitně
 * uvedeno něco jiného.
**
** Nemusíte ošetřovat situaci, kdy místo legálního ukazatele na seznam
** předá někdo jako parametr hodnotu NULL.
**
** Svou implementaci vhodně komentujte!
**
** Terminologická poznámka: Jazyk C nepoužívá pojem procedura.
** Proto zde používáme pojem funkce i pro operace, které by byly
** v algoritmickém jazyce Pascalovského typu implemenovány jako procedury
** (v jazyce C procedurám odpovídají funkce vracející typ void).
**
**/
/* AUTHOR: Peter Stahl xstahl01 */

#include "c206.h"

bool error_flag;
bool solved;

/**
 * Vytiskne upozornění na to, že došlo k chybě.
 * Tato funkce bude volána z některých dále implementovaných operací.
 */
void DLL_Error(void) {
	printf("*ERROR* The program has performed an illegal operation.\n");
	error_flag = true;
}

/**
 * Provede inicializaci seznamu list před jeho prvním použitím (tzn. žádná
 * z následujících funkcí nebude volána nad neinicializovaným seznamem).
 * Tato inicializace se nikdy nebude provádět nad již inicializovaným seznamem,
 * a proto tuto možnost neošetřujte.
 * Vždy předpokládejte, že neinicializované proměnné mají nedefinovanou hodnotu.
 *
 * @param list Ukazatel na strukturu dvousměrně vázaného seznamu
 */
void DLL_Init( DLList *list ) {
	list->activeElement = NULL;
	list->firstElement = NULL;
	list->lastElement = NULL; 
}

/**
 * Zruší všechny prvky seznamu list a uvede seznam do stavu, v jakém se nacházel
 * po inicializaci.
 * Rušené prvky seznamu budou korektně uvolněny voláním operace free.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_Dispose( DLList *list ) {
	// dispose v prípade, že list nie je prázdny 
	if (list != NULL){ //
		DLLElementPtr tmp;
		// keďže list nebol prázdny cyklenie dokým bude 
		while(list->firstElement != NULL){ 
			// uloží nasledujúci prvok, uvoľní Element a nahrá nasledujúci prvok pomocou temporary premennej 
			tmp = list->firstElement->nextElement;
			free(list->firstElement);
			list->firstElement = tmp;
		}
		
		list->activeElement = NULL;
		list->firstElement = NULL;
		list->lastElement = NULL;
	}
	else{
		// prázdny list --> splnená úloha funkcie 
		return;
	}
}

/**
 * Vloží nový prvek na začátek seznamu list.
 * V případě, že není dostatek paměti pro nový prvek při operaci malloc,
 * volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Hodnota k vložení na začátek seznamu
 */
void DLL_InsertFirst( DLList *list, int data ) { 
	//pridelenie pamäti + overenie či dej prebehol úspešne 
	DLLElementPtr new = malloc(sizeof(struct DLLElement));
	if (new == NULL){
		DLL_Error();
		return;
	}

	// nastavenie parametrov pre nový element
	new->data = data; 
	// nextElement novo vloženého elementu je aktuálnym prvým elementom listu
	new->nextElement = list->firstElement; 
	//lebo bude vložený ako prvý
	new->previousElement = NULL; 

	// if statement, či pracuje funkcia s prázdnym listom 
	if (list->firstElement != NULL){
		// ak nepracuje s prádznym listom, predchádzajúci prvok aktuálneho prvého prvku sa aktualizuje tak, aby ukazoval na nový prvok.
		list->firstElement->previousElement = new;
	}
	else{
		//ak pracuje s prádznym listom, tak poslesdnou položkou je novo vložená poloťka
		list->lastElement = new;
	}

	//nakoniec vloží položku 
	list->firstElement = new;
}

/**
 * Vloží nový prvek na konec seznamu list (symetrická operace k DLL_InsertFirst).
 * V případě, že není dostatek paměti pro nový prvek při operaci malloc,
 * volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Hodnota k vložení na konec seznamu
 */
void DLL_InsertLast( DLList *list, int data ) {
	//rovnaký postup ako u funkcie DLL_InsertFirst len opačný smer listu 
	//pridelenie pamäti + overenie či dej prebehol úspešne 
	DLLElementPtr new = malloc(sizeof(struct DLLElement));
	if (new == NULL){
		DLL_Error();
		return;
	}

	// nastavenie parametrov pre nový element
	new->data=data;
	// previousElement novo vloženej poloťky je akutálnou poslednou položkou 
	new->previousElement = list->lastElement;
	// nasledujúci element neexistuje, lebo bude vložený ako posledný
	new->nextElement = NULL;
	
	//if statement, či pracuje funkcia s prázdnym listom
	if (list->lastElement != NULL){
		// ak nepracuje s prádznym listom,tak v nasledujúci prvok v aktuálnom liste bude novo-vložená položka
		list->lastElement->nextElement = new; 
	}
	else{
		// ak pracuje s prádznym listom, tak prvý je tak isto posledný
		list->firstElement = new;
	}
	// vloženie položky na posledné miesto 
	list->lastElement = new;
}

/**
 * Nastaví první prvek seznamu list jako aktivní.
 * Funkci implementujte jako jediný příkaz, aniž byste testovali,
 * zda je seznam list prázdný.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_First( DLList *list ) {
	list->activeElement = list->firstElement;
}

/**
 * Nastaví poslední prvek seznamu list jako aktivní.
 * Funkci implementujte jako jediný příkaz, aniž byste testovali,
 * zda je seznam list prázdný.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_Last( DLList *list ) {
	list->activeElement = list->lastElement;
}

/**
 * Prostřednictvím parametru dataPtr vrátí hodnotu prvního prvku seznamu list.
 * Pokud je seznam list prázdný, volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param dataPtr Ukazatel na cílovou proměnnou
 */
void DLL_GetFirst( DLList *list, int *dataPtr ) {
	//kontrola či má funkcia všetky potrebné parametre pre vykonanie svojej funkcionality 
	if (list == NULL || list->firstElement == NULL || dataPtr == NULL) {
		DLL_Error();
		return;
	}
	else{
		*dataPtr = list->firstElement->data;
	}
}

/**
 * Prostřednictvím parametru dataPtr vrátí hodnotu posledního prvku seznamu list.
 * Pokud je seznam list prázdný, volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param dataPtr Ukazatel na cílovou proměnnou
 */
void DLL_GetLast( DLList *list, int *dataPtr ) {
	//kontrola či má funkcia všetky potrebné parametre pre vykonanie svojej funkcionality 
	if (list == NULL || list->lastElement == NULL || dataPtr == NULL){
		DLL_Error();
		return;
	} 
	else{
		*dataPtr = list->lastElement->data;
	}

}

/**
 * Zruší první prvek seznamu list.
 * Pokud byl první prvek aktivní, aktivita se ztrácí.
 * Pokud byl seznam list prázdný, nic se neděje.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_DeleteFirst( DLList *list ) {
	DLLElementPtr tmp;
	//ak existuje prvý prvok
	if(list->firstElement != NULL){
		tmp = list->firstElement->nextElement;
		if (tmp != NULL){
			tmp->previousElement = NULL;
		}
		else{
			// ak bol 1 element v liste 
			list->lastElement = NULL;
		}
		free(list->firstElement);
		list->firstElement= tmp;
	}
}

/**
 * Zruší poslední prvek seznamu list.
 * Pokud byl poslední prvek aktivní, aktivita seznamu se ztrácí.
 * Pokud byl seznam list prázdný, nic se neděje.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_DeleteLast( DLList *list ) {
	DLLElementPtr tmp;
	// ak existuje posledný prvok
	if (list->lastElement != NULL){
		tmp = list->lastElement->previousElement;
		if(tmp != NULL){
			tmp->nextElement = NULL;
		}
		else{
			// ak by bol 1 element v liste
			list->firstElement = NULL;
		}
		free(list->lastElement);
		list->lastElement = tmp;
	}
}

/**
 * Zruší prvek seznamu list za aktivním prvkem.
 * Pokud je seznam list neaktivní nebo pokud je aktivní prvek
 * posledním prvkem seznamu, nic se neděje.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_DeleteAfter( DLList *list ) {
	//kontrola aktívneho elementu, či existuje a či nie je posledný
	if(list->activeElement == NULL || list->activeElement == list->lastElement){
		return;
	}

	//zoberiem si prvok za aktívnym prvkom 
	DLLElementPtr tmp = list->activeElement->nextElement;

	if (tmp == NULL){
		//ak nie je žiadny za aktívnym
		return;
	}
	// Aktualizácia ďalšieho odkazu aktívneho prvku
	list->activeElement->nextElement = tmp->nextElement;
	// Ak je prvok, ktorý sa má odstrániť, posledný, aktualizuje funkcia ukazovateľ posledného prvku
	if(tmp == list->lastElement){
		list->lastElement = list->activeElement;
	}
	else{
		// ak nie, aktualizuje predchádzajúci odkaz nasledujúceho prvku
		tmp->nextElement->previousElement = list->activeElement;
	}
	free(tmp);
}

/**
 * Zruší prvek před aktivním prvkem seznamu list .
 * Pokud je seznam list neaktivní nebo pokud je aktivní prvek
 * prvním prvkem seznamu, nic se neděje.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_DeleteBefore( DLList *list ) {
	//kontrola aktívneho elementu, či existuje a či nie je prvým
	if(list->activeElement == NULL || list->activeElement == list->firstElement){
		return;
	}
	DLLElementPtr tmp = list->activeElement->previousElement;
	if(tmp == NULL){
		//neexistuje element pred aktívnym
		return; 
	}
	//aktualizácia predošlého odkazu aktívneho prvku
	list->activeElement->previousElement = tmp->previousElement;
	// ak je prvok, ktorý sa má odstrániť prvý, aktualizuje funkcie ukazovateľ prvého prvku
	if (tmp == list->firstElement){
		list->firstElement = list->activeElement;
	}
	else{
		//ak nie, aktualizuje nasledujúci odkaz predošlého prvku
		tmp->previousElement->nextElement = list->activeElement;
	}
	free(tmp);
}

/**
 * Vloží prvek za aktivní prvek seznamu list.
 * Pokud nebyl seznam list aktivní, nic se neděje.
 * V případě, že není dostatek paměti pro nový prvek při operaci malloc,
 * volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Hodnota k vložení do seznamu za právě aktivní prvek
 */
void DLL_InsertAfter( DLList *list, int data ) {
	// kontrola existencie aktívneho prvku
	if (list->activeElement == NULL){
		return;
	}

	//alokácia miesta pre novž element + check 
	DLLElementPtr new = malloc (sizeof(struct DLLElement));
	if (new == NULL){
		DLL_Error();
		return;
	}

	//nastavenie parametrov pre nový element 
	new->data = data;
	new->nextElement = list->activeElement->nextElement;
	new->previousElement = list->activeElement;
	//ak je aktívny element posledný, aktualizácia ukazovateľa pre posledný element
	if(list->activeElement == list->lastElement){
		list->lastElement = new;
	}
	else{
		//v opačnom prípade aktualizuje ukazovateľ prvku za aktívnym prvkom
		list->activeElement->nextElement->previousElement = new;
	}
	list->activeElement->nextElement = new;
}

/**
 * Vloží prvek před aktivní prvek seznamu list.
 * Pokud nebyl seznam list aktivní, nic se neděje.
 * V případě, že není dostatek paměti pro nový prvek při operaci malloc,
 * volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Hodnota k vložení do seznamu před právě aktivní prvek
 */
void DLL_InsertBefore( DLList *list, int data ) {
	//kontrola existencie aktívneho prvku + check
	if(list->activeElement){
		DLLElementPtr new = malloc(sizeof(struct DLLElement));
		if (new == NULL){
			DLL_Error();
			return;
		}

		//nastavenie parametrov pre nový element
		new->data = data;
		new->nextElement = list->activeElement;
		new->previousElement = list->activeElement->previousElement;

		// ak je aktívny element prvým prvko, aktualizácia akuzatovateľa pre prvý prvok zoznamu
		if(list->activeElement == list->firstElement){
			list->firstElement = new;
		}
		else{
			//v opačnom prípade priradí ukazateľ pprvku pred aktívny prvok
			list->activeElement->previousElement->nextElement = new;
		}
		//vloženie nového elementu pred aktívny
		list->activeElement->previousElement = new;
	}
}

/**
 * Prostřednictvím parametru dataPtr vrátí hodnotu aktivního prvku seznamu list.
 * Pokud seznam list není aktivní, volá funkci DLL_Error ().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param dataPtr Ukazatel na cílovou proměnnou
 */
void DLL_GetValue( DLList *list, int *dataPtr ) {
	//kontrola existencie aktívneho prvku
	if(list->activeElement == NULL){
		DLL_Error();
	}
	else{
		//pomocou dataPtr vráti hodnotu 
		*dataPtr = list->activeElement->data;
	}
}

/**
 * Přepíše obsah aktivního prvku seznamu list.
 * Pokud seznam list není aktivní, nedělá nic.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Nová hodnota právě aktivního prvku
 */
void DLL_SetValue( DLList *list, int data ) {
	//kontrola existencie aktívneho prvku
	if(list->activeElement == NULL){
		return;
	}
	else{
		//bez kontroly, či data je NULL pretože môžeme chcieť nastaviť prvok na NULL
		list->activeElement->data = data;
	}
}

/**
 * Posune aktivitu na následující prvek seznamu list.
 * Není-li seznam aktivní, nedělá nic.
 * Všimněte si, že při aktivitě na posledním prvku se seznam stane neaktivním.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_Next( DLList *list ) {
	if(list->activeElement != NULL){
		list->activeElement = list->activeElement->nextElement;
	}
}


/**
 * Posune aktivitu na předchozí prvek seznamu list.
 * Není-li seznam aktivní, nedělá nic.
 * Všimněte si, že při aktivitě na prvním prvku se seznam stane neaktivním.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_Previous( DLList *list ) {
	if(list->activeElement != NULL){
		list->activeElement = list->activeElement->previousElement;
	}
}

/**
 * Je-li seznam list aktivní, vrací nenulovou hodnotu, jinak vrací 0.
 * Funkci je vhodné implementovat jedním příkazem return.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 *
 * @returns Nenulovou hodnotu v případě aktivity prvku seznamu, jinak nulu
 */
int DLL_IsActive(DLList *list)
{
    if (list == NULL){
		return 0;
	} // or handle error as appropriate
    return list->activeElement != NULL;
}


/* Konec c206.c */
