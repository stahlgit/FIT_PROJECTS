/**
 * Kostra programu pro 2. projekt IZP 2022/23
 *
 * Jednoducha shlukova analyza: 2D nejblizsi soused.
 * Single linkage
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h> // sqrtf
#include <limits.h> // INT_MAX


/*****************************************************************
 * Ladici makra. Vypnout jejich efekt lze definici makra NDEBUG,
 * napr.:
 *   a) pri prekladu argumentem prekladaci -DNDEBUG
 *   b) v souboru (na radek pred #include <assert.h>
 *      #define NDEBUG
 */

/*****************************************************************
 * Ladici makra. Vypnout jejich efekt lze definici makra
 * NDEBUG, napr.:
 *   a) pri prekladu argumentem prekladaci -DNDEBUG
 *   b) v souboru (na radek pred #include <assert.h>
 *      #define NDEBUG
 */
#ifdef NDEBUG
#define debug(s)
#define dfmt(s, ...)
#define dint(i)
#define dfloat(f)
#else

// vypise ladici retezec
#define debug(s) printf("- %s\n", s)

// vypise formatovany ladici vystup - pouziti podobne jako printf
#define dfmt(s, ...) printf(" - "__FILE__":%u: "s"\n",__LINE__,__VA_ARGS__)

// vypise ladici informaci o promenne - pouziti dint(identifikator_promenne)
#define dint(i) printf(" - " __FILE__ ":%u: " #i " = %d\n", __LINE__, i)

// vypise ladici informaci o promenne typu float - pouziti
// dfloat(identifikator_promenne)
#define dfloat(f) printf(" - " __FILE__ ":%u: " #f " = %g\n", __LINE__, f)

#endif

/*****************************************************************
 * Deklarace potrebnych datovych typu:
 *
 * TYTO DEKLARACE NEMENTE
 *
 *   struct obj_t - struktura objektu: identifikator a souradnice
 *   struct cluster_t - shluk objektu:
 *      pocet objektu ve shluku,
 *      kapacita shluku (pocet objektu, pro ktere je rezervovano
 *          misto v poli),
 *      ukazatel na pole shluku.
 */
struct obj_t {
    int id;
    float x;
    float y;
};

struct cluster_t {
    int size;
    int capacity;
    struct obj_t *obj;
};

// -- GLOBALNE KONSTATY -- //

/// hodnota pre reaokizaciu shluku
const int CLUSTER_CHUNK = 10;

/*****************************************************************
 * Deklarace potrebnych funkci.
 *
 * PROTOTYPY FUNKCI NEMENTE
 *
 * IMPLEMENTUJTE POUZE FUNKCE NA MISTECH OZNACENYCH 'TODO'
 *
 */



// -- DEKLARACIA FUNKCII -- //


/////////////// Prace so shlukmi ///////////////

/*
 Inicializace shluku 'c'. Alokuje pamet pro cap objektu (kapacitu).
 Ukazatel NULL u pole objektu znamena kapacitu 0.
*/
void init_cluster(struct cluster_t *c, int cap)
{
    assert(c);
    assert(cap >= 0);

    c->size = 0;
    if (cap > 0)
    {
        if ((c->obj = malloc(cap * sizeof(struct obj_t))))  // alokacia pameti pre pole objektov
        {
            c->capacity = cap;
            return;
        }
    }
    c->capacity = 0; // neuspech alokacie --> kapacita = 0
    c->obj = NULL;
}


/*
 Odstranenie vsetkych objektov zhluku ++ inicializacia na prazdny zhluk.
 */
void clear_cluster(struct cluster_t *c)
{
    assert(c);

    free(c->obj); // uvolnenie pamate pole objektov
    init_cluster(c, 0);// inicializacia na prazdny zhluk
}
/*
 Odstranenie pola zhluku.
 */
void clear_clusters(struct cluster_t *carr, const int narr)
{
    assert(carr);
    assert(narr >= 0);


    for (int i = 0; i < narr; i++) // uvolneni pamate objektov pola
    {
        clear_cluster(&carr[i]);
    }
    free(carr); // uvolnenie pameta pola zhluku
}

/*
 Zmena kapacity zhluku 'c' na kapacitu 'new_cap'.
 */
struct cluster_t *resize_cluster(struct cluster_t *c, int new_cap)
{
    // TUTO FUNKCI NEMENTE
    assert(c);
    assert(c->capacity >= 0);
    assert(new_cap >= 0);

    if (c->capacity >= new_cap)
        return c;

    size_t size = sizeof(struct obj_t) * new_cap;

    void *arr = realloc(c->obj, size);
    if (arr == NULL)
        return NULL;

    c->obj = arr;
    c->capacity = new_cap;
    return c;
}


/*
 Prida objekt 'obj' na konec shluku 'c'. Rozsiri shluk, pokud se do nej objekt
 nevejde.
 */
void append_cluster(struct cluster_t *c, struct obj_t obj)
{
    assert(c);
    assert(c->size >= 0);

    if (c->capacity <= c->size)
    {
        if ( ! resize_cluster(c, c->capacity + CLUSTER_CHUNK)) // rozsirenie kapacity zhluku
        {
            return;
        }
    }

    assert(c->obj);

    c->obj[c->size++] = obj; // pridanie objektu na prvu volnu poziciu a +1 pocet prvkov
}

// pomocna funkce pro razeni shluku
static int obj_sort_compar(const void *a, const void *b)
{
    const struct obj_t *o1 = a;
    const struct obj_t *o2 = b;
    if (o1->id < o2->id) return -1;
    if (o1->id > o2->id) return 1;
    return 0;
}

/*
 Razeni objektu ve shluku vzestupne podle jejich identifikatoru.
*/
void sort_cluster(struct cluster_t *c)
{
    qsort(c->obj, c->size, sizeof(struct obj_t), &obj_sort_compar);
}

/*
 Do shluku 'c1' prida objekty 'c2'. Shluk 'c1' bude v pripade nutnosti rozsiren.
 Objekty ve shluku 'c1' budou serazeny vzestupne podle identifikacniho cisla.
 Shluk 'c2' bude nezmenen.
 */
void merge_clusters(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c2 != NULL);
    assert(c2->size >= 0);
    if (c2->size != 0)
    {
        assert(c2->obj);
    }

    int previous_c1_size = c1->size;
    for (int i = 0; i < c2->size; i++)// pridanie objektu zhluku `c2` do zhluku `c1`
    {
        append_cluster(c1, c2->obj[i]);
    }
    if (c2->size > 0 && c1->size == previous_c1_size + c2->size)// zoradenie objektu v zhluku
    {
        sort_cluster(c1);
    }
}

//Hlada objekt podla jeho ID v zhluku
struct obj_t *cluster_obj_id(const struct cluster_t *c, const int id)
{
    assert(c);
    assert(c->size >= 0);
    if (c->size != 0) assert(c->obj);

    // prochazeni objektu a porovnavani jejich ID
    for (int i = 0; i < c->size; i++)
    {
        if (c->obj[i].id == id)
        {
            return &c->obj[i];
        }
    }

    return NULL;
}


/*
 Vypis zhluku 'c' na stdout.
*/
void print_cluster(struct cluster_t *c)
{
    for (int i = 0; i < c->size; i++)
    {
        if (i) putchar(' ');
        printf("%d[%g,%g]", c->obj[i].id, c->obj[i].x, c->obj[i].y);
    }
    putchar('\n');
}

//-- PRACA S POLOM -- //

/**
 Inicializace pole shluku, alokuje pamet pro `narr` shluku.
 */
void init_clusters(struct cluster_t **carr, const int narr)
{
    assert(carr);
    assert(narr >= 0);

    if ( ! (*carr = malloc(narr * sizeof(struct cluster_t)))) //alokacia pamate pre pole zhluku
    {
        return;
    }
    for (int i = 0; i < narr; i++) //inicializacia objektu pola
    {
        init_cluster(&(*carr)[i], 0);
    }
}

/*
 Odstrani shluk z pole shluku 'carr'. Pole shluku obsahuje 'narr' polozek
 (shluku). Shluk pro odstraneni se nachazi na indexu 'idx'. Funkce vraci novy
 pocet shluku v poli.
*/
int remove_cluster(struct cluster_t *carr, int narr, int idx)
{
    assert(carr);
    assert(narr > 0);
    assert(idx >= 0);
    assert(idx < narr);

    int new_narr = narr - 1; //nova veÄžkosĹĽ o 1 mensia
    clear_cluster(&carr[idx]); //uvolenenie pamati pre zhluk na index idc
    for (int i = idx; i < new_narr; i++) //posunutie zhluku v poli od indexu idx do lava
    {
        carr[i] = carr[i + 1];
    }

    return new_narr;
}


/*
 Pocita Euklidovskou vzdalenost mezi dvema objekty.
 */
float obj_distance(struct obj_t *o1, struct obj_t *o2)
{
    assert(o1);
    assert(o2);
    return sqrtf(powf(o1->x - o2->x, 2.0) + powf(o1->y - o2->y, 2.0)); //vypocet Euklidovej vzdialenosti medzi objektmi
}


/*
 Pocita vzdalenost dvou shluku.
*/
float cluster_distance(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1);
    assert(c1->size > 0);
    assert(c1->obj);
    assert(c2);
    assert(c2->size > 0);
    assert(c2->obj);

    float distance;
    for (int i = 0; i < c1->size; i++) //vypocet vzdialenosti vsetkych objektov z zhluku 'c1' so vsetkymi objektmi zo zhluku 'c2'
    {
        for (int j = i; j < c2->size; j++)
        {
            distance = obj_distance(&c1->obj[i], &c2->obj[j]); //vypocet vzdialenosti
        }
    }

    return distance;
}
/*
 Funkce najde dva nejblizsi shluky. V poli shluku 'carr' o velikosti 'narr'
 hleda dva nejblizsi shluky. Nalezene shluky identifikuje jejich indexy v poli
 'carr'. Funkce nalezene shluky (indexy do pole 'carr') uklada do pameti na
 adresu 'c1' resp. 'c2'.
*/
void find_neighbours(struct cluster_t *carr, int narr, int *c1, int *c2)
{
    assert(carr);
    assert(narr > 0);
    assert(c1);
    assert(c2);

    if (narr == 1)
    {
        *c1 = *c2 = 0;
        return;
    }
    float min_distance = -1, distance;
    for (int i = 0; i < narr; i++) //vzajomne pocitanie vzdialenosti medzi vsetkymi zhlukmi
    {
        for (int j = i + 1; j < narr; j++)
        {
            distance = cluster_distance(&carr[i], &carr[j]); //vypocet vzdialenosti
            if (min_distance == -1 || distance < min_distance) //hladanie minimalnej vzdialenosti
            {
                min_distance = distance;
                *c1 = i;
                *c2 = j;
            }
        }
    }
}


/**
 Kontrola originality ID
 */
struct obj_t *array_obj_id(const struct cluster_t *carr, const int narr, const int id)
{
    assert(carr);
    assert(narr >= 0);

    struct obj_t *obj = NULL;
    for (int i = 0; i < narr; i++) //prechadza pole zhluku a vyhladava objekty podla ID
    {
        if ((obj = cluster_obj_id(&carr[i], id)))
        {
            break;
        }
    }

    return obj;
}
/*
 Ze souboru 'filename' nacte objekty. Pro kazdy objekt vytvori shluk a ulozi
 jej do pole shluku. Alokuje prostor pro pole vsech shluku a ukazatel na prvni
 polozku pole (ukalazatel na prvni shluk v alokovanem poli) ulozi do pameti,
 kam se odkazuje parametr 'arr'. Funkce vraci pocet nactenych objektu (shluku).
 V pripade nejake chyby uklada do pameti, kam se odkazuje 'arr', hodnotu NULL.
*/
int load_clusters(char *filename, struct cluster_t **arr)
{
    assert(arr);

    *arr = NULL;

    FILE *file;
    file = fopen(filename, "r"); //otvorenie subore pre citanie
    if (!file)
    {
        printf("Chyba pri otvoreni suboru: %s.\n", filename);
        return -1;
    }

    // spocitani C velikosti radku
    //int max_len = 12; // max pocet znakov na riadok
    //char line[max_len];

    int fcount=1, file_obj_count = 0, obj_x, obj_y, obj_id;
    struct obj_t obj;
    struct cluster_t *cluster;
    //char pEnd;

    fscanf(file, "count=%d\n", &file_obj_count); //prvy riadok -> nacitanie poctu objektov
    //ak je viac objektov ako je zadane skratka sa nenacitaju
    if(file_obj_count%1 != 0)
    {
        printf("Chybny format poctu: count=%d.\n", file_obj_count);
        fclose(file);
        return -1;
    }
    if (file_obj_count <= 0)
    {
        printf("Hodnota poctu objektov: count=%d , musi byt vacsia ako 0\n", file_obj_count);
    }
    init_clusters(arr, file_obj_count); //alokacia pamati pre objekty
    if (! *arr)
    {
        printf("Chyba alokacie pamati\n");
        fclose(file);
        return -1;
    }
    for(int i = 0; i<file_obj_count; i++) // nacitavanie jednotlivych objektov zo suboru po riadku
    {
        fscanf(file, "%d %d %d", &obj_id, &obj_x, &obj_y);
        if(obj_x < 0 || obj_x > 1000 || obj_y < 0 || obj_y > 1000 || array_obj_id(*arr, file_obj_count, obj_id))
        {
            printf("Chyba zo suboru na riadku %d.\n", i);
            clear_clusters(*arr, file_obj_count); //uvolnenie pamate
            *arr = NULL;
            fclose(file);
            return -1;
        }

        // -- PRIRADENIE DO STRUCT obj_t --//
        obj.x = obj_x;
        obj.y = obj_y;
        obj.id = obj_id;

        // objekt do spravneho zhluku
        cluster = &(*arr)[i];
        append_cluster(cluster, obj);
        if (cluster->size != 1)
        {
            printf("Chyba alokacie pamati.\n");
            clear_clusters(*arr, file_obj_count); //uvolenie pamati
            *arr = NULL;
            fclose(file);
            return -1;
        }
        fcount++;
    }

    fclose(file);

    if (fcount - 1 < file_obj_count) //pocet objektov v subore nemoze byt mensi ako uvedeny pocet
    {
        printf( "Pocet objektu v souboru '%d' musi byt vetsi nebo roven hodnote 'count=%d'\n", fcount - 1, file_obj_count);
        clear_clusters(*arr, file_obj_count); // uvolnenie pamati
        *arr = NULL;
        return -1;
    }
    return file_obj_count;
}


/*
 Tisk pole shluku. Parametr 'carr' je ukazatel na prvni polozku (shluk).
 Tiskne se prvnich 'narr' shluku.
*/
void print_clusters(struct cluster_t *carr, int narr)
{
    printf("Clusters:\n");
    for (int i = 0; i < narr; i++)
    {
        printf("cluster %d: ", i);
        print_cluster(&carr[i]);
    }
}


/**
 * Upravi pole zhlukov na pozadovanu velkost --> size.

 */
int required_size_of_clusters(struct cluster_t *clusters, int size, const int default_size)
{
    assert(clusters);
    assert(size >= 0);
    assert(default_size > 0);


    if (default_size > size)  // kontrola poctu zhlukov na pozadovanu hodnotu
    {
        printf("Zadany pocet pozadovanych shluku '%d' je vetsi nez celkovy pocet objektu '%d'.\n", default_size, size);
        return -1;
    }

    int c1_idx, c2_idx, previous_c1_size; //pokial nie je pocet zhlukovrovny pozadovanemu poctu, susediace shluky sa spoja
    while (size > default_size)
    {
        find_neighbours(clusters, size, &c1_idx, &c2_idx); //hladanie susedov
        previous_c1_size = clusters[c1_idx].size; //spajanie susedov do zhluku na indexe 'c1_idx'
        merge_clusters(&clusters[c1_idx], &clusters[c2_idx]);
        if (clusters[c2_idx].size > 0 && clusters[c1_idx].size != previous_c1_size + clusters[c2_idx].size)
        {
            printf("Chyba alokace pameti.\n");
            return -1;
        }
        size = remove_cluster(clusters, size, c2_idx); //odstranenie shluku v poli z indexu 'c2_idx'
    }
    return size;
}


//-- SPRACOVANIE ARGUMENTOV -- //

int input_args(int argc, char *argv[])
{

    // -- ARGUMENT CONTROL POINT -- //
    int default_size = 1;
    if (argc > 1 && argc <= 3) // musi byt aspon 1 argument
    {
        if (argc == 3)
        {
            char *pEnd = NULL;
            default_size = strtol(argv[2], &pEnd, 10);
            if (*pEnd) //kontrola hodnoty argumentu
            {
                printf("Hodnota argumentu musi byt cislo, pricom je zadany argument: %s.\n", pEnd);
                return 0;
            }
            else if (default_size <= 0)
            {
                printf("Hodnota argumentu musi byt cislo vacia ako 0, pricom je zadane: %d.\n", default_size);
                return 0;
            }
        }
    }

    // -- CLUSTER WORK -- //

    struct cluster_t *clusters;
    int size, new_size;
    // -- NACITANIE SHLUKU -- //
    if ((size = load_clusters(argv[1], &clusters)) == -1)
    {
        return 0;
    }

    // priprava shluku na vypis (ziskanie pozadovaneho poctu zhlukov)
    if ((new_size = required_size_of_clusters(clusters, size, default_size)) == -1) //here chce default size z
    {
        clear_clusters(clusters, size); // uvolenenie pamate
        return 0;
    }
    size = new_size;

    // -- VYPIS ZHLUKU -- //
    print_clusters(clusters, size);
    // -- UVOLENENIE PAMATE -- //
    clear_clusters(clusters, size);

    return 1;
}


int main(int argc, char *argv[])
{
    // spracovanie argumentu
    int progress = input_args(argc, argv);
    if (!progress)
    {
        printf("NEUSPESNE PREVEDENIE PROGRAMU\n");
        return 1;
    }
    return 0;
}
