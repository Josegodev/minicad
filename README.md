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
- `include/langcad/api`: contrato JSON, servidor HTTP, trazas y logs.
- `include/langcad/llm`: cliente Ollama, separado de geometria y renderer.
- `include/langcad/scene`: `Scene`, que almacena figuras 3D.
- `include/langcad/render`: camara, proyeccion y renderer SDL2 + OpenGL.
- `src/core`, `src/geometry`, `src/api`, `src/llm`, `src/scene`, `src/render`: implementaciones de esos modulos.

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

Visor interactivo:

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

API local:

```bash
./build/langcad_api
```

Variables opcionales:

```bash
LANGCAD_API_PORT=8080
OLLAMA_URL=http://localhost:11434
OLLAMA_MODEL=qwen2.5-coder:7b
```

Endpoints principales:

```bash
curl http://127.0.0.1:8080/api/v1/health
curl http://127.0.0.1:8080/api/v1/shapes

curl -X POST http://127.0.0.1:8080/api/v1/shapes/validate \
  -H 'Content-Type: application/json' \
  -d '{"shape_type":"sphere","dimensions":{"radius":1.5},"units":"unitless","render":{"segments":24,"rings":12},"centered":true}'

curl -X POST http://127.0.0.1:8080/api/v1/shapes/mesh \
  -H 'Content-Type: application/json' \
  -d '{"shape_type":"cube","dimensions":{"size":2},"units":"unitless","render":{},"centered":true}'

curl -X POST http://127.0.0.1:8080/api/v1/llm/shape-definition \
  -H 'Content-Type: application/json' \
  -d '{"prompt":"un cubo de tamano 2"}'
```

`/api/v1/shapes/render` y `/api/v1/llm/render` abren una ventana persistente del editor CAD con ImGui + ImGuizmo y una unica figura validada. Cada nuevo render de API reemplaza el anterior; para cerrar la ventana activa usa el cierre de ventana o `ESC`.

UI local acoplada a la API C++:

```bash
./build/langcad_api
cd ui-test
python3 -m http.server 5173
```

Abre `http://127.0.0.1:5173`. La UI consume `/api/v1/health`, `/api/v1/shapes`, `/api/v1/shapes/*` y `/api/v1/llm/*`.

Pruebas minimas de prompt injection:

```bash
./build/langcad_api
tests/api_prompt_injection.sh
```

El script asume `http://127.0.0.1:8080`. Puedes cambiarlo con:

```bash
API_URL=http://127.0.0.1:8080 tests/api_prompt_injection.sh
```

## Controles

- `ESC`: cerrar la aplicacion.
- `A` / `D`: rotar la camara horizontalmente.
- `W` / `S`: rotar la camara verticalmente.
- `Q` / rueda arriba: acercar la camara.
- `E` / rueda abajo: alejar la camara.
- Flechas izquierda/derecha: rotar horizontalmente.
- Flechas arriba/abajo: rotar verticalmente.

## Notas Tecnicas

- La capa matematica no depende de SDL2 ni de OpenGL.
- `ShapeFactory` crea figuras desde una seleccion local o desde una `ShapeDefinition` ya validada.
- Los endpoints `/api/v1/shapes/*` son deterministicos; los endpoints `/api/v1/llm/*` usan Ollama solo para generar JSON estructurado.
- Ollama no ejecuta codigo ni toca el renderer: su salida se valida antes de crear figuras.
- Cada request devuelve `trace_id` y escribe un log JSON por linea con endpoint, modelo, figura, dimensiones, `validation_ok`, `render_started`, `error_type`, latencia y estado.
- El renderer dibuja `Mesh`; no conoce clases concretas como cubo, cilindro, piramide o esfera.
- El renderer usa OpenGL en modo compatible/simple para mantener el codigo claro.
- El antialiasing depende de que el driver acepte el framebuffer multisample solicitado por SDL.
