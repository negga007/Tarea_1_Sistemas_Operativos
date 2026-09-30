# Tarea 1 - Sistemas Operativos: Planificador Dieciochero

Ignacio Espinoza - Ingeniería Civil en Informática y Telecomunicaciones, UDP

## Qué hace

Lee un plan.txt con actividades (ID, nombre, tiempo en ms y dependencias) que forman un DAG y las ejecuta respetando las dependencias. Cada actividad corre en un proceso hijo con fork(), con máximo K procesos a la vez. Los procesos se comunican con pipes. No se usan hilos.

## Archivos

- planificador.cpp: todo el código
- plan.txt
- README.md: este archivo

## Compilar y ejecutar

Compilar: g++ -Wall -Wextra -std=c++17 planificador.cpp -o planificador -lpthread

Ejecutar: ./planificador plan.txt K

K tiene que ser un entero mayor que 0. Si faltan argumentos o K no sirve, avisa y termina. Ctrl+C aborta todo el plan.

## Formato de plan.txt

Una actividad por línea: ID : nombre : tiempo_ms : dep1, dep2, ...

- Si el tiempo viene vacío o no es un número, se asigna uno aleatorio entre 100 y 5000 ms.

## Funciones implementadas

- Actividad: struct del nodo (id, nombre, tiempo, dependencias, dependientes y contador_dependencias).
- limpiar_espacios: recorta espacios, tabs y saltos de línea.
- generar_tiempo_aleatorio: número al azar entre min y max.
- tiempo_en_ms: pasa los ms del plan a microsegundos para usleep.
- guardia_de_la_fonda: manejador de SIGINT, solo pone en 1 la bandera llego_seremi.
- main: lee el plan, arma el grafo y corre el ciclo del planificador.

## Cómo funciona

- Parseo: El programa lee el archivo línea por línea con getline y va separando los pedazos usando los dos puntos y las comas. Guarda todo en un map usando el ID de la actividad para encontrar los datos rápido.

- El Grafo (DAG): Básicamente, a cada actividad se le anota qué otras tareas dependen de ella. Además, el nodo usa un contador que dice cuántas dependencias le faltan para poder empezar.

- Planificación: Utiliza una cola simple. Entran ahí las actividades que tienen su contador en 0 (que ya pueden empezar). Si hay algo en la cola y el sistema no se ha pasado del límite de procesos K, lanza un hijo con fork(). Si ya llegó al tope K o no hay nada listo, el padre se queda esperando tranquilo con un wait() a que algún hijo termine. Cuando un hijo termina bien, el programa le resta 1 al contador de sus dependientes; si alguno llega a 0, pasa a la cola.

- Pipes: Crea dos pipes para cada hijo. Una para que el padre le pase el texto con el mensaje al hijo que dependia del otro hijo y otra para que el otro hijo avise que terminó. El padre hace de intermediario entre todos para que no sea un enredo.

- Errores: Si un hijo termina mal se revisa con WEXITSTATU, simplemente no le baja el contador a las actividades que dependían de él. Así, esa rama nunca se lanza.

- Ctrl+C: cuando wait() es interrumpido por la señal, el padre manda SIGTERM a los hijos, los recoge con wait() y cierra los pipes.

## Decisiones de diseño

- wait() bloqueante: Se optó por dejar al proceso padre en estado de espera con wait() hasta que un hijo termine, en lugar de implementar una espera activa que consuma CPU innecesariamente.

- Contador de dependencias: En lugar de recorrer el grafo completo en cada iteración para buscar tareas listas, el uso de un contador permite identificar de forma directa qué actividades pueden iniciar. 

- Dos pipes por actividad: Centralizar la comunicación a través del padre (actuando como intermediario para recibir y enviar mensajes) simplificó la estructura del programa, facilitando un control ordenado y el cierre adecuado de los descriptores de archivo.

- Mensaje acotado en los pipes: Acumular miles de dependencias en un solo texto puede llenar el buffer del pipe. Para evitarlo, el programa deja de agregar a la cadena a 120 caracteres y añade una etiqueta de resumen, garantizando una escritura segura y fluida.

- Manejador de señal mínimo: solo cambia una bandera, la limpieza se hace en el flujo normal.

- Manejo de fallos ignorar la rama: Al detectar la falla de un hijo, el programa simplemente omite la reducción del contador en las actividades que dependían de él. Esta solución aísla la rama afectada de forma natural sin requerir algoritmos adicionales para borrar nodos del grafo, las actividades nunca llegan al contador 0 y no entran en la cola.
