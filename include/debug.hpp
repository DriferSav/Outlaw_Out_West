#pragma once

// ---------------------------------------------------------------------------
// Debug overlay — compiled only in GAME_DEBUG builds.
//
// Use the macros at call sites so no #ifdef is needed outside this header:
//
//   DEBUG_UPDATE();          // call once per frame in update path
//   DEBUG_DRAW_WORLD();      // call inside BeginMode2D / EndMode2D
//   DEBUG_DRAW_SCREEN();     // call after EndMode2D (screen-space overlay)
//
// All macros expand to nothing in release builds.
// ---------------------------------------------------------------------------

#ifdef GAME_DEBUG

void DebugUpdate();
void DebugDrawWorld();   // collider wireframes, camera target — in world space
void DebugDrawScreen();  // fps, player stats, state — in screen space

#define DEBUG_UPDATE()        DebugUpdate()
#define DEBUG_DRAW_WORLD()    DebugDrawWorld()
#define DEBUG_DRAW_SCREEN()   DebugDrawScreen()

#else

#define DEBUG_UPDATE()        ((void)0)
#define DEBUG_DRAW_WORLD()    ((void)0)
#define DEBUG_DRAW_SCREEN()   ((void)0)

#endif
