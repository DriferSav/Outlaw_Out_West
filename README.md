# Outlaw Out West
A simple 2D platformer that uses raylib and nlohmann/json.
 
## Build Steps
 
```bash
git clone https://github.com/DriferSav/Outlaw_Out_West.git
cd Outlaw_Out_West
 
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

## Play Game

```bash
cd build
./Outlaw
```

## Controls

**Keyboard**
- A - go left
- D - go right
- SPACE - jump
- F - interact
- E - melee
- LEFT MOUSE CLICK - shoot
- R - reload
- SHIFT - dash

**Gamepad**
- LEFT STICK - move
- RIGHT STICK - aim gun
- LEFT BUMPER - dash
- RIGHT TRIGGER - shoot
- A - jump
- b - interact
- x - reload
- y - melee
---
