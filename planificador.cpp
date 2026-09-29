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
    int contador_dependencias; //para optimizar el algoritmo y salir del o² q m persigue...
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

        if(linea.empty() || linea.find_first_not_of(" \n\r\t") == string::npos){
            continue;
        }

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

        int cuantos_dependen = dependencias.size();
        //creacion objeto
        Actividad actividad = {acti_id, act_nombre, acti_tiempo, dependencias, {}, cuantos_dependen};
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
    map <int,int> registro_canales;

    while(!actividades_cola.empty() || registrador_de_pecausas.size() > 0){

        if(!actividades_cola.empty() && registrador_de_pecausas.size() < k){
            string id_actividad_actual = actividades_cola.front();
            actividades_cola.pop();
            

            int canal[2]; //se crea y abre canal
            pipe(canal);
            int id_del_proceso = fork();

            if(id_del_proceso == 0){
                close(canal[0]); //el hijo solo habla, no escucha cerramos esto por si las moscas
                
                usleep(tiempo_en_ms(actividades[id_actividad_actual].tiempo_ms));
                
                string mensaje = "Lista la actividad: " + actividades[id_actividad_actual].nombre ;

                write(canal[1], mensaje.c_str(), mensaje.size() +1);
                close(canal[1]);
                exit(0);

            }else{ //papdre

                close(canal[1]); //el padre ecucha pero no talkea
                
                registro_canales[id_del_proceso] = canal[0]; //se guarda el canal por donde se responde
                registrador_de_pecausas[id_del_proceso] = id_actividad_actual;
            }
        }else if(registrador_de_pecausas.size() == k || (!registrador_de_pecausas.empty() && actividades_cola.empty())){
            if(registrador_de_pecausas.size() == k){cout << "Capacidad maxima alcanzada, hay " + to_string(k) + " procesos activos, esperando" << endl;}
            
            int estado;
            int id_del_proceso_terminado = wait(&estado);
            
            
            string id_actividad_terminada = registrador_de_pecausas[id_del_proceso_terminado];
            
            char cubeta_para_el_msj[256];
            int mensaje_baiteado =  registro_canales[id_del_proceso_terminado];
            //Se trae el infice de la tabla del PCB, el cual apunta a donde esta el mensaje.

            read(mensaje_baiteado, cubeta_para_el_msj, sizeof(cubeta_para_el_msj));
            //sizeof para medir la memoria fisica de el coso

            cout << "Mensaje PIPE: " << cubeta_para_el_msj << endl;

            close(mensaje_baiteado); //liberamos el canal 

            for(auto &id_del_dependiente : actividades[id_actividad_terminada].dependientes){ //actualizar contador
                actividades[id_del_dependiente].contador_dependencias--;
                
                if(actividades[id_del_dependiente].contador_dependencias == 0){
                    actividades_cola.push(id_del_dependiente);
                }
            }
           
            registrador_de_pecausas.erase(id_del_proceso_terminado);
            registro_canales.erase(id_del_proceso_terminado);
        }
    }
    archivo.close();

    return 0;
}