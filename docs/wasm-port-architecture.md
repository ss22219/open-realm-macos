# Browser/WASM Port Architecture

The native C build is the reference implementation. Browser builds must reuse
the same game, asset, client, and renderer code wherever possible; the
TypeScript project is not a second source of truth.

## Runtime boundary

```text
Browser shell (JS)
  -> platform adapter (WASM imports/exports)
  -> C client
  -> loopback transport
  -> C server and Warcraft III game module
  -> snapshots, layouts, and render commands
  -> C renderer on the browser graphics context
```

The adapter owns only browser-specific concerns:

- window size, frame scheduling, keyboard, mouse, and pointer lock;
- virtual files backed by preload/fetch/IndexedDB;
- browser audio and persistent settings;
- optional WebSocket transport.

The C side remains authoritative for MPQ/VFS, JASS, triggers, units, combat,
terrain, MDX animation, particles, UI layout, snapshots, and rendering policy.

## Required extraction

Introduce a narrow platform interface behind the existing subsystems:

| Area | Native implementation | Browser implementation |
| --- | --- | --- |
| filesystem | POSIX/Win32 directory and files | Emscripten FS + preload/IndexedDB |
| window/context | SDL2 + desktop OpenGL | SDL2 web backend + WebGL2 |
| input | SDL events | browser events forwarded to SDL/adapter |
| audio | SDL audio device | Web Audio callback |
| network | UDP/loopback | loopback first, WebSocket later |
| clock | native monotonic clock | browser monotonic clock |

The first WASM milestone should use a single local client/server process and
preloaded map data. Networking and persistence are separate milestones.

## Renderer choice

For visual parity, compile the existing C renderer and its Warcraft III hooks
to WebGL2 through Emscripten. Three.js may remain as a launcher, inspector, or
test harness, but should not render the authoritative game world.

If Three.js renders the world instead, C can still be the simulation authority,
but the result is a compatibility implementation rather than pixel-level
parity because terrain blending, MDX materials, animation, particles, fog,
shadows, and UI layout would be duplicated.

## Migration gates

1. Native build and all tests remain unchanged.
2. Platform interface is introduced with native implementations first.
3. A headless/native golden frame is captured for one fixed map state.
4. The same state runs in WASM with the same assets and produces a frame.
5. Image and state diffs are used to close renderer and simulation gaps.
6. Only after parity is demonstrated should the current TypeScript mock world be
   retired or reduced to an adapter.

The absence of Emscripten is currently the only toolchain blocker for the
browser target; the native C target already builds and runs on macOS. The
current save-game ABI also assumes 64-bit function/data pointers, so the
browser build must either use Emscripten Memory64 or first replace those
pointer-packed fields with explicit 64-bit handles. A WASM32 build must not
silently be treated as compatible.
