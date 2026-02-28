# SVG Tiny 1.2 Animation (Parser + Player) — Design / Progress / Plan

> Last updated: 2026-02-28

## 0. Scope & Constraints

### Goal
Implement a **practical SVG Tiny 1.2 animation subset** for this project, using **TDD** and keeping the implementation small, deterministic, and testable.

### Hard constraints
- **C++98** only.
- **No STL containers**, no lambdas/closures.
- Keep allocations explicit; avoid hidden ownership.
- Tests are written with **GoogleTest**.
- Beware of sanitizer behavior: `ASSERT_*` early-exit can skip cleanup and look like leaks; prefer `EXPECT_*` when cleanup must always run.

### Supported / Targeted SVG subset
We implement a staged subset of SVG Tiny 1.2 animation:
- Timing: `begin` / `end` lists, `repeatCount`, `repeatDur`, `dur`, `fill=freeze|remove`, and a minimal external event trigger (e.g. `begin="click"`).
- Value sampling/interpolation: `values`, optional `keyTimes`, `calcMode` = `linear` (default), `discrete`, `spline` (cubic-bezier), `paced`.
- Property animation currently covered by tests:
  - Numeric: `x`, `y`, `width`, `height`, `opacity`, `rx`, `ry`, `stroke-width`, `fill-opacity`, `stop-opacity`.
  - Color: minimal `animateColor` for `fill` (discrete).
   - Transform: minimal `animateTransform` for `translate` (linear default + discrete), plus `scale` (discrete) and `rotate` (discrete + linear), tracked as matrix `a,b,c,d,e,f`.

Non-goals for now (explicitly deferred)
- Full SMIL timing model (syncbase, indefinite end, restart modes, etc.).
- Full transform types and composition rules.
- Full animateColor interpolation.
- animateMotion playback and path following.

---

## 1. High-level Architecture

### 1.1 Parser (existing + extensions)
The parser converts XML attributes into internal attribute representations:
- `psx_svg_attr` stores `attr_id`, `val_type`, and a union `value`.
- `psx_svg_attr_values_list` is a variable-sized blob: `{ length; data[1]; }`.

Key internal list layouts (must match parser logic)
- Numeric animation values (`<animate>`, `<set>`): list payload is `float[length]`.
- `animateColor` values: list payload is `uint32_t[length]` (packed RGB).
- `animateTransform` values (from `ext/svg/psx_svg_parser.cpp`): each entry is
  - `_transform_values_list { uint32_t length; float data[4]; }`
  - stored in a `psx_svg_attr_values_list` blob; the top-level `length` is the number of entries.
- `begin` / `end` timing lists are parsed into a dedicated timing-list struct (stored opaquely via pointer type), with:
  - `offsets_ms[]`
  - optionally an `event_token` (for `begin="click"`-style tokens)

### 1.2 Player
The player walks the DOM, collects animation nodes, and evaluates them at a document time.

Primary file:
- `ext/svg/psx_svg_player.cpp`

Key concepts
- **Collection phase**: `_collect_anims()` traverses the DOM and stores lightweight `psx_svg_anim_item` entries.
- **Evaluation phase**: `seek()` / `tick()` / `trigger()` rebuild an **override state** for the current time.
- **Rendering hook**: `psx_svg_player_draw()` passes the override state to `psx_svg_render_list_draw_anim()`.

### 1.3 Override state (anim_state)
`psx_svg_anim_state` holds evaluated overrides ("current animated values"):
- `overrides`: numeric (and packed color bits stored in float) overrides.
- `transforms`: transform overrides per target node as a 2D matrix `a,b,c,d,e,f`.

Reasoning
- Rebuilding override tables on `seek()` / `tick()` / `trigger()` makes unit tests deterministic (no need to tick forward to observe state).
- Transform overrides are stored separately from float overrides to avoid type-punning and mixed semantics.

### 1.4 Debug/testing API
Unit tests query the player’s evaluated state:
- `psx_svg_player_debug_get_float_override(...)`
- `psx_svg_player_debug_get_transform_override(..., a,b,c,d,e,f)`

These are treated as **test hooks**; production rendering can later read the same data.

---

## 2. Implementation Strategy (TDD / incremental steps)

We deliberately progress in small, test-driven increments:

### Step-1: Timing lists + minimal event trigger
- Implement `begin` / `end` timing lists, per-animation instance activation.
- Add minimal external event trigger support for tokens like `begin="click"`.

### Step-2: values/keyTimes + calcMode
- `values` list evaluation.
- `keyTimes` handling (if absent, evenly spaced).
- `calcMode` support: `linear`, `discrete`, `spline` (cubic-bezier), `paced`.

### Step-2.5: animateColor(fill) minimal
- Minimal player support for `animateColor` on `fill`.
- For now: discrete behavior with exact packed RGB bit preservation.

### Step-3: animateTransform minimal
- Minimal player support for `animateTransform`:
  - `type="translate"`
   - `calcMode`:
      - default `linear`
      - `discrete`
- Add transform override table with `a,b,c,d,e,f`.

---

## 3. Current Progress (as of 2026-02-28)

### 3.1 Completed

#### Timing / begin-end / repeat / fill
- `begin` list semantics: supports multiple begins; for a given doc time chooses the latest begin `<= t` as the current activation.
- `end` list semantics: chooses the earliest end `>= begin` for each activation.
- `repeatCount` (including fractional -> effectively ceiled) and `repeatDur`.
- `fill=freeze|remove` semantics (including end instant handling).
- Event triggers: `begin="click"` supported via `psx_svg_player_trigger()`.

#### values / keyTimes / calcMode
- `values` overrides from/to.
- KeyTimes: if provided, determines segment mapping.
- calcMode:
  - `linear` (default)
  - `discrete`
  - `spline` via cubic-bezier helpers
  - `paced` via segment-length weighting and half-open interval handling

#### animateColor (fill) minimal playback
- `animateColor attributeName="fill"` supported minimally (discrete).
- Packed RGB (no alpha) is stored in float bits in the numeric override table.

#### animateTransform (translate, discrete) minimal playback
- Stores per-target transform overrides as `a,b,c,d,e,f`.
- Evaluated in:
  - `psx_svg_player_tick()`
  - `psx_svg_player_seek()`
  - `psx_svg_player_trigger()` (added to match seek/tick behavior)

#### animateTransform (translate, linear) minimal playback
- Default `calcMode` is treated as `linear` (interpolates `tx/ty` across `dur`).
- `calcMode="discrete"` remains supported.
- `keyTimes` is supported for `values` segmentation (non-uniform timing).
- `from`/`to` fallback is supported when `values` is absent.
- `repeatCount` / `repeatDur` and `fill=freeze|remove` are supported for `translate` (time clamping/looping aligned with `_anim_eval_simple()`).
- `repeatCount` / `repeatDur` and `fill=freeze|remove` are supported for `translate` (time clamping/looping aligned with `_anim_eval_simple()`).

#### animateTransform (scale/rotate, discrete) minimal playback
- `type="scale"` in `calcMode="discrete"` (matrix: `a=sx`, `d=sy`, `e=f=0`).
- `type="rotate"` in `calcMode="discrete"` (currently only rotate around origin; no `cx/cy`).

#### animateTransform (rotate, linear) minimal playback
- `type="rotate"` supports default `linear` interpolation of the angle in degrees.
- `values` + optional `keyTimes` segment mapping is supported.
- Time handling matches numeric/translate behavior: `repeatCount` / `repeatDur` and `fill=freeze|remove`.
- Current limitation: rotation is about origin only (no `cx/cy`).

### 3.2 Test coverage status
- Full test suite passes: **668/668**.

Key unit tests:
- `SVGPlayerTest.AnimateColorFill_FromTo_Discrete`
- `SVGPlayerTest.AnimateTransform_Translate_Discrete`
- `SVGPlayerTest.AnimateTransform_Translate_FreezeHoldsAfterDur`
- `SVGPlayerTest.AnimateTransform_Translate_RepeatCount`
- `SVGPlayerTest.AnimateTransform_Scale_Discrete`
- `SVGPlayerTest.AnimateTransform_Rotate_Discrete`
- `SVGPlayerTest.AnimateTransform_Rotate_Linear`

Notes:
- Some existing tests print sanitizer warnings (signed overflow in rasterizer) but tests still pass; not part of animation work.

---

## 4. Key Data Structures / Functions (reference)

### 4.1 `psx_svg_anim_item` (player-collected)
Stored per animation element:
- `tag`, `anim_node`, `target_node`, `target_attr`
- timing: `begin_sec`, `begins_sec[]`, `end_sec`, `ends_sec[]`, `dur_sec`
- repetition: `repeat_count`, `repeat_dur_sec`
- `fill_mode`
- `begin_event` token for external trigger support

### 4.2 Evaluation functions
- `_anim_eval_simple(...)`: numeric animate / animateColor
- `_anim_eval_set(...)`: `<set>` semantics
- `_anim_eval_transform_translate_discrete(...)`: minimal translate/discrete

### 4.3 Parser layout dependency (important)
`animateTransform values` decoding must match parser’s `_transform_values_list`:
- Each value entry contains `length` and up to 4 floats.
- Player uses a local mirror struct to interpret the blob.

---

## 5. Known Limitations / Technical Debt

1. **animateTransform coverage is still partial**
   - Supported: `translate` (linear/discrete), `scale` (discrete), `rotate` (discrete + linear).
   - Missing: `skewX`, `skewY`, `matrix`, and transform composition rules.
   - No `additive` / `accumulate`.

2. **animateTransform repeat/fill coverage is still partial**
   - `repeatCount` / `repeatDur` and `fill=freeze|remove` are implemented for `type="translate"` and `type="rotate"` only.
   - Other transform types will need the same time handling when added.

4. **Renderer integration**
   - Player passes `anim_state` to renderer; numeric override lookup exists.
   - Transform override consumption by renderer may still be incomplete (tests validate via debug getter).

5. **Event triggers**
   - Implemented for `begin_event` tokens, but only minimal matching (token equality) and optional filtering by target id.

---

## 6. Next Plan (ordered)

### Step-3 continuation (animateTransform)
1. **Support more transform types (one-by-one, TDD)**
   - `scale` (discrete first)
   - `rotate` (discrete first; then linear)
   - `skewX`, `skewY` (discrete first)
   - Potentially `type="matrix"` (if parser supports)

2. **Renderer application of transform overrides**
   - If not already implemented: apply override matrix during render list draw.
   - Add a rendering-level test only if deterministic and cheap.

### Step-4 (placeholder)
- Decide next SVG Tiny 1.2 feature area after transforms stabilize (candidate: animateMotion minimal playback or better event timing).

### Step-5 (placeholder)
- Hardening: fuzzing-ish inputs, invalid attribute behavior, performance checks, sanitizers.

---

## 7. Working Conventions
- Always land features in small increments.
- For each increment:
  1. Add failing test.
  2. Implement minimal code to pass.
  3. Run focused test and full `./unit_tests`.
  4. Provide a commitlint-style commit message.

---

## 8. Change Log (high level)
- Step-1: timing lists + event triggers.
- Step-2: values/keyTimes + calcMode.
- Step-2.5: animateColor(fill) minimal discrete.
- Step-3: animateTransform translate discrete + matrix override table; trigger path rebuild aligned with seek/tick.
