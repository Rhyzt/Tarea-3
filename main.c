#include "tdas/extra.h"
#include "tdas/list.h"
#include "tdas/map.h"
#include "tdas/stack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#define CAPACIDAD_INICIAL 50

typedef struct {
    char *nombre;
    int peso;
    int valor;
} item;

typedef struct escenario escenario;

typedef struct escenario {
    char *nombre; // Nombre del escenario
    int id; // ID del escenario
    char *desc; // Descripcion del escenario
    List *loot; // Array con los items presentes en el escenario
    int pos[4]; // array para almacenar los ids adyacentes y poder convertirlo a un grafo
    escenario *adyacentes[4]; // array con conexiones a escenarios adyacentes (arriba, abajo, izquierda, derecha en ese orden)
    int esFinal; // Marca si es un escenario final
} escenario;

typedef struct {
    int peso; // Peso actual del jugador
    float tiempo; // Tiempo restante
    int puntaje;
    List *inventario; // Array con los items actuales del jugador
} jugador;

// Crear el escenario (data)
escenario *crearEscenario(char **linea) { 
    escenario *s = malloc(sizeof(escenario));
    s -> id = atoi(linea[0]) - 1; // Se usa el - 1 para poder empezar a contar desde el 0 en vez del 1 (Suponiendo que las IDs que nos pasan comienzan del 1)
    s -> nombre = strdup(linea[1]);
    s -> desc = strdup(linea[2]);
    s -> loot = list_create();
    List *items = split_string(linea[3], ";"); // Separa los items presentes en el escenario (por ;)
    for (char *i = list_first(items) ; i != NULL ; i = list_next(items)) { // Itera por cada item de la lista
        List *valores = split_string(i, ",");
        item *obj = malloc(sizeof(item)); // Crea cada objeto presente
        obj -> nombre = strdup(list_first(valores)); // Copia los valores al struct
        obj -> peso = atoi(list_next(valores));
        obj -> valor = atoi(list_next(valores));
        list_pushBack(s -> loot, obj); // Añade el item a la lista de items del escenario actual
        listDataClean(valores);
        free(valores);
    }

    for (int i = 0 ; i < 4 ; i++) { // Marcar las direcciones posibles (arriba, abajo, izquierda, derecha)
        if (strcmp(linea[i + 4], "-1") != 0)
            s -> pos[i] = atoi(linea[i + 4]) - 1;
        else
            s -> pos[i] = atoi(linea[i + 4]);
    }

    for (int i = 4 ; i < 8 ; i++)
    s -> esFinal = strcmp(linea[8], "Si") == 0;

    listDataClean(items);
    free(items);
    return s;
}

// Lee todas las lineas del archivo dado y las transforma a escenarios
escenario **cargarArchivo(char *direccion) { // Lee un archivo CSV y crea el grafo, retorna el Nodo inicial
    // Intenta abrir el archivo CSV que contiene los datos de los escenarios
    FILE *archivo = fopen(direccion, "r");
    if (archivo == NULL) {
        perror("Error al abrir el archivo"); // Informa si el archivo no puede abrirse
        exit(EXIT_FAILURE);
    }

    char **campos;
    // Leer y parsear una línea del archivo CSV. La función devuelve un array de
    // strings, donde cada elemento representa un campo de la línea CSV procesada.
    campos = leer_linea_csv(archivo, ','); // Lee los encabezados del CSV

    escenario **arrayEscenarios = malloc(sizeof(escenario *) * CAPACIDAD_INICIAL); // Crea el array de escenarios
    int pos = 0, capacidad = CAPACIDAD_INICIAL; 
    // Lee cada línea del archivo CSV hasta el final
    while ((campos = leer_linea_csv(archivo, ',')) != NULL) {
        escenario *s = crearEscenario(campos); // Lee la linea actual como un escenario
        if (pos >= (capacidad - 1)) { // Si se alcanza el limite de escenarios
            capacidad *= 2;
            arrayEscenarios = realloc(arrayEscenarios, sizeof(escenario *) * (capacidad)); // Se redimensiona
            if (arrayEscenarios == NULL) {
                perror("Error al reasignar memoria");
                exit(EXIT_FAILURE);
            }
        }
        arrayEscenarios[pos++] = s; // Se añade al array de escenarios
    }
    arrayEscenarios[pos] = NULL; // Marcar final del array

    fclose(archivo); // Cierra el archivo después de leer todas las líneas
    return arrayEscenarios;
}

// Conecta los nodos con sus respectivas direcciones
void conectarNodos(escenario *s, escenario **array) {
    s -> adyacentes[0] = (s -> pos[0] == -1) ? NULL : array[s -> pos[0]];
    s -> adyacentes[1] = (s -> pos[1] == -1) ? NULL : array[s -> pos[1]];
    s -> adyacentes[2] = (s -> pos[2] == -1) ? NULL : array[s -> pos[2]];
    s -> adyacentes[3] = (s -> pos[3] == -1) ? NULL : array[s -> pos[3]];
}

// Crea el grafo respecto a un array de escenarios
escenario *crearGrafo(escenario **array) { // Conecta los nodos, creando un grafo y retorna el escenario inicial (ID 0)
    for (int i = 0 ; array[i] != NULL ; i++) // Itera por el array de escenarios
        conectarNodos(array[i], array); // Conecta todos los nodos adyacentes al escenario
    return array[0]; // Se retorna el nodo entrada
}

// Muestra las intrucciones para iniciar una partida
void mostrarMenuPrincipal() {
    puts("========================================");
    puts("              GraphQuest");
    puts("========================================");
    puts("Instrucciones: ");
    puts("1) Cargar Laberinto desde Archivo CSV");
    puts("2) Iniciar Partida");
}

// Muestra las diferentes opciones del juego
void mostrarMenuJuego() {
    puts("1) Recoger Item(s)");
    puts("2) Descartar Item(s)");
    puts("3) Avanzar en una Direccion");
    puts("4) Reiniciar Partida");
    puts("5) Salir del Juego");
}

// Para esperar a que el usuario realice una accion antes de seguir con el programa
void esperarAccion() {
    puts("Presione enter para continuar");
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    puts(""); 
}

// Para verificar si una opcion se encuentra dentro de unos limites establecidos
void verificarOpcion(int *num, int limite) {
    char str[3];
    while (1) {
        fgets(str, 3, stdin);
        if (str[strlen(str) - 1] != '\n') { //Se revisa si el usuario escribio mas de 2 caracteres
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF); // Limpiar stdin para leer correctamente el proximo input
        } 
        else {
            if (isdigit(str[0]) && str[1] == '\n') { //En caso de que el numero ingresado no sea valido
                *num = str[0] - '0';
                if (*num > 0 && *num <= limite) break;
            }
        }
        puts("Ingresa una opcion Valida");
    }
}

// Funcion para cargar los datos del laberinto, devolviendo el escenario de partida
escenario *menuPrincipal() {
    char direccion[200];
    printf("Ingrese la direccion del archivo CSV: ");
    fgets(direccion, sizeof(direccion), stdin);
    direccion[strcspn(direccion, "\n")] = '\0';
    escenario **arrayTemp = cargarArchivo(direccion);
    escenario *grafo = crearGrafo(arrayTemp);
    free(arrayTemp);
    puts("Iniciando nueva partida de GraphQuest...");
    return grafo;
}

//Inicializa los datos de un nuevo jugador
jugador *cargarJugador() {
    jugador *j = malloc(sizeof(jugador));
    j -> inventario = list_create();
    j -> peso = 0; 
    j -> puntaje = 0;
    j -> tiempo = 10; // Se inicializa el tiempo en 10
    return j;
}

// Imprime todos los items de la lista proporcionada
int imprimirItems(List *lista) {
    int pesoTotal = 0, valorTotal = 0;
    if (list_first(lista) == NULL) {
        return 0;
    }
    for (item *i = list_first(lista) ; i != NULL ; i = list_next(lista)) {
        puts(i -> nombre);
        printf("Peso: %d \n", i -> peso);
        printf("Valor: %d \n", i -> valor);
        puts("--------------------");
        pesoTotal += i -> peso;
        valorTotal += i -> valor;
    }
    printf("Peso Total: %d \n", pesoTotal);
    printf("Valor Total: %d \n", valorTotal);
    return 1;
}

// Printea las opciones disponibles
void printearDirecciones(escenario *s) { 
    char *direcciones[4] = {"ARRIBA", "ABAJO", "IZQUIERDA", "DERECHA"};
    int flag = 1;
    for (int i = 0 ; i < 4 ; i++) {
        if (s -> adyacentes[i] != NULL) {
            if (flag) {
                printf("%s", direcciones[i]);
                flag = 0;
            }
            else {
                printf(", %s", direcciones[i]);
            }
        }
    }
    printf("\n");
} 

// Descripcion del escenario
void descEscenario(escenario *s, jugador *j) { 
    puts("========================================");
    puts(s -> nombre);
    puts("Descripcion: ");
    puts(s -> desc);
    puts("");
    puts("Items disponibles:");
    if (imprimirItems(s -> loot) == 0) {
        puts("Este escenario no posee items");
    }
    puts("");
    puts("Inventario actual: ");
    if (imprimirItems(j -> inventario) == 0) {
        puts("Inventario vacio\n");
    }

    printf("Direcciones posibles: ");
    printearDirecciones(s);

    printf("Tiempo restante: %.0f \n", j -> tiempo);

    puts("========================================");
}

// Compara strings, sin tomar en cuenta los espacios ni mayusculas y minusculas (0 iguales, 1 diferentes)
int compararStrings(char *s1, char *s2) { 
    while (*s1 != '\0' || *s2 != '\0') { // Recorrer ambas cadenas
        while (*s1 == ' ') s1++; // Para saltar los espacios
        while (*s2 == ' ') s2++;

        if (*s1 == '\0' && *s2 == '\0') return 0; // Si ambas llegaron al final

        if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2)) // Se comparan ambos caracteres en minuscula
            return 1;

        s1++; // Avanzar los punteros
        s2++;
    }
    return 0;
}

// Revisa si el jugador posee cierto item
int sinItem(jugador *j, item *i) {
    for (item *inv = list_first(j -> inventario) ; inv != NULL ; inv = list_next(j -> inventario)) {
        if (strcmp(inv -> nombre, i -> nombre) == 0) return 0;
    }
    return 1;
}

// Obtiene todos los items validos de un escenario a partir de un input
List *recogerItem(escenario *s, List *input, jugador *j) { 
    List *listaItems = list_create(); // Lista para almacenar los datos de los items validos
    for (char *str = list_first(input) ; str != NULL ; str = list_next(input)) // Recorre la lista de inputs
        for (item *i = list_first(s -> loot) ; i != NULL ; i = list_next(s -> loot)) // Recorre la lista de items para ver cuales son validos
            if (compararStrings(str, i -> nombre) == 0 && sinItem(j, i)) { // Si el nombre del item es igual al proporcionado y el usuario no posee el item
                list_pushBack(listaItems, i);
                break; // Si encontro el item con el que coincidia, no es necesario seguir iterando
            }
    return listaItems;
}

// Se anaden todos los items de la lista al inventario
void anadirItems(jugador *j, List *items) { 
    for (item *i = list_first(items) ; i != NULL ; i = list_next(items)) { 
        list_pushBack(j -> inventario, i);
        j -> peso += i -> peso; // Se le agrega al peso total del jugador
        j -> puntaje += i -> valor;
    }
}

// Agrega items, peso y valor, descuenta 1 de tiempo
void opcion1(escenario *s, jugador *j, List *input) { 
    List *listaItems = recogerItem(s, input, j);
    if (list_first(listaItems) == NULL) return; // Si no se recogio ningun objeto
    anadirItems(j, listaItems);
    j -> tiempo -= 1;
}

// Funcion para remover items del inventario de un jugador usando una lista de strings
int quitarItems(jugador *j, List *input) {
    int flag = 0;
    for (char *str = list_first(input) ; str != NULL ; str = list_next(input)) {// Recorre la lista de inputs
        item *i = list_first(j -> inventario);
        while (i != NULL) { // Recorre el inventario para ver cuales son validos
            item *siguiente = list_next(j -> inventario); 
            if ((compararStrings(str, i -> nombre) == 0) && !sinItem(j, i)) { // Si el nombre del item es igual al proporcionado y el usuario posee el item
                flag = 1; // Se elimino al menos un objeto
                j -> peso -= i -> peso;
                j -> puntaje -= i -> valor;
                list_popCurrent(j -> inventario);
                break; // Si encontro el item con el que coincidia, no es necesario seguir iterando
            }
            i = siguiente;
        }
    }
    return flag;
}

// Remueve uno o varios items
void opcion2(jugador *j, List *input) {
    if(quitarItems(j, input)) // Si se quito al menos un item
        j -> tiempo -= 1;
}

// Muestra si la direccion escogida es valida para el escenario actual
int dirValida(escenario *s, char *input) {
    char *direcciones[4] = {"ARRIBA", "ABAJO", "IZQUIERDA", "DERECHA"};
    
    for (int i = 0 ; i < 4 ; i++) {
        if (compararStrings(direcciones[i], input) == 0) {
            if (s->pos[i] != -1) {
                return i; // Direccion valida y accedible
            }
            return -1; // Direccion no accesible desde este escenario
        }
    }
    
    return -1; // No se encontro una direccion valida
}

// Avanza a la direccion especificada, actualizando el escenario actual y el tiempo del jugador
void avanzar(escenario **s, jugador *j, int dir) {
    *s = (*s) -> adyacentes[dir];
    int tiempoRestado = (int) ceil( ((j -> peso) + 1.0) / 10.0);
    j -> tiempo -= tiempoRestado;
}

// Procesa todas las opciones del juego
int opcionesJuego(escenario **s, jugador *j) {
    while(1) {
        mostrarMenuJuego();
        puts("Seleccione una opcion: ");
        int num;
        verificarOpcion(&num, 6); //Verifica que la opcion ingresada sea valida
        char temp[500]; // Capacidad para leer cadenas de hasta 499 caracteres
        List *listaStr = NULL;
        switch (num) {
        case 1: // Recoger Ítem(s)
            puts("Si se desea elegir otra opcion, escribir \"SALIR\"");
            printf("Ingrese el/los nombre/s de el/los item a recoger (Usar comas sin espacios como separadores): ");

            fgets(temp, sizeof(temp), stdin); // Lee el nombre del item entregado
            temp[strcspn(temp, "\n")] = '\0';
            if (strcmp(temp, "SALIR") == 0) {
                continue;
            }

            listaStr = split_string(temp, ","); // Separa cada input por la coma
            opcion1(*s, j, listaStr);
            listDataClean(listaStr);
            free(listaStr);

            return 0;
        case 2: // Descartar Ítem(s)
            puts("Si se desea elegir otra opcion, escribir: \"SALIR\"");
            printf("Ingrese el/los nombre/s de el/los item a descartar (Usar comas sin espacios como separadores): ");

            fgets(temp, sizeof(temp), stdin);
            temp[strcspn(temp, "\n")] = '\0';
            if (strcmp(temp, "SALIR") == 0) {
                continue;
            }

            listaStr = split_string(temp, ","); // Separa cada input por la coma
            opcion2(j, listaStr);
            listDataClean(listaStr);
            free(listaStr);

            return 0;
        case 3: // Avanzar en una Dirección
        {
            puts("Si se desea elegir otra opcion, escribir \"SALIR\"");
            printf("Ingrese la direccion (ARRIBA, ABAJO, IZQUIERDA o DERECHA): ");
            while (1) {
                fgets(temp, sizeof(temp), stdin);
                temp[strcspn(temp, "\n")] = '\0';
                if (strcmp(temp, "SALIR") == 0) {
                    break;
                }

                int dir = dirValida(*s, temp);
                if (dir == -1)
                    puts("Direccion no valida");
                else {
                    avanzar(s, j, dir);
                    return 0;
                }
            }
            break;
        }
        case 4: // Reiniciar Partida
            return 1;
        case 5: // Salir del Juego
            return 2;
        }
    }
}

// Comprueba si el jugador se encuentra en un estado de fin de partida (sin tiempo o escenario final)
int comprobarTermino(jugador *j,escenario *s) { // retorna 0 si continua la partida, 1 si se pierde y 2 si se llega al final
    if (s -> esFinal) {
        return 2;
    }
    if (j -> tiempo <= 0) {
        return 1;
    }
    return 0;
}

void limpiarJugador(jugador *j) {
    list_clean(j -> inventario);
    free(j -> inventario);
    free(j);
}

// Todos los nodos que apunten a este, pasaran a apuntar a NULL
void borrarConexiones(escenario *s) {
    if (s -> adyacentes[0] != NULL) { // Nodo de Arriba
        s -> adyacentes[0] -> adyacentes[1] = NULL;
    }
    if (s -> adyacentes[1] != NULL) { // Nodo de Abajo
        s -> adyacentes[1] -> adyacentes[0] = NULL;
    }
    if (s -> adyacentes[2] != NULL) { // Nodo de la Izquierda
        s -> adyacentes[2] -> adyacentes[3] = NULL;
    }
    if (s -> adyacentes[3] != NULL) { // Nodo de la Derecha
        s -> adyacentes[3] -> adyacentes[2] = NULL;
    }
}

// Borra la data de tipo item de una List
void borrarListaItems(List *l) {
    item *i = NULL;
    while ((i = list_popBack(l))) {
        free (i -> nombre);
        free(i);
    }
}

// Limpia el nombre, descripcion y lista de objetos de un escenario
void limpiarEscenario(escenario *s) { 
    // Romper conexiones
    borrarConexiones(s);

    // Liberar data
    free(s -> nombre);
    free(s -> desc);
    borrarListaItems(s -> loot);
    free(s -> loot);

    // Liberar el nodo
    free(s);
}

// Revisa si un escenario esta en la lista
int enLista(List *l, escenario *s) {
    for (escenario *i = list_first(l) ; i != NULL ; i = list_next(l)) {
        if ((i -> id) == (s -> id)) return 1;
    }
    return 0;
}

// Limpia el grafo a partir de un objeto en este
void limpiarGrafo(escenario *s) {
    List *vistos = list_create(); // Crear una lista que almacenara los nodos, actuara de manera FIFO
    list_pushBack(vistos, s); // Se guarda el escenario actual en la lista

    while (list_first(vistos) != NULL) { // Hasta que el stack este vacio (se limpien todos los nodos)
        escenario *aux = list_popBack(vistos); // variable auxiliar para guardar el nodo que se saca de la lista
        for (int i = 0 ; i < 4 ; i++) { // Agrega a la lista todos los adyacentes que no sean NULL ni esten en la lista
            escenario *ady = aux -> adyacentes[i];
            if (ady != NULL && !enLista(vistos, ady)) // Si el nodo no se borro previamente y su adyacente no es NULL, se agrega al stack
                list_pushBack(vistos, ady);
        }
        limpiarEscenario(aux); // Limpia el escenario extraido de la lista
    }
    //La lista deberia llegar aqui vacia
    free(vistos);
}

void limpiarPrograma(escenario *s, jugador *j) {
    limpiarJugador(j);
    limpiarGrafo(s);
}

int main() {
    escenario *inicial = NULL; // Almacenara el escenario inicial
    
    mostrarMenuPrincipal(); // Muestra menu principal
    inicial = menuPrincipal(); // Muestra las opciones del menu principal
    // Se inicia el juego
    reinicio: {
        jugador *j = cargarJugador(); // Se carga el estado inicial del jugador
        escenario *actual = inicial; // Marcar posicion actual
        while(1) { // Menu juego
            descEscenario(actual, j); // Muestra la informacion del escenario actual
            
            int opcion = opcionesJuego(&actual, j); // Muestra las diferentes opciones
                // 0: Avanzar normalmente
                // 1: Reiniciar Partida
                // 2: Salir del juego
            if (opcion == 1) { // Para reiniciar partida
                limpiarJugador(j);
                goto reinicio;
            }
            else if (opcion == 2) { // Par salir del juego
                break;
            }

            int termino = comprobarTermino(j,actual);
            printf("%d\n", termino);
            if (termino == 0)
                continue; // Continuar normalmente con las instrucciones

            // Se entrara aqui solo si termina la partida (termino == 1 || termino == 2)
            if (termino == 1) { // Se acabo el tiempo
                puts("========================================");
                puts("DERROTA (El tiempo llego a 0)");
                puts("========================================");
            }
            else { // termino == 2 (Se encontro la salida)
                puts("========================================");
                puts("Has llegado a la salida!");
                puts("Items recogidos: ");
                if (!imprimirItems(j -> inventario)) {
                    puts("No recogiste ningun item");
                }
                printf("Puntaje: %d \n", j -> puntaje);
                puts("========================================");
            }
            
            puts("Inserte \"1\" para reiniciar la partida o \"2\" para cerrar el juego");
                int num;
                verificarOpcion(&num, 2); //Verifica que la opcion ingresada sea valida
                if (num == 1) { // Reiniciar Partida
                    limpiarJugador(j); 
                    goto reinicio;
                }
                else // Salir del juego
                    break;
        }
        // Se llegara aca cuando se termine el juego
        puts("Saliendo de GraphQuest...");
        limpiarPrograma(actual, j);
        exit(EXIT_SUCCESS);
        return 0; // Salir del programa
    }
}

