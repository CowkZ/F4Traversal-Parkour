# F4Traversal - Activity Log

This log tracks all actions, decisions, and current status of the project to ensure continuity across sessions.

## Current Status
- **Goal**: Evolve "debug base" into a parkour system for Fallout 4 (v1.11.221).
- **Last Action**: Implemented `ProjectWorldToScreen` logic flow in `LedgeDetector`.
- **Active Task**: Mapping View-Projection matrix offsets for accurate 2D marker placement.
- **Pending**: Real Havok Raycast implementation and climb mechanics.

## Completed Tasks
- [x] Analysis of current codebase (`src/main.cpp`, `src/hotkey.cpp`, `src/debug.cpp`).
- [x] Setup CommonLibF4 dependency and GitHub Actions workflow (`build.yml`).
- [x] Validated camera position and forward vector capture via HUD.
- [x] Implemented `LedgeDetector` class with the "3-Ray Method" infrastructure.
- [x] Integrated `LedgeDetector::Update()` into the F4SE messaging loop.
- [x] Resolved compilation errors related to include paths and logger headers.
- [x] Defined `ProjectWorldToScreen` method for 3D to 2D coordinate conversion.

## Blockers
- **Raycast Implementation**: Needs the correct `hknpWorld` function offset to avoid `ACCESS_VIOLATION`.
- **Projection Matrix**: Needs the specific memory offset for the camera's View-Projection matrix in v1.11.221.

## Next Steps
1. Map the View-Projection matrix offset to enable real 2D marker rendering.
2. Map the correct Havok Raycast function for F4 v1.11.221.
3. Integrate the real physics results into the `LedgeDetector`.
4. Implement the "Climb" action trigger.

---
*Last Updated: 2026-09-21*
