/* ******************************* c204.c *********************************** */
/*  Předmět: Algoritmy (IAL) - FIT VUT v Brně                                 */
/*  Úkol: c204 - Převod infixového výrazu na postfixový (s využitím c202)     */
/*  Referenční implementace: Petr Přikryl, listopad 1994                      */
/*  Přepis do jazyka C: Lukáš Maršík, prosinec 2012                           */
/*  Upravil: Kamil Jeřábek, září 2019                                         */
/*           Daniel Dolejška, září 2021                                       */
/* ************************************************************************** */
/*
** Implementujte proceduru pro převod infixového zápisu matematického výrazu
** do postfixového tvaru. Pro převod využijte zásobník (Stack), který byl
** implementován v rámci příkladu c202. Bez správného vyřešení příkladu c202
** se o řešení tohoto příkladu nepokoušejte.
**
** Implementujte následující funkci:
**
**    infix2postfix ... konverzní funkce pro převod infixového výrazu
**                      na postfixový
**
** Pro lepší přehlednost kódu implementujte následující pomocné funkce:
**    
**    untilLeftPar ... vyprázdnění zásobníku až po levou závorku
**    doOperation .... zpracování operátoru konvertovaného výrazu
**
** Své řešení účelně komentujte.
**
** Terminologická poznámka: Jazyk C nepoužívá pojem procedura.
** Proto zde používáme pojem funkce i pro operace, které by byly
** v algoritmickém jazyce Pascalovského typu implemenovány jako procedury
** (v jazyce C procedurám odpovídají funkce vracející typ void).
**
**/

#include "c204.h"

bool solved;

/**
 * Pomocná funkce untilLeftPar.
 * Slouží k vyprázdnění zásobníku až po levou závorku, přičemž levá závorka bude
 * také odstraněna.
 * Pokud je zásobník prázdný, provádění funkce se ukončí.
 *
 * Operátory odstraňované ze zásobníku postupně vkládejte do výstupního pole
 * znaků postfixExpression.
 * Délka převedeného výrazu a též ukazatel na první volné místo, na které se má
 * zapisovat, představuje parametr postfixExpressionLength.
 *
 * Aby se minimalizoval počet přístupů ke struktuře zásobníku, můžete zde
 * nadeklarovat a používat pomocnou proměnnou typu char.
 *
 * @param stack Ukazatel na inicializovanou strukturu zásobníku
 * @param postfixExpression Znakový řetězec obsahující výsledný postfixový výraz
 * @param postfixExpressionLength Ukazatel na aktuální délku výsledného postfixového výrazu
 */
void untilLeftPar( Stack *stack, char *postfixExpression, unsigned *postfixExpressionLength ) {
	
	char t; //premenna na ukayanie vrcholu zasobniku
	Stack_Top(stack,&t);
	
	

	while(t != '(' && Stack_IsEmpty(stack) != 0){ //cyklus ktory vypise na postfix prvy zasobniku a vyprazdni zasobnik
		
		Stack_Top(stack, &t);
		Stack_Top(stack,&postfixExpression[*postfixExpressionLength]);
		Stack_Pop(stack);
		(*postfixExpressionLength)++;

	}
	
	Stack_Pop(stack);//popne zo zasobniku poslednu zavorku

}

/**
 * Pomocná funkce doOperation.
 * Zpracuje operátor, který je předán parametrem c po načtení znaku ze
 * vstupního pole znaků.
 *
 * Dle priority předaného operátoru a případně priority operátoru na vrcholu
 * zásobníku rozhodneme o dalším postupu.
 * Délka převedeného výrazu a taktéž ukazatel na první volné místo, do kterého
 * se má zapisovat, představuje parametr postfixExpressionLength, výstupním
 * polem znaků je opět postfixExpression.
 *
 * @param stack Ukazatel na inicializovanou strukturu zásobníku
 * @param c Znak operátoru ve výrazu
 * @param postfixExpression Znakový řetězec obsahující výsledný postfixový výraz
 * @param postfixExpressionLength Ukazatel na aktuální délku výsledného postfixového výrazu
 */
void doOperation( Stack *stack, char c, char *postfixExpression, unsigned *postfixExpressionLength ) {

char x;


if (Stack_IsEmpty(stack)){ // ak je zasobnik prazdnny pushne na zasobnik prvok c

	Stack_Push(stack,c);
	return;

}else{Stack_Top(stack, &x);


if(x == '('){  //ak je na stacku lava zavtorka pushne na zasobnik prvok c
		
	Stack_Push(stack,c);
	return;
}



else if ((x == '-' && x != c && c!= '+') || (x == '+' && x !=c && c!='-') ){ // pushne na zasobnik prvok s mensou prioritou

	Stack_Push(stack,c);
	return;

}

else {//ak je na vrcholu zasobniku prvok s rovnakou alebo vyssou prioritou tak odstrani najvyssi prvok a prida ho do postfix zoznamu

	postfixExpression[*postfixExpressionLength] = x;
	Stack_Pop(stack);
	(*postfixExpressionLength)++;
	doOperation(stack,c,postfixExpression, postfixExpressionLength ); //rekurzivne zavola samu seba
	return;
}

}	
	
		
}


/**
 * Konverzní funkce infix2postfix.
 * Čte infixový výraz ze vstupního řetězce infixExpression a generuje
 * odpovídající postfixový výraz do výstupního řetězce (postup převodu hledejte
 * v přednáškách nebo ve studijní opoře).
 * Paměť pro výstupní řetězec (o velikosti MAX_LEN) je třeba alokovat. Volající
 * funkce, tedy příjemce konvertovaného řetězce, zajistí korektní uvolnění zde
 * alokované paměti.
 *
 * Tvar výrazu:
 * 1. Výraz obsahuje operátory + - * / ve významu sčítání, odčítání,
 *    násobení a dělení. Sčítání má stejnou prioritu jako odčítání,
 *    násobení má stejnou prioritu jako dělení. Priorita násobení je
 *    větší než priorita sčítání. Všechny operátory jsou binární
 *    (neuvažujte unární mínus).
 *
 * 2. Hodnoty ve výrazu jsou reprezentovány jednoznakovými identifikátory
 *    a číslicemi - 0..9, a..z, A..Z (velikost písmen se rozlišuje).
 *
 * 3. Ve výrazu může být použit předem neurčený počet dvojic kulatých
 *    závorek. Uvažujte, že vstupní výraz je zapsán správně (neošetřujte
 *    chybné zadání výrazu).
 *
 * 4. Každý korektně zapsaný výraz (infixový i postfixový) musí být uzavřen
 *    ukončovacím znakem '='.
 *
 * 5. Při stejné prioritě operátorů se výraz vyhodnocuje zleva doprava.
 *
 * Poznámky k implementaci
 * -----------------------
 * Jako zásobník použijte zásobník znaků Stack implementovaný v příkladu c202.
 * Pro práci se zásobníkem pak používejte výhradně operace z jeho rozhraní.
 *
 * Při implementaci využijte pomocné funkce untilLeftPar a doOperation.
 *
 * Řetězcem (infixového a postfixového výrazu) je zde myšleno pole znaků typu
 * char, jenž je korektně ukončeno nulovým znakem dle zvyklostí jazyka C.
 *
 * Na vstupu očekávejte pouze korektně zapsané a ukončené výrazy. Jejich délka
 * nepřesáhne MAX_LEN-1 (MAX_LEN i s nulovým znakem) a tedy i výsledný výraz
 * by se měl vejít do alokovaného pole. Po alokaci dynamické paměti si vždycky
 * ověřte, že se alokace skutečně zdrařila. V případě chyby alokace vraťte namísto
 * řetězce konstantu NULL.
 *
 * @param infixExpression vstupní znakový řetězec obsahující infixový výraz k převedení
 *
 * @returns znakový řetězec obsahující výsledný postfixový výraz
 */
char *infix2postfix( const char *infixExpression ) {
	
	int v;
	v = 0;
	
	Stack *stack = malloc(MAX_LEN); // alocuje miesto pre stack
	if (stack == NULL) //kontrola ci prebehol malloc spravne
	{
		return NULL;
	}
	
	Stack_Init(stack);

	char* PFM = malloc(MAX_LEN);// alocuje miesto pre postfix string (PFM = postfixstring)
	if (PFM == NULL)//kontrola ci prebehol malloc spravne
	{
		return NULL;
	}
	

	for (int i = 0; infixExpression[i]  != '\0'; i++) //cyklus ktory sa posuva v infix retazci 
	{
		if (infixExpression[i] >='0' && infixExpression[i] <='9') // kontroluje ci je operand cislo
		{
			PFM[v] = infixExpression[i]; //zapisuje cisla na postfix zoznam
			v++; //posuva index v postfix zozname
		
		} else if ((infixExpression[i] >='a' && infixExpression[i] <='z') || (infixExpression[i] >='A' && infixExpression[i] <='Z')) // kontroluje ci je operand pismeno
		{
			PFM[v] = infixExpression[i]; // zapisuje pismena na postfix zoznam
			v++;//posuva index v postfix zozname

		} else if (infixExpression[i] == '+' || infixExpression[i] ==  '-' || infixExpression[i] == '*' || infixExpression[i] == '/')
		{
			doOperation(stack, infixExpression[i], PFM, &v ); // vola funkciu doOperation
			
			
		} else if (infixExpression[i] == '(') // kontrluje ci je znak lava zavorka
		{
			Stack_Push(stack,infixExpression[i]); // pushne lavu zavorku na stack
			
		} else if(infixExpression[i] == ')') // kontroluje ci je znak prava zavorka
		{
			
			untilLeftPar(stack, PFM, &v);// vola funkciu untilLeftPar
			
		}else if (infixExpression[i] == '=') // kontroluje ci je znak =
		{ 
			while(Stack_IsEmpty(stack) != 1){//zakial stack neni prazdny pokracuje cyklus

					Stack_Top(stack,&PFM[v]); //ulozi najvyssi prvok zasobniku na postfix zoznam
					Stack_Pop(stack);//vyhodi najvyssi prvok zasobniku
					v++;
			}
			PFM[v]='='; // prida na koniec postfix zoznamu =
		}
	}

	
		 

	return PFM;
}


/**
 * Pomocná metoda pro vložení celočíselné hodnoty na zásobník.
 *
 * Použitá implementace zásobníku aktuálně umožňuje vkládání pouze
 * hodnot o velikosti jednoho byte (char). Využijte této metody
 * k rozdělení a postupné vložení celočíselné (čtyřbytové) hodnoty
 * na vrchol poskytnutého zásobníku.
 *
 * @param stack ukazatel na inicializovanou strukturu zásobníku
 * @param value hodnota k vložení na zásobník
 */
void expr_value_push( Stack *stack, int value ) {
    for(int i = 3; i>= 0; i--){
        char byte = (value >> (i * 8)) & 0xFF; //extrahujem bajt (byte) z integeru
        Stack_Push(stack, byte); //pushnem po bytoch
    }

}

/**
 * Pomocná metoda pro extrakci celočíselné hodnoty ze zásobníku.
 *
 * Využijte této metody k opětovnému načtení a složení celočíselné
 * hodnoty z aktuálního vrcholu poskytnutého zásobníku. Implementujte
 * tedy algoritmus opačný k algoritmu použitému v metodě
 * `expr_value_push`.
 *
 * @param stack ukazatel na inicializovanou strukturu zásobníku
 * @param value ukazatel na celočíselnou proměnnou pro uložení
 *   výsledné celočíselné hodnoty z vrcholu zásobníku
 */
void expr_value_pop(Stack *stack, int *value) {
    *value = 0;
    for (int i = 0; i < 4; i++) { //loop na zber všetkých 4 bajty
        char byte;
        if (Stack_IsEmpty(stack)) {
            // Handle error: stack underflow
            *value = 0;
            return;
        }
        // spojenie naspäť do integeru
        Stack_Top(stack, &byte);
        Stack_Pop(stack);
        *value |= (byte & 0xFF) << (i * 8);
    }
}



/**
 * Tato metoda provede vyhodnocení výrazu zadaném v `infixExpression`,
 * kde hodnoty proměnných použitých v daném výrazu jsou definovány
 * v poli `variableValues`.
 *
 * K vyhodnocení vstupního výrazu využijte implementaci zásobníku
 * ze cvičení c202. Dále také využijte pomocných funkcí `expr_value_push`,
 * respektive `expr_value_pop`. Při řešení si můžete definovat libovolné
 * množství vlastních pomocných funkcí.
 *
 * Předpokládejte, že hodnoty budou vždy definovány
 * pro všechy proměnné použité ve vstupním výrazu.
 *
 * @param infixExpression vstpní infixový výraz s proměnnými
 * @param variableValues hodnoty proměnných ze vstupního výrazu
 * @param variableValueCount počet hodnot (unikátních proměnných
 *   ve vstupním výrazu)
 * @param value ukazatel na celočíselnou proměnnou pro uložení
 *   výsledné hodnoty vyhodnocení vstupního výrazu
 *
 * @return výsledek vyhodnocení daného výrazu na základě poskytnutých hodnot proměnných
 */

bool eval(const char *infixExpression, VariableValue variableValues[], int variableValueCount, int *value) {
    char *postfixExpression = infix2postfix(infixExpression);
    if (!postfixExpression) {
        return false;
    }

    Stack stack;
    Stack_Init(&stack);

    char *ptr = postfixExpression; // pointer na prechádzanie výrazom
    while (*ptr) {  //loop cez postfix výraz
        if ((*ptr >= 'a' && *ptr <= 'z') || (*ptr >='A' && *ptr <= 'Z')) { // Variable
            for (int i = 0; i < variableValueCount; i++) {
                if (variableValues[i].name == *ptr) {
                    expr_value_push(&stack, variableValues[i].value);
                    break;
                }
            }
        } else if (*ptr == '+' || *ptr == '-' || *ptr == '*' || *ptr == '/') {
            int operand2, operand1;
            expr_value_pop(&stack, &operand2);
            expr_value_pop(&stack, &operand1);

            switch (*ptr) {
                case '+':
                    expr_value_push(&stack, operand1 + operand2);
                    break;
                case '-':
                    expr_value_push(&stack, operand1 - operand2);
                    break;
                case '*':
                    expr_value_push(&stack, operand1 * operand2);
                    break;
                case '/':
                    if (operand2 == 0) {
                        free(postfixExpression);
                        return false; // Division by zero
                    }
                    expr_value_push(&stack, operand1 / operand2);
                    break;
            }
        }
        ptr++;
    }

    expr_value_pop(&stack, value);
    free(postfixExpression);
    return true;
}


/* Konec c204.c */
