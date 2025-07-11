# MegaManX

Este repositorio contiene un videojuego que recrea el juego de MegamanX de SNES con un motor creado desde 0 en C++.

## Imágenes del juego

### Level Selector
![overview1](/Screenshots/overview1.png "overview1")

### Chill Penguin stage
![overview2](/Screenshots/overview2.png "overview1")

### Flame Mammoth stage
![overview3](/Screenshots/overview3.png "overview3")

## Contenido

- **`docs`**  
  Toda la documentación del motor hecho con SDL2.

- **`build`**  
  Juego ejecutable en Windows.

- **`engine`**  
  Código fuente del motor.

- **`game`**  
  Código fuente del juego.

- **`resources`**  
  Todos los assets que componen el juego.


## Requisitos

### Linux
Tener instalado SDL2, SDL2 mixer y un compilador de C++.

### Windows
Tener instalado un compilador de C++ y extraer el zip "dev.zip" en el disco local C.

## Instalación y Compilación

Clona el repositorio:
```sh
git clone https://github.com/DonJulve/MegaManX
```

Borra la carpeta build.

### Windows

```sh
./build.bat
```

### Linux
```sh
./build.sh
```

## Teclonogías empleadas:

- **Shell** y **Batch scripting**: Lenguaje de scripting usado para poder compilar.
- **C++**: Lenguaje en el que está escrito todo el código del juego.

## Problemas:

En Linux el juego tiene un bug visual que hace que los niveles salgan en negro, aún no se ha resuelto y se aceptan pull requests.
