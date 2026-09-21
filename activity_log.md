# F4Traversal - Activity Log

Continuity file for every AI/dev session. READ THIS FIRST, then `src/`. Update it at the end of each session.

## Current Status (2026-09-21)
- **Target**: Fallout 4 v1.11.221, F4SE plugin, CommonLibF4 pinned to `d890b843bc1fd697b790af0f62146a69b87fb213` (last commit whose `RUNTIME_LATEST` is 1.11.221; see `.github/workflows/build.yml`, field `ref:`). Do NOT bump it.
- **Build**: GitHub Actions only (windows-latest, xmake). Linux cannot build this. Local gcc/clang runs only fake-header sanity checks.
- **Verified in game (earlier)**: plugin loads, F8 shows position/yaw on the HUD and in `F4Traversal.log`.
- **Written but NOT yet tested in game**: ledge detector (raycast), F7 toggle, Space-to-climb.

## Session 2026-09-21 - review + fixes (this session)
Bugs found in the previous code and fixed:
1. `LedgeDetector::Update()` was called inside the F4SE message callback. That callback fires only on ~10 events (PostLoad, GameDataReady, ...), NOT per frame, and also ran before the player/camera exist. Removed. Ticks now come from the hotkey thread via `AddTask` (~20 Hz, ~60 Hz while climbing) with a `g_tickPending` guard, only when the game is ready.
2. `static REL::Relocation g_RaycastFunc{ REL::ID(103892) }` was an invented ID (calling it caused the earlier crash; the declaration was left behind, and it resolves at DLL load). Removed.
3. Ray "up" vector was `{0,1,0}`. In Fallout 4 Z is up. Fixed (and the algorithm changed, see below).
4. Forward vector was computed from the camera quaternion with the formula that rotates the +Z (up) axis; identity rotation gave "up". Replaced by the player yaw: `forward = (sin(yaw), cos(yaw), 0)` with `yaw = player->data.angle.z`. The convention is self-consistent in the offline tests but must be confirmed in game (see test protocol).
5. Climb interpolation lerped from the CURRENT position (wrong easing/overshoot) and used a fixed 0.016 dt. Now lerps from `m_startPos` with real elapsed time, rising vertically first (60%) then moving over the ledge, to avoid clipping the corner.
6. Missing guards: game-ready flag (PreLoadGame/PostLoadGame/NewGame), `player->loadedData`, `parentCell`, and menus (Main/Loading/Pause/Pipboy/Console).
7. Removed unused/wrong includes (`RE/H/HUDMarkerData.h` is for compass markers, not for a hand icon). Headers use the PCH (`RE/Fallout.h`); do not include `RE/...` headers in `.cpp`/`.h` files.
8. `.gitignore` added: `build/`, `.xmake/`, `F4SE/` (DLL/PDB) were committed by mistake. Run `git rm -r --cached build .xmake F4SE`.
9. HUD text no longer uses the emoji glyph (the game font cannot render it): `[MAO] Escalavel (media) - altura 92`.

## Ledge detection (3-ray method, `src/ledge_detector.cpp`)
Constants at the top of the file (game units; player is ~128 tall). All rays use collision layer `kCharController`.
1. Horizontal ray at knee height (40) from 22 units in front of the player, reach 75: needs a near-vertical hit (`|normal.z| <= 0.5`).
2. Horizontal ray at 230 above the feet: if it hits near the same wall, the wall is too tall.
3. Down ray 12 units beyond the wall, from 230 down to 40: top surface height. Accept 45..190, `normal.z >= 0.7`.
4. Up ray from the top (100 units): headroom.
Classification: <90 baixa, <150 media, else alta. State changes are debounced (3 equal ticks) before the HUD message/log.
Offline test (fake headers, 13 checks: wall/height/yaw rotation/too tall/step/headroom/climb path) passes; run `tools/offline_test/check.sh` after ANY change to `src/` (Linux, gcc). It does not replace the CI build. Known limits of this method: low-ceiling interiors (down ray starts inside the ceiling), single-column headroom check (ignores capsule width), NPC bodies count as walls.

## Verified CommonLibF4 API (pinned commit) - use these, do not guess
- Raycast: `RE::bhkPickData pd; pd.castQuery.m_filterData.m_collisionFilterInfo = <u32>; pd.SetStartEnd(from, to); cell->Pick(pd); pd.HasHit(); pd.GetHitFraction(); pd.result.normal`. `cell = player->parentCell`. Hit point = `from + (to-from)*fraction` (game units).
- Collision layer enum: `RE::COL_LAYER` (`kCharController`, `kLOS`, ...). `RE::CFilter` has the bit layout (layer = low 7 bits, system group = high 16 bits).
- Projection (solves the old "View-Projection matrix offset" blocker; no offset hunting needed): `RE::Main::WorldRootCamera()` -> `NiCamera*`; `camera->WorldPtToScreenPt3(point, x, y, z, tolerance)` (x,y normalized 0..1). `NiCamera` also exposes `worldToCam[4][4]` and `port`.
- Menus: `RE::UI::GetSingleton()->GetMenuOpen<RE::PauseMenu>()` etc.
- HUD text: `RE::SendHUDMessage::ShowHUDMessage(msg, "", false, false)`.
- Game thread: `F4SE::GetTaskInterface()->AddTask(std::function<void()>)`.

## RISK: Address Library IDs
Every CommonLibF4 wrapper resolves an Address Library ID on first use. A missing ID makes CommonLibF4 abort the game with "Invalid ID: <number>". Another F4 plugin reported the `bhkPickData` constructor ID (2230668) missing on NG 1.10.984 and 1.11.191; 1.11.221 is UNVERIFIED. That is why the detector starts OFF and is enabled with F7. IDs involved in the raycast: 2230668 (ctor), 2236622 (SetStartEnd), 2200263 (TESObjectCELL::Pick), 2277770 (HasHit), 2277771 (GetHitFraction). Planned check: parse `Data/F4SE/Plugins/version-1-11-221-0.bin` (Address Library) for these IDs.

## Test protocol (in game)
1. Load a save, stand in an open area. F8: HUD shows position/yaw + "detector desligado". Walk straight ahead and press F8 again: the position must move along `(sin(yaw), cos(yaw))`.
2. F7: "detector LIGADO". If the game closes with "Invalid ID: N", write N down (log + dialog) and stop: the raycast IDs are missing on 1.11.221.
3. Face a waist/chest-high wall or crate: expect `[MAO] Escalavel (...)` and a `borda ...` line in `F4Traversal.log` with height and top position. Space climbs (experimental, teleport-based).
4. Tune the constants at the top of `ledge_detector.cpp` using the logged heights.

## Blockers
- Raycast availability on 1.11.221 (see RISK). If IDs are missing: build a fallback (own initialization of `bhkPickData`, or another query path) only after checking the Address Library file.
- The hand ICON needs a Scaleform/SWF HUD widget (or another UI path); only the screen position is solved (projection API above).

## Next Steps
1. Commit, push, let CI build, run the test protocol, report `F4Traversal.log`.
2. Wire the projection into `ProjectWorldToScreen` (`Main::WorldRootCamera` + `WorldPtToScreenPt3`); log x,y and check them against the ledge on screen.
3. Decide the icon route (Scaleform widget vs. simple text) and implement it.
4. Replace the teleport-style climb with a smoother movement + an animation trigger; then move constants to an `.ini`.

## Rules for AI sessions
- Never invent Address Library IDs, offsets or includes. Verify names in `lib/commonlibf4/include` (pinned commit) before using them.
- CommonLibF4 headers live in single-letter folders: `<RE/P/PlayerCharacter.h>`. There is no `RE/F4/`.
- `<Windows.h>` stays isolated at the bottom of `hotkey.cpp`; every project include goes above it.
- Anything touching game state runs on the game thread (`AddTask`); the hotkey thread only reads keys.
- F9 is quickload in Fallout 4: never bind it. Detector hotkeys: F7 toggle, F8 debug, Space climb.

---
*Last Updated: 2026-09-21*

## Session 2026-09-21 - Projeção 2D
1. Implementada a projeção de coordenadas 3D para 2D usando `RE::Main::WorldRootCamera()` e `NiCamera::WorldPtToScreenPt3`.
2. Adicionada flag `m_projectionEnabled` (DESLIGADA por padrão) para evitar crashes com IDs não verificados.
3. Mapeada a tecla **F6** para alternar a projeção.
4. Adicionado logging de x, y, z e visibilidade do ponto projetado no log.
5. Os IDs de `WorldRootCamera` e `WorldPtToScreenPt3` ainda não foram verificados no binário da 1.11.221; a ativação via F6 serve como teste seguro.
