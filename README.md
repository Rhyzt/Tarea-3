# GraphQuest

## Descripción

E## Descripción

GraphQuest es un juego de exploración en el que el jugador recorre un laberinto compuesto por distintos escenarios interconectados. En cada escenario, el jugador puede tomar decisiones que afectan su progreso, recolectar objetos con valor y peso, o desplazarse hacia otras áreas del laberinto.

El juego permite cargar la estructura del laberinto desde un archivo CSV, lo que facilita la personalización y expansión de niveles. Durante la partida, se muestra al jugador información relevante sobre su estado actual, incluyendo la ubicación, el tiempo restante, y los objetos en su inventario.

El objetivo principal es alcanzar la salida antes de que se agote el tiempo, maximizando el puntaje obtenido mediante la recolección de objetos valiosos en el camino. Las decisiones estratégicas, como qué objetos llevar o descartar y qué ruta seguir, son clave para lograr la victoria.

## Cómo compilar y ejecutar
Este sistema ha sido desarrollado en lenguaje C y puede ejecutarse fácilmente utilizando **Visual Studio Code** junto con una extensión para C/C++, como **C/C++ Extension Pack** de Microsoft. Para comenzar a trabajar con el sistema en tu equipo local, sigue estos pasos:

### Requisitos previos:

- Tener instalado [Visual Studio Code](https://code.visualstudio.com/).
- Instalar la extensión **C/C++** (Microsoft).
- Tener instalado un compilador de C (como **gcc**). Si estás en Windows, se recomienda instalar [MinGW](https://www.mingw-w64.org/) o utilizar el entorno [WSL](https://learn.microsoft.com/en-us/windows/wsl/).

### Pasos para compilar y ejecutar:

1. **Descarga y descomprime el** archivo `.zip` en una carpeta de tu elección.
2. **Abre el proyecto en Visual Studio Code**
    - Inicia Visual Studio Code.
    - Selecciona `Archivo > Abrir carpeta...` y elige la carpeta donde descomprimiste el proyecto.
3. **Compila el código**
    - Abre el archivo principal (`main.c`).
    - Abre la terminal integrada (`Terminal > Nueva terminal`).
    - En la terminal, compila el programa con el siguiente comando:
        
        ```bash
        gcc *.c tdas/*.c -o GraphQuest
        ```
        
4. **Ejecuta el programa**
    - Una vez compilado, puedes ejecutar la aplicación con:
        
        ```
        ./GraphQuest
        ```
        

## Funcionalidades

### Funcionando correctamente:
Para el correcto funcionamiento de la aplicación, se asumen las siguientes condiciones:

**Interacción del usuario:** 
Se asume que el usuario interactúa correctamente con la aplicación, entregando siempre datos válidos y pertinentes. Por ejemplo:
- En los menús, el usuario selecciona únicamente opciones dentro del rango permitido (por ejemplo, del 1 al 6).
- No se ingresan caracteres alfabéticos u otros símbolos cuando se solicita un valor numérico.

**Formato del archivo CSV:**
El archivo que contiene la información del laberinto debe cumplir con las siguientes condiciones:
- Las IDs de los escenarios deben estar ordenadas de forma secuencial, comenzando desde 1 y aumentando sin omisiones (1, 2, 3, ...).
- No deben existir ítems repetidos entre escenarios. Cada ítem debe tener un identificador único dentro del laberinto.
- Ubicación del archivo CSV: El archivo de entrada se debe encontrar en la carpeta data, siguiendo el formato estructural ejemplificado por el archivo base proporcionado.
- Tomar como ejemplo el archivo csv que se encuentra en la carpeta data.

### Opciones posibles
- Cargar laberinto de un archivo, que tenga 8 columnas correspondientes a: id,nombre,descripcion,items,arriba,abajo,izquierda,derecha,esFinal(la primera linea se saltara ya que se asume que posee los nombres de las columnas)
- Recoger Items del escenario actual.
- Descartar Items del inventario del jugador.
- Avanzar en una direccion.
- Reiniciar Partida.
- Salir del juego.

### A mejorar:

- Mejorar los mensajes para algunos casos (como cuando se recogen/descartan items que no se poseen)
=======


## Ejemplo de uso

**Primer Paso: Cargar el laberinto**

Se carga la informacion del laberinto para el juego.

```
Ingrese la direccion del archivo CSV: data\graphquest.csv
```

La aplicación cargará en memoria todos los datos contenidos en el archivo, incluyendo los escenarios, sus descripciones, ítems disponibles y conexiones entre ellos. Esta información se procesará automáticamente para construir la estructura interna del juego y habilitar las distintas opciones de navegación e interacción del jugador.

Si la carga del archivo se realiza correctamente, el sistema mostrará los datos del escenario actual junto con una lista de acciones disponibles. A partir de este punto, el jugador podrá comenzar a explorar el laberinto, tomar decisiones, recolectar objetos y avanzar hacia la salida antes de que se agote el tiempo.

**Opcion 1: Recoger Item(s)**

Se recoge el Item del escenario actual.

```
Opción seleccionada: 1) Recoger Item(s)
Si se desea elegir otra opcion, escribir "SALIR"
Ingrese el/los nombre/s de el/los item a recoger (Usar comas sin espacios como separadores):
```

El jugador debe ingresar los nombres de los ítems tal como aparecen en el escenario, separados por comas y sin espacios (por ejemplo: Espada,Escudo,Pocion). Todos los ítems ingresados correctamente serán añadidos al inventario del jugador, siempre que el item no este previamente presente en el inventario del jugador.

Si se escribe la palabra "SALIR" (en mayusculas), se volvera al menu de opciones, descartando la ultima accion. Esto tambien se puede hacer en la opcion 2 y 3.

**Opcion 2: Descartar Item(s)**

```
Opción seleccionada: 2) Descartar Item(s)
IIngrese el/los nombre/s de el/los item a descartar (Usar comas sin espacios como separadores): Espada
```

Los ítems indicados se eliminarán del inventario del jugador, reduciendo el peso y valor de este de su inventario. Esta acción tiene una penalizacion de 1 de tiempo.

**Opcion 3: Avanzar en una Direccion**

```
Opción seleccionada: 3) Avanzar en una Direccion
Ingrese la direccion (ARRIBA, ABAJO, IZQUIERDA o DERECHA):
```

Se ingresa una opcion valida (las cuales se habran mencionado en la descripcion del escenario) y el jugador procede a moverse al escenario en esa direccion. Reduciendo el tiempo segun la siguiente funcion: 

**Tperdido = ceil((peso_total + 1) / 10)**
- peso_total: es la suma de los pesos de todos los ítems que el jugador lleva en su inventario.
- ceil (función techo): redondea hacia arriba para garantizar que se pierda al menos un turno.


**Opcion 4: Reiniciar Partida**

```
Opción seleccionada: 4) Reiniciar Partida
```
Cuando el jugador selecciona esta opción, se reinicia el estado del juego a su condición inicial. Esto implica que:
- El inventario del jugador queda vacío.
- El jugador vuelve al escenario inicial del laberinto.
- Se restablecen las variables relacionadas con el tiempo y puntaje al inicio.


**Opcion 5: Salir del Juego**

```
Opción seleccionada: 5) Salir del Juego
```
El programa limpia todo lo que tenga almacenado en ese momento y luego se cierra.

=======