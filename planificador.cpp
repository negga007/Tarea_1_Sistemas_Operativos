#include <fstream>
#include <stdio.h>
#include <iostream>
#include <string>
using namespace std;

int main(char argc, char *argv[]){
    if (argc < 3){
        printf("Faltan argumentos");
        return 1;
    }
    ifstream archivo(argv[1]);
    if (!archivo.is_open()) {
    std::cout << "Error: no se pudo abrir el archivo" << std::endl;
    return 1;
    }
    
    string linea;
    while (getline(archivo, linea)) {
        printf("%s\n", linea.c_str());
    }
    archivo.close();

    return 0;
}