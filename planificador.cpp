#include <fstream>
#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
using namespace std;

struct Actividad{//ahora si k no entendi como era el heap en c.......
    int id;
    string nombre;
    int tiempo_ms;
    vector<int> dependencias;
};

string limpiar_espacios(const string &palabra){
    size_t inicio = palabra.find_first_not_of(" ");
    size_t fin = palabra.find_last_not_of(" "); //filtros de posicion 

    if(inicio == string::npos){ //por si ta vacio
        return "";
    }
    return palabra.substr(inicio, fin - inicio + 1); //funcion de str q recorta
}


int main(int argc, char *argv[]){
    if (argc < 3){
        cout << "Faltan argumentos" << endl;
        return 1;
    }
    ifstream archivo(argv[1]);
    if (!archivo.is_open()) {
        cout << "No se pudo abrir el archivo" << endl;
    return 1;
    }
    
    string linea; 
    vector<Actividad> actividades;
    string act_id, act_nombre, act_tiempo, act_deps;

    while (getline(archivo, linea)) {
        stringstream separador_papu_pro(linea);
        getline(separador_papu_pro, act_id, ':');
        getline(separador_papu_pro, act_nombre, ':');
        getline(separador_papu_pro, act_tiempo, ':');
        getline(separador_papu_pro, act_deps, ':');
        
        act_id = limpiar_espacios(act_id);
        act_nombre = limpiar_espacios(act_nombre);
        act_tiempo = limpiar_espacios(act_tiempo);
        act_deps = limpiar_espacios(act_deps);

        int acti_id = stoi(act_id); //cambio de string a int
        int acti_tiempo = stoi(act_tiempo);

        //separacion de dependencias en vector
        vector<int> dependencias;
        if (!act_deps.empty()) { //si las dependencias existen, se separan
            stringstream separador_deps(act_deps);
            string dependencias_solitas;

            while (getline(separador_deps, dependencias_solitas, ',')) {
                dependencias_solitas = limpiar_espacios(dependencias_solitas);
                if (!dependencias_solitas.empty()) {
                    dependencias.push_back(stoi(dependencias_solitas));
                }
            }
        }


        cout << acti_id << "|" << act_nombre << "|" << acti_tiempo << "|" << "{";
        int size = dependencias.size(); 

        for(int xd = 0; xd < size ; xd++){
            if(xd == size - 1){
                cout << dependencias[xd];
                break;
            }
            cout << dependencias[xd] << ",";
        }
        
        cout << "}" << endl;

        //creacion objeto
        Actividad actividad = {acti_id, act_nombre, acti_tiempo, dependencias};
        actividades.push_back(actividad);
    }
    archivo.close();

    return 0;
}