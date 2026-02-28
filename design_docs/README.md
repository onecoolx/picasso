# design_docs

Project-internal design notes intended as **long-term memory** for ongoing development.

## Documents
- `svg_tiny12_animation_design.md`
  - SVG Tiny 1.2 animation subset: design, current progress, and next plan.

## Update policy
- Update the relevant doc whenever:
  - A new SVG animation feature lands.
  - Semantics change (timing, fill/repeat, calcMode, etc.).
  - Parser storage format changes (critical for player decoding).
  - New debug/test hooks are introduced.
