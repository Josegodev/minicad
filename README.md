# MiniCAD

MiniCAD educativo en C++17. La version actual abre un visor 3D minimo usando SDL2 + OpenGL, con matematicas propias para vectores, matrices, camara y geometria basica.

El render actual no escala una imagen de baja resolucion: OpenGL dibuja directamente a la resolucion real de la ventana. `PixelCanvas` sigue existiendo como laboratorio 2D, pero la aplicacion principal usa la nueva capa 3D.

## Estado Actual

- Ventana SDL2 con contexto OpenGL.
- MSAA solicitado para antialiasing basico.
- Depth test activado.
- Fondo gris claro.
- Ejes 3D X/Y/Z.
- Cubo o cilindro wireframe centrado en el origen.
- Camara orbital mirando al origen.

## Estructura

- `include/Vec3.hpp`, `src/Vec3.cpp`: vector 3D matematico puro.
- `include/Mat4.hpp`, `src/Mat4.cpp`: matrices 4x4, perspectiva y `lookAt`.
- `include/Camera.hpp`, `src/Camera.cpp`: camara orbital simple.
- `include/Mesh.hpp`, `src/Mesh.cpp`: geometria minima, por ahora cubo y cilindro.
- `include/OpenGLRenderer.hpp`, `src/OpenGLRenderer.cpp`: render 3D con SDL2 + OpenGL.
- `include/PixelCanvas.hpp`, `src/PixelCanvas.cpp`: canvas 2D por pixeles, conservado como laboratorio.
- `include/CoordinateSystem.hpp`, `src/CoordinateSystem.cpp`: sistema de coordenadas 2D del laboratorio anterior.

## Requisitos

- CMake 3.16 o superior.
- Compilador compatible con C++17.
- SDL2 y sus cabeceras de desarrollo.
- OpenGL y sus cabeceras de desarrollo.

En Ubuntu/Debian puedes instalar las dependencias con:

```bash
sudo apt install cmake g++ libsdl2-dev libgl1-mesa-dev
```

## Compilar

Desde la raiz del proyecto:

```bash
cmake -S . -B build
cmake --build build
```

## Ejecutar

```bash
./build/minicad
```

Al iniciar, el programa pide que figura quieres ver:

```text
Elige figura 3D:
  1) cubo
  2) cilindro
> 
```

Despues se abre una ventana 3D con los ejes y la figura seleccionada centrada.

## Controles

- `ESC`: cerrar la aplicacion.
- `A` / `D`: rotar la camara horizontalmente.
- `W` / `S`: rotar la camara verticalmente.
- Flechas izquierda/derecha: rotar horizontalmente.
- Flechas arriba/abajo: rotar verticalmente.

## Notas Tecnicas

- La capa matematica no depende de SDL2 ni de OpenGL.
- El renderer usa OpenGL en modo compatible/simple para mantener el codigo claro.
- El antialiasing depende de que el driver acepte el framebuffer multisample solicitado por SDL.
