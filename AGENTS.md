# AGENTS.md

## Project Intent

MiniCAD is evolving from an educational mesh/render prototype into a CAD application with a future in-house B-Rep kernel, compared against OpenCascade, and assisted by Ollama.

Ollama is only an inference component. It may propose structured operations or shape definitions, but it must never be treated as an authority, executor, renderer, filesystem actor, or kernel component.

## Architectural Rules

- Keep the future B-Rep kernel independent from SDL2, OpenGL, ImGui, HTTP, Ollama, and OpenCascade.
- Treat `Mesh` as render/presentation output, not as authoritative CAD geometry.
- Treat `Shape3D` as legacy/renderable behavior. It is not the final CAD entity model.
- OpenCascade should be used as an adapter/reference backend, not as an implicit dependency of the custom kernel.
- API code must not define CAD semantics. It should parse, validate, call domain logic, and serialize results.
- Renderer/editor code must not own domain truth. It should display documents/entities produced elsewhere.
- LLM code must produce closed, validated JSON contracts only. Never let LLM output introduce new fields, code, commands, paths, scripts, or behavior.

## Refactor Direction

Prefer small changes that move the code toward these layers:

- `kernel`: B-Rep/topology, analytic geometry, tolerances, deterministic operations.
- `operations`: make box/cylinder/sphere, extrude, boolean ops, future feature commands.
- `occ_adapter`: OpenCascade conversions and comparisons.
- `render`: tessellation and OpenGL presentation.
- `document`: persistent CAD model, feature history, stable IDs.
- `api`: HTTP/JSON boundary.
- `llm`: Ollama prompt/schema handling and proposal validation.
- `editor`: SDL/OpenGL/ImGui UI and interaction.

## Coding Guidelines

- Prefer minimal, safe refactors over broad rewrites.
- Do not rename public JSON fields unless compatibility is intentionally handled.
- Keep current endpoints working unless explicitly changing API behavior.
- Preserve `render` in JSON while internal code may use clearer names like `tessellation`.
- Avoid adding dependencies to `langcad_kernel`.
- If a change makes `langcad_kernel` depend on UI, HTTP, Ollama, or OCC, reconsider the design.
- Keep validation deterministic and independent from LLM behavior.
- Add comments only when they clarify architectural boundaries or non-obvious CAD assumptions.

## README Accuracy

When changing behavior, update `README.md`.

Be explicit about:

- `./build/minicad` opens the native editor.
- `./build/langcad_api` starts the API only and does not open UI.
- `ui-test/index.html` is a web console served separately.
- `tests/api_prompt_injection.sh` tests API behavior, not browser UI.
- Render endpoints open a native SDL/OpenGL window only after receiving a render request.

## Testing

After code changes, prefer:

```bash
cmake --build build
```

For API prompt safety checks:

```bash
./build/langcad_api
tests/api_prompt_injection.sh
```

For UI web checks:

```bash
./build/langcad_api
cd ui-test
python3 -m http.server 5173
```

Then open:

```text
http://127.0.0.1:5173
```

## Review Focus

When reviewing changes, prioritize:

- Semantic drift between CAD model, mesh, scene, render, API, and LLM layers.
- New coupling into `langcad_kernel`.
- LLM output bypassing validation.
- Render/tessellation options leaking into geometry identity.
- OpenCascade becoming hidden kernel truth instead of explicit adapter/reference.
- README instructions becoming inaccurate.
