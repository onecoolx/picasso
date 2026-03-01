# design_docs

Project-internal design notes intended as **long-term memory** for ongoing development.

## Engineering constraints
- Language: **C++98** only.
- **No STL containers**, no lambdas/closures.
- Prefer explicit ownership and deterministic evaluation.
- Tests: **GoogleTest** (prefer `EXPECT_*` when cleanup must always run).

## Working method (TDD)
- Work in small steps: **red test → minimal implementation → refactor/cleanup**.
- After each step:
  - run the focused test(s) and then the full `./unit_tests` regression.
  - update docs so they match current behavior.

## AI-agent workflow rules
- Do **not** auto-commit.
- Provide **commitlint-style commit messages only**; the user will review and commit manually.
- Avoid introducing new public APIs unless required; test hooks are allowed when justified.

## Documents
- `svg_tiny12_animation_design.md`
  - SVG Tiny 1.2 animation subset: design, current progress, and next plan.

## How to validate
- Build: use the existing CMake build under `proj/`.
- Run tests:
  - focused: `./unit_tests --gtest_filter='SVGPlayerTest.*'`
  - full: `./unit_tests`

## Update policy
- Update the relevant doc whenever:
  - A new SVG animation feature lands.
  - Semantics change (timing, fill/repeat, calcMode, etc.).
  - Parser storage format changes (critical for player decoding).
  - New debug/test hooks are introduced.
