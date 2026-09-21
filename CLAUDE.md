# CLAUDE.md - Project Guidelines

**IMPORTANT: Read and respect this document before executing any task.**

## Build & Compilation
- **Toolchain**: xmake, MSVC (Windows)
- **CI/CD**: GitHub Actions (`windows-latest`)
- **Build Command**: `xmake`
- **Target**: CommonLibF4 / F4SE Plugin

## Code Rules & Preferences
- **Language**: C++23
- **Namespace**: All project-specific logic must reside within the `Traversal` namespace.
- **Includes**:
    - Project headers: Relative to `src/` (e.g., `#include "ledge_detector.h"`).
    - CommonLibF4 headers: Use the `<RE/...>` pattern.
    - **CRITICAL**: Use `<RE/P/...>` for Player-related headers. NEVER use `<RE/F4/...>`.
    - **Windows Headers**: `<Windows.h>` must be isolated. Include it AFTER project headers and always wrap it with `WIN32_LEAN_AND_MEAN` and `NOMINMAX` to prevent macro pollution.
- **Coding Style**:
    - Minimal changes: Prefer surgical edits over rewriting.
    - Singleton Pattern: Use thread-safe local static variables for singletons.
    - Logging: Use `REX::INFO`, `REX::ERROR` for diagnostics.
- **Constraints**:
    - Do not invent IDs, offsets, or includes that do not exist in `lib/commonlibf4/include`.
    - Do not modify `xmake.lua` or GitHub workflow files unless explicitly requested.

## Git Workflow
- **Branch**: Work on `master`.
- **Commits**: 
    - Use clear, descriptive messages.
    - Every commit must end with: `Co-Authored-By: Claude Code <noreply@anthropic.com>`.
- **Process**: `git add .` $\rightarrow$ `git commit` $\rightarrow$ `git push origin master`.

## Communication
- **Language**: Always communicate with the user in Portuguese. Thinking/reasoning can be in English, but all output must be in Portuguese.
- `src/main.cpp`: Entry point, F4SE messaging loop.
- `src/ledge_detector.cpp/h`: Ledge detection logic (3-ray method) and state management.
- `src/hotkey.cpp/h`: Input polling thread (detached).
- `src/debug.cpp/h`: Debugging utilities and snapshots.

# Pesquisa e Download Autônomo de Dependências
- Se a compilação via `xmake` falhar por falta de alguma biblioteca, arquivo de cabeçalho (.h / .hpp) ou ferramenta:
  1. Use o comando `ddgr "termo da busca"` ou `curl` para pesquisar a solução na internet.
  2. Baixe o repositório ou arquivo necessário usando `git clone`, `wget` ou `curl` para a pasta apropriada do projeto (ex: `lib/`).
  3. Tente compilar novamente via `xmake` para confirmar a correção.
