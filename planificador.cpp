#include <fstream>
#include <stdio.h>
#include <iostream>
#include <string>
#include <queue>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <csignal>
#include <cerrno>
#include <vector>
#include <algorithm>
#include <map>
#include <unistd.h>
#include <sys/wait.h>
using namespace std;
volatile sig_atomic_t llego_seremi = 0;

void guardia_de_la_fonda(int alo){
    (void)alo;
    llego_seremi=1;
}

struct Actividad{
    string id;
    string nombre;
    int tiempo_ms;
    vector<string> dependencias;
    vector<string> dependientes;
    int contador_dependencias; 
};

string limpiar_espacios(const string &palabra){
    size_t inicio = palabra.find_first_not_of(" \t\r\n");
    size_t fin = palabra.find_last_not_of(" \t\r\n"); //filtros de posicion 

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
    //Seguridad extra, k sea un numero
    int prueba = 0;
    try{
        prueba = stoi(argv[2]);
    }catch(...){
        cout << "K no es numero valido, error" << endl;
        return 1;
    }

    if(prueba <= 0){cout<< "error, k debe ser numero natural"; return 1;}
    const size_t k = stoi(argv[2]);
    srand(time(NULL)); //para lo del tiempo random
    
    signal(SIGPIPE, SIG_IGN); //por si un hijo muere
    
    //para lo de la seremi
    struct sigaction escudo_seremi;
    escudo_seremi.sa_handler = guardia_de_la_fonda;
    sigemptyset(&escudo_seremi.sa_mask);
    escudo_seremi.sa_flags = 0;
    sigaction(SIGINT, &escudo_seremi, NULL);

    
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
            try{
                 acti_tiempo = stoi(act_tiempo);
            }catch(...){
                cout << "Advertencia: tiempo invalido en el parametro de tiempo dela actividad " << acti_id << "se usara uno random" << endl;
                acti_tiempo = generar_tiempo_aleatorio(100, 5000);
            }
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
                if( actividades.count(id_del_vector_dep)){
                    actividades[id_del_vector_dep].dependientes.push_back(Objeto_Actividad.id);
                }else{
                    cout << "se registro un error en las dependencias, no existe la " << id_del_vector_dep << endl;
                }
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
    map <string, string> cartero;

    while((!actividades_cola.empty() || registrador_de_pecausas.size() > 0) && !llego_seremi){

        if(!actividades_cola.empty() && registrador_de_pecausas.size() < k){


            string id_actividad_actual = actividades_cola.front();
            actividades_cola.pop();
            

            int canal[2]; //se crea y abre canal
            pipe(canal);

            int canal_secu[2]; //paso del canal de padre a hijo
            pipe(canal_secu);

            int id_del_proceso = fork();

            if(id_del_proceso == 0){
                close(canal[0]); //el hijo solo habla, no escucha cerramos esto por si las moscas
                close(canal_secu[1]);
                
                string requeridos;
                char bucket[256];
                ssize_t leer;
                while((leer = read(canal_secu[0], bucket, sizeof(bucket))) > 0){
                    requeridos.append(bucket, leer);
                } //se añaden los x caracteres primeros del bucket

                close(canal_secu[0]);

                cout << "Actividad " << actividades[id_actividad_actual].nombre << " lista para ejecutar";

                if(!requeridos.empty()){
                    cout << "requerimientos captados: [";
                    cout << requeridos << "]";
                }
                
                cout << endl;
                usleep(tiempo_en_ms(actividades[id_actividad_actual].tiempo_ms));
                
                /*if(id_actividad_actual == "5"){ prueba de errores
                    exit(1);
                }*/

                string mensaje = "Actividad: " + actividades[id_actividad_actual].nombre + " lista";

                write(canal[1], mensaje.c_str(), mensaje.size() +1);
                close(canal[1]);
                exit(0);

            }else{ //papdre

                close(canal[1]); //el padre ecucha pero no talkea
                close(canal_secu[0]);
                if(!cartero[id_actividad_actual].empty()){
                    write(canal_secu[1], cartero[id_actividad_actual].c_str(), cartero[id_actividad_actual].size());
                }
                close(canal_secu[1]);
                registro_canales[id_del_proceso] = canal[0]; //se guarda el canal por donde se responde
                registrador_de_pecausas[id_del_proceso] = id_actividad_actual;
            }
        }else if(registrador_de_pecausas.size() == k || (!registrador_de_pecausas.empty() && actividades_cola.empty())){
            if(registrador_de_pecausas.size() == k){cout << "Capacidad maxima alcanzada, hay " + to_string(k) + " procesos activos, esperando" << endl;}
            
            int estado;
            int id_del_proceso_terminado = wait(&estado);
            if(id_del_proceso_terminado == -1){//si recive un control+c
                if(errno == EINTR && llego_seremi){
                    //a pesar de ser un true/false && numero, sig_atomict_t es en 0 false y en otro cualquiera true
                    cout << "aborten llego seremi causas!!1" << endl;

                    for(auto &actividades_echas : registrador_de_pecausas){
                        kill(actividades_echas.first, SIGTERM); 
                    }

                    while (wait(NULL) > 0); //recoge el cadaver de todos sus hijos
                    //termina una vez recogio a todos los hijos asesinados a sangre fria por la seremi

                    for(auto &orejas_abiertas : registro_canales){
                        close(orejas_abiertas.second); // cierra los canales de escucha del hijo
                        //al morir el hijo queda el unico canala abierto su escucha por lo tanto cierramos este
                        //ya que guarde la oreja de cada pipe del parte del padre en el mapa registro canales
                    }
                    
                    break;
                }
                continue; //por si por alguna razon llego un -1 y no fue del guarda,,,
            }
            
            string id_actividad_terminada = registrador_de_pecausas[id_del_proceso_terminado];
            
            char cubeta_para_el_msj[256];
            int mensaje_baiteado =  registro_canales[id_del_proceso_terminado];
            // Se recupera el File Descriptor asociado al proceso hijo
            ssize_t lector1 = read(mensaje_baiteado, cubeta_para_el_msj, sizeof(cubeta_para_el_msj) - 1);
            if (lector1 < 0) lector1 = 0; //si hay error en el pipe pues decimos 0
            cubeta_para_el_msj[lector1] = '\0';//sizeof para medir la memoria fisica de el coso

            close(mensaje_baiteado); //liberamos el canal 
            
            //wifexited es para true falso si el hijo termino solito ono
            //wexistatus es como el exit del hijo se supone, si es 0 deberia ser 0 entonces eso es que termino bien
            
            if(WIFEXITED(estado) && WEXITSTATUS(estado) == 0){    
                
                for(auto &id_del_dependiente : actividades[id_actividad_terminada].dependientes){ //actualizar contador
                    actividades[id_del_dependiente].contador_dependencias--;
                    
                    if(cartero[id_del_dependiente].length() < 120){ //para gran escala si el mensaje es mayor a 120, si lo es añade de forma resumida
                        cartero[id_del_dependiente] += string(cubeta_para_el_msj) + " | " ;
                    } else if (cartero[id_del_dependiente].find(" otros") == string::npos) {
                        cartero[id_del_dependiente] += " otros(limitado para ahorro de recursos)" ;
                    }
                    //añade para el proceso dependiente un texto que diga una por una que todos su requerimientos
                    //se completaron
                    if(actividades[id_del_dependiente].contador_dependencias == 0){
                        actividades_cola.push(id_del_dependiente);
                    }
                }
            }else{
                cout << "hubo un error en la rama de " << actividades[id_actividad_terminada].nombre << endl;
            }

            registrador_de_pecausas.erase(id_del_proceso_terminado);
            registro_canales.erase(id_del_proceso_terminado);
            
        }
    }
    archivo.close();

    return 0;
}