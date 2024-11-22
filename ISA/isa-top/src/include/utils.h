#include <iostream>
#include <vector>


using namespace std;

struct Config{
    string interface;
    char sort_option = 'b'; //default to bytes
    int interval = 1;
};

/**
    @brief Vypíše nápovedu na použitie programu
 */
void print_usage();
/**
    @brief Zoznam dostupných sieťových rozhraní
    @return vektor názvov sieťových rozhraní
 */
vector<string> list_interfaces();
/**
    @brief Spracovanie argumentov programu
    @param argc počet argumentov
    @param argv pole argumentov
    @return konfiguračný objekt
 */
Config parse_arguments(int argc, char* argv[]);

