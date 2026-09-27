#include <stdio.h>

int main(int argc, char *argv[]) {
    // lo del argc contador, lo otro era algo de como el codigo que deben ingresar
    //nota: el argv el 0 era como el nombre del archivo ese no importar
    //los otros dos weyes son el 1 y 2 esos si importan el 1 es el nombre de archivo a abrir
    //y el 2 es el numero de procesps q quieren q haga
    //bro se dejaba notas a el mismo .... :(

    printf("Hola github \n");
    if(argc < 3){ //esto para eso d q verificar que metieron todos los argu bn
        printf("Error: numero de argumentos insuficiente \n");
        return 1;
    }

    FILE *planificacion = fopen(argv[1], "r");
    if(planificacion == NULL){//por si no encuentro el coso
        printf("Error: no se pudo abrir el archivo %s \n", argv[1]);
        return 1;
    }

    //ingesta del FILE
    char linea[256]; //algo vi de que el estandar era 256, chekear esa data
    // aunque las lineas sean cortas dijeron que podian hacer hasta como 20k de k
    // mjor q sobre q falte ww
    while(fgets(linea, sizeof(linea), planificacion) != NULL){
        printf("%s", linea);
        }
    fclose(planificacion);
    return 0;
}