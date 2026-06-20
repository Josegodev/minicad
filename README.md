# MiniCAD

MiniCAD educativo en C++17. La version actual abre un visor 3D minimo usando SDL2 + OpenGL, con matematicas propias para vectores, matrices, camara y geometria basica.

El render actual no escala una imagen de baja resolucion: OpenGL dibuja directamente a la resolucion real de la ventana.

## Estado Actual

- Ventana SDL2 con contexto OpenGL.
- MSAA solicitado para antialiasing basico.
- Depth test activado.
- Fondo gris claro.
- Ejes 3D X/Y/Z.
- Una figura wireframe seleccionable por terminal o argumento.
- Camara orbital mirando al origen.

## Estructura

- `include/langcad/core`: tipos comunes como `Vec3`, `Mat4`, `Edge` y `Mesh`.
- `include/langcad/geometry`: figuras independientes que heredan de `Shape3D` y fabrica de figuras.
- `include/langcad/scene`: `Scene`, que almacena figuras 3D.
- `include/langcad/render`: camara, proyeccion y renderer SDL2 + OpenGL.
- `src/core`, `src/geometry`, `src/scene`, `src/render`: implementaciones de esos modulos.

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

Al iniciar sin argumentos, el programa muestra un menu por terminal y pide que figura quieres ver:

```text
1 = Cube
2 = Cylinder
3 = Pyramid
4 = Sphere
```

Tambien puedes abrir una figura directamente pasando su numero como argumento:

```bash
./build/minicad 1  # Cubo
./build/minicad 2  # Cilindro
./build/minicad 3  # Piramide
./build/minicad 4  # Esfera
```

La escena contiene solamente la figura seleccionada, centrada en el origen y alineada con los ejes 3D.

## Controles

- `ESC`: cerrar la aplicacion.
- `A` / `D`: rotar la camara horizontalmente.
- `W` / `S`: rotar la camara verticalmente.
- Flechas izquierda/derecha: rotar horizontalmente.
- Flechas arriba/abajo: rotar verticalmente.

## Notas Tecnicas

- La capa matematica no depende de SDL2 ni de OpenGL.
- `ShapeFactory` centraliza la seleccion de figuras para mantener `main.cpp` simple.
- El renderer dibuja `Mesh`; no conoce clases concretas como cubo, cilindro, piramide o esfera.
- El renderer usa OpenGL en modo compatible/simple para mantener el codigo claro.
- El antialiasing depende de que el driver acepte el framebuffer multisample solicitado por SDL.
