#include <fstream>
#include <stdio.h>
#include <iostream>
#include <string>
#include <queue>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <algorithm>
#include <map>
#include <unistd.h>
#include <sys/wait.h>
using namespace std;

struct Actividad{//ahora si, k no entendi como era el heap en c.......
    string id;
    string nombre;
    int tiempo_ms;
    vector<string> dependencias;
    vector<string> dependientes;
};

string limpiar_espacios(const string &palabra){
    size_t inicio = palabra.find_first_not_of(" ");
    size_t fin = palabra.find_last_not_of(" "); //filtros de posicion 

    if(inicio == string::npos){ //por si ta vacio
        return "";
    }
    return palabra.substr(inicio, fin - inicio + 1); //funcion de str q recorta
}

int generar_tiempo_aleatorio(int min, int max) {
    return rand() % (max - min + 1) + min;
}

int tiempo_en_ms(int tiempo) {
    return tiempo * 1000;
}

int main(int argc, char *argv[]){
    if (argc < 3){
        cout << "Faltan argumentos" << endl;
        return 1;
    }
 
    const size_t k = stoi(argv[2]);
    srand(time(NULL)); //para lo del tiempo random

    ifstream archivo(argv[1]);
    if (!archivo.is_open()) {
        cout << "No se pudo abrir el archivo" << endl;
    return 1;
    }
    
    string linea; 
    map<string, Actividad> actividades;
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

        string acti_id = act_id;
        int acti_tiempo = 0;
        if(!act_tiempo.empty()){
            acti_tiempo = stoi(act_tiempo);
        }else{//si esta vacio ranog entre 100-5000
            acti_tiempo = generar_tiempo_aleatorio(100, 5000);
        }
        //separacion de dependencias en vector
        vector<string> dependencias;
        if (!act_deps.empty()) { //si las dependencias existen, se separan
            stringstream separador_deps(act_deps);
            string dependencias_solitas;

            while (getline(separador_deps, dependencias_solitas, ',')) {
                dependencias_solitas = limpiar_espacios(dependencias_solitas);
                if (!dependencias_solitas.empty()) {
                    dependencias.push_back(dependencias_solitas);
                }
            }
        }

        //creacion objeto
        Actividad actividad = {acti_id, act_nombre, acti_tiempo, dependencias, {}};
        actividades[acti_id] = actividad;
    }

    //rellenado de dependientes
    for(auto &carlitos : actividades){
        Actividad &Objeto_Actividad = carlitos.second;
        
        if(!Objeto_Actividad.dependencias.empty()){ //Asegurarse de no recorrer algo vacio
            for(string id_del_vector_dep : Objeto_Actividad.dependencias){
                actividades[id_del_vector_dep].dependientes.push_back(Objeto_Actividad.id);
            }
        }
    }

    //algoritmo para ejecutar las actividades
    queue<string> actividades_cola;
    for(auto &carlitos_sin_cola : actividades){ //relenar los tier 0
        Actividad &Objeto_Actividad = carlitos_sin_cola.second;
        if(Objeto_Actividad.dependencias.empty()){
            actividades_cola.push(Objeto_Actividad.id);
        }
    }

    map<int,string> registrador_de_pecausas;

    while(!actividades_cola.empty() || registrador_de_pecausas.size() > 0){

        if(!actividades_cola.empty() && registrador_de_pecausas.size() < k){
            string id_actividad_actual = actividades_cola.front();
            actividades_cola.pop();

            int id_del_proceso = fork();

            if(id_del_proceso == 0){
                cout << "Ejecutando actividad: " << actividades[id_actividad_actual].nombre << " hay " << registrador_de_pecausas.size() + 1<< " activos" << endl;
                usleep(tiempo_en_ms(actividades[id_actividad_actual].tiempo_ms));
                exit(0);
            }else{
               registrador_de_pecausas[id_del_proceso] = id_actividad_actual;
            }
        }else if(registrador_de_pecausas.size() == k || (!registrador_de_pecausas.empty() && actividades_cola.empty())){
            int id_del_proceso_terminado = wait(NULL);
            string id_actividad_terminada = registrador_de_pecausas[id_del_proceso_terminado];

            for(auto &carlitos_identificador_de_dependencias : actividades[id_actividad_terminada].dependientes){
                vector<string> &quienes_dependen_de_carlitos = actividades[carlitos_identificador_de_dependencias].dependencias;
                vector<string>::iterator posicion_del_listo = find(quienes_dependen_de_carlitos.begin(), quienes_dependen_de_carlitos.end(), id_actividad_terminada);
                //complicao pero practicamente guarda lo que dice el nombre

                if(posicion_del_listo != quienes_dependen_de_carlitos.end()){
                    quienes_dependen_de_carlitos.erase(posicion_del_listo);
                }//sin esto si no encontro el coso va a devolver el ultimo, es como para asegurar q todo no explote aunque no se deberia ejecutar

                if(quienes_dependen_de_carlitos.empty()){
                    actividades_cola.push(carlitos_identificador_de_dependencias);
                }
            }
            cout << "Actividad terminada " << actividades[id_actividad_terminada].nombre << " , ahora quedan " << registrador_de_pecausas.size() - 1 << " procesos activos" << endl;
            registrador_de_pecausas.erase(id_del_proceso_terminado);
        }
    }
    archivo.close();

    return 0;
}