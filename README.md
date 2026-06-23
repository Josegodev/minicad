# MiniCAD

MiniCAD educativo en C++17. La version actual abre un visor 3D minimo usando SDL2 + OpenGL, con matematicas propias para vectores, matrices, camara y geometria basica.

El objetivo arquitectonico es evolucionar hacia un kernel CAD/B-Rep propio, comparado en paralelo con OpenCascade, y asistido por Ollama solo como motor de inferencia para proponer operaciones/figuras en formatos predefinidos. Ollama no ejecuta codigo, no toca el renderer y no es autoridad geometrica.

El render actual no escala una imagen de baja resolucion: OpenGL dibuja directamente a la resolucion real de la ventana.

## Estado Actual

- Ventana SDL2 con contexto OpenGL.
- MSAA solicitado para antialiasing basico.
- Depth test activado.
- Fondo gris claro.
- Ejes 3D X/Y/Z.
- Una figura wireframe seleccionable por terminal o argumento.
- Camara orbital mirando al origen.
- Separacion inicial de build entre `langcad_kernel` y la capa de integracion `langcad_core`.

## Estructura

- `include/langcad/core`: tipos matematicos comunes como `Vec3`, `Mat4`, `Edge` y `Mesh`.
- `include/langcad/geometry`: primitivas renderizables actuales, catalogo, validacion y fabrica de figuras.
- `include/langcad/api`: contrato JSON, servidor HTTP, trazas y logs.
- `include/langcad/llm`: cliente Ollama, separado de geometria y renderer.
- `include/langcad/scene`: `Scene`, que almacena figuras 3D.
- `include/langcad/render`: camara, proyeccion y renderer SDL2 + OpenGL.
- `src/core`, `src/geometry`, `src/api`, `src/llm`, `src/scene`, `src/render`: implementaciones de esos modulos.

Targets CMake principales:

- `langcad_kernel`: matematicas, primitivas propias, validacion y escena. No enlaza SDL2, OpenGL, Ollama ni OpenCascade.
- `langcad_core`: capa de integracion actual con API, LLM, render y adaptadores OpenCascade.
- `langcad_editor`: UI/editor sobre la capa de integracion.

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

Editor nativo SDL/OpenGL/ImGui:

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

La escena inicial contiene la figura seleccionada, centrada en el origen y alineada con los ejes 3D. Desde el panel `CAD Tools` puedes crear mas figuras y manipular la seleccion con ImGuizmo.

API local sin UI:

```bash
./build/langcad_api
```

Este comando no abre una ventana ni un navegador. Deja la API escuchando en `http://127.0.0.1:8080` y espera peticiones HTTP.

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

`/api/v1/shapes/render` y `/api/v1/llm/render` abren una ventana persistente del editor CAD con ImGui + ImGuizmo solo cuando reciben una peticion. Cada nuevo render de API reemplaza el anterior; para cerrar la ventana activa usa el cierre de ventana o `ESC`.

Consola web local acoplada a la API C++:

Terminal 1, API:

```bash
./build/langcad_api
```

Terminal 2, servidor estatico de la UI:

```bash
cd ui-test
python3 -m http.server 5173
```

Abre `http://127.0.0.1:5173` en el navegador. La UI consume `/api/v1/health`, `/api/v1/shapes`, `/api/v1/shapes/*` y `/api/v1/llm/*`.

El boton `Render` de la consola web llama a la API y entonces la API abre la ventana nativa SDL/OpenGL. La pagina web no incrusta el viewport 3D.

Pruebas minimas de prompt injection de la API:

Terminal 1, API:

```bash
./build/langcad_api
```

Terminal 2, smoke test HTTP:

```bash
tests/api_prompt_injection.sh
```

Este script no prueba la UI web ni abre un navegador. Envia prompts maliciosos a `/api/v1/llm/shape-definition` y `/api/v1/llm/render`, y muestra las respuestas JSON esperadas para comprobar que la API rechaza codigo, rutas, comandos, cambios de schema y multiples figuras.

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
- `ShapeFactory` crea figuras renderizables desde una seleccion local o desde una `ShapeDefinition` ya validada.
- Los endpoints `/api/v1/shapes/*` son deterministicos; los endpoints `/api/v1/llm/*` usan Ollama solo para generar JSON estructurado.
- Ollama no ejecuta codigo ni toca el renderer: su salida se valida antes de crear figuras.
- Cada request devuelve `trace_id` y escribe un log JSON por linea con endpoint, modelo, figura, dimensiones, `validation_ok`, `render_started`, `error_type`, latencia y estado.
- El renderer dibuja `Mesh`; no conoce clases concretas como cubo, cilindro, piramide o esfera.
- `Shape3D` queda como alias legacy de `RenderableShape`: representa una fuente de `Mesh`, no el modelo CAD/B-Rep autoritativo.
- Internamente `ShapeDefinition` usa `tessellation` para opciones de discretizacion. La API sigue aceptando y devolviendo el campo JSON `render` por compatibilidad.
- El renderer usa OpenGL en modo compatible/simple para mantener el codigo claro.
- El antialiasing depende de que el driver acepte el framebuffer multisample solicitado por SDL.

## Reglas Para Escalar A Kernel B-Rep

- El kernel B-Rep no debe depender de SDL2, OpenGL, ImGui, HTTP, Ollama ni OpenCascade.
- `Mesh` es una salida de presentacion/render, no la verdad geometrica del CAD.
- OpenCascade debe funcionar como adaptador y referencia de comparacion, no como dependencia implicita del kernel propio.
- Ollama debe proponer operaciones en un contrato cerrado y validable; el sistema decide si son validas.
- Las opciones de teselacion/discretizacion no deben cambiar la identidad geometrica de una figura.
- La escena/editor puede tener seleccion, visibilidad y gizmos; el documento CAD futuro debe tener entidades, operaciones, historial e IDs persistentes.

Direccion esperada de capas:

- `kernel`: B-Rep propio, tolerancias, topologia, geometria analitica y operaciones deterministas.
- `operations`: primitivas, extrusiones, booleanas, fillets y comandos auditables.
- `occ_adapter`: conversion y comparacion contra OpenCascade.
- `render`: teselacion a `Mesh` y dibujo.
- `document`: arbol parametricamente reproducible.
- `api`: contratos HTTP/JSON sin control directo de ventana.
- `llm`: propuestas de operaciones, siempre validadas por dominio.
