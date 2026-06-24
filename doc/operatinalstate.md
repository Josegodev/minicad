# Operational State

## Estado General

MiniCAD mantiene una separacion entre geometria/dominio, API, LLM y render. La capa `langcad_kernel` sigue siendo la zona pura para matematicas, validacion, primitivas renderizables actuales, escena y ahora geometria skeleton interna. No depende de SDL2, OpenGL, ImGui, HTTP, Ollama ni OpenCascade.

## Geometria Renderizable Actual

- `Shape3D` es un alias legacy de `RenderableShape`.
- `Shape3D` no es el modelo CAD final; solo produce `core::Mesh` para presentacion.
- `Mesh` contiene vertices, normales, aristas y triangulos, pero sigue siendo salida de render, no verdad geometrica CAD.
- `faceted_shape` permite representar una figura por vertices y caras.
- El runtime deriva aristas desde los loops de caras para exponer vertices, aristas y caras como elementos separados.
- Las aristas explicitas se mantienen para casos wireframe/manuales, pero los solidos deben derivarlas desde caras.

## API Y LLM

- Los endpoints `/api/v1/shapes/*` aceptan JSON manual validado.
- Los endpoints `/api/v1/llm/*` usan Ollama solo para producir JSON cerrado.
- Ollama no ejecuta codigo, no toca archivos, no renderiza y no decide la verdad geometrica.
- Para geometria dibujable, Ollama debe proponer `faceted_shape` con informacion minima: vertices y caras.
- La validacion local normaliza la definicion y deriva aristas antes de crear la figura o generar malla.
- La salida normalizada expone vertices, aristas y caras.

## Render

- El renderer consume `Mesh` y no conoce clases concretas de dominio.
- Las caras se triangulan para OpenGL.
- Las aristas se dibujan como lineas.
- Los vertices aislados pueden dibujarse como puntos.
- El render no debe crear ni poseer verdad CAD.

## Skeleton Interno

Se introdujo un modelo interno inicial para geometria skeleton en `langcad_kernel`:

- `SkeletonDefinition`
- `AxisDefinition`
- `PlaneDefinition`
- `ResolvedAxis`
- `ResolvedPlane`
- `SkeletonResolutionResult`
- `resolveSkeleton(...)`

Los ejes se definen por vector y se normalizan. Los planos resueltos usan la forma:

```text
normal dot point = offset
```

Tipos de planos soportados internamente:

- Plano desde plano coordenado `XY`, `YZ` o `XZ` y distancia.
- Plano paralelo desde otro plano, heredando normal y sumando distancia.
- Plano desde eje normal y distancia al origen.
- Plano desde eje normal y distancia respecto a otro plano.

Para un plano definido por eje normal desde otro plano se usa:

```text
new_normal = normalize(axis.direction)
reference_point = reference_plane.normal * reference_plane.offset
new_offset = dot(new_normal, reference_point) + distance
```

Validaciones actuales:

- IDs de ejes y planos no vacios.
- IDs unicos por tipo.
- Ejes no nulos.
- Referencias a ejes y planos existentes.
- Rechazo de referencias no resolubles o ciclicas.

## Limitaciones Actuales

- El skeleton todavia no esta expuesto por API JSON.
- Ollama todavia no propone skeletons.
- No hay persistencia de documento CAD ni historial de features.
- `Shape3D` y las primitivas existentes siguen siendo renderizables legacy.
- No existe aun un B-Rep propio autoritativo.

## Proximos Pasos Recomendados

1. Exponer skeleton por API manual con contrato cerrado y validacion deterministica.
2. Anadir tests unitarios para `resolveSkeleton`.
3. Conectar skeleton con futuras operaciones: sketches, extrusiones y features.
4. Permitir que Ollama proponga skeletons solo despues de que la API manual este estable.
5. Introducir un documento CAD persistente con entidades, IDs estables e historial.
6. Mantener OpenCascade como adaptador/referencia, no como verdad implicita del kernel.
