#include <fstream>
#include <stdio.h>
#include <iostream>
#include <string>
#include <queue>
#include <sstream>
#include <vector>
#include <algorithm>
#include <map>
using namespace std;

struct Actividad{//ahora si, k no entendi como era el heap en c.......
    int id;
    string nombre;
    int tiempo_ms;
    vector<int> dependencias;
    vector<int> dependientes;
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
    map<int, Actividad> actividades;
    string act_id, act_nombre, act_tiempo, act_deps;

    while (getline(archivo, linea)) { //lectura e ingestion del texto
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
        Actividad actividad = {acti_id, act_nombre, acti_tiempo, dependencias, {}};
        actividades[acti_id] = actividad;
    }

    //rellenado de dependientes
    for(auto &carlitos : actividades){
        Actividad &Objeto_Actividad = carlitos.second;
        
        if(!Objeto_Actividad.dependencias.empty()){ //Asegurarse de no recorrer algo vacio
            for(int id_del_vector_dep : Objeto_Actividad.dependencias){
                actividades[id_del_vector_dep].dependientes.push_back(Objeto_Actividad.id);
            }
        }
    }

    //algoritmo para ejecutar las actividades
    queue<int> actividades_cola;
    for(auto &carlitos_sin_cola : actividades){ //relenar los tier 0
        Actividad &Objeto_Actividad = carlitos_sin_cola.second;
        if(Objeto_Actividad.dependencias.empty()){
            actividades_cola.push(Objeto_Actividad.id);
        }
    }

    while(!actividades_cola.empty()){
        int id_actividad_actual = actividades_cola.front();
        actividades_cola.pop();

        cout << "Tamo haciendo eto" << id_actividad_actual << endl;

        for(auto &carlitos_identificador_de_dependencias : actividades[id_actividad_actual].dependientes){
            vector<int> &quienes_dependen_de_carlitos = actividades[carlitos_identificador_de_dependencias].dependencias;
            vector<int>::iterator posicion_del_listo = find(quienes_dependen_de_carlitos.begin(), quienes_dependen_de_carlitos.end(), id_actividad_actual);
            //complicao pero practicamente guarda lo que dice el nombre
            quienes_dependen_de_carlitos.erase(posicion_del_listo);

            if(quienes_dependen_de_carlitos.empty()){
                actividades_cola.push(carlitos_identificador_de_dependencias);
            }
        }
    }

    archivo.close();

    return 0;
}