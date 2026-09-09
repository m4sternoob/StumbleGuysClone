# StumbleGuysClone

3D multiplayer party game inspired by Stumble Guys. Built solo in C++ with Unreal Engine 5.6.

**Status:** Day 1 of 30 — project skeleton + first commit. See [PLAN.md](./PLAN.md) for the full roadmap.

**Author:** m4sternoob · Ontario, Canada · targeting entry-level roles at Ubisoft Toronto / Rockstar Toronto / similar GTA studios.

---

## Why this project exists

- **Portfolio centerpiece** for game-developer job applications.
- Shows I can ship a working C++ game in Unreal Engine (not just Blueprints).
- Demonstrates daily discipline (public commit log), problem-solving (devlog), and polish (juice + UI).
- Goal: one playable vertical slice by day 14, full single-player + LAN multiplayer by day 30.

## Stack

| Layer | Tech |
|-------|------|
| Engine | Unreal Engine **5.6** |
| Language | **C++** (no Blueprints for gameplay — proves I understand the engine) |
| Physics | UE Physics (Chaos) |
| Networking | UE Replication |
| Version control | Git + GitHub |
| Target | PC (Windows + macOS), 4 players LAN |

## Quickstart

### 1. Install UE 5.6

- Get the Epic Games Launcher: https://www.epicgames.com/store/en-US/download
- Install **Unreal Engine 5.6** (free).

### 2. Clone this repo

```bash
git clone https://github.com/m4sternoob/StumbleGuysClone.git
cd StumbleGuysClone
```

### 3. Generate IDE project files

#### macOS
```bash
/Users/gillz/Applications/Epic\ Games/UE_5.6/Engine/Binaries/Mac/UnrealBuildTool \
    -projectfiles \
    -project="$(pwd)/StumbleGuysClone.uproject" \
    -game -engine -progress
open StumbleGuysClone.xcworkspace
```

#### Windows
- Right-click `StumbleGuysClone.uproject` → **Generate Visual Studio project files**
- Open the generated `.sln` in Visual Studio 2022
- Set `StumbleGuysCloneEditor` as the startup project
- Build → Run

### 4. Open in UE 5.6

- Launch UE 5.6 Editor
- File → Open Project → pick `StumbleGuysClone.uproject`
- Compile C++ when prompted
- Hit **Play** — you should see the default empty level (we don't have a custom map yet on Day 1)

## Project structure

```
StumbleGuysClone/
├── StumbleGuysClone.uproject   # UE project descriptor
├── Config/                     # Engine + Game config
│   ├── DefaultEngine.ini
│   └── DefaultGame.ini
├── Source/
│   └── StumbleGuysClone/       # C++ module (game code)
│       ├── StumbleGuysClone.Build.cs    # module build rules
│       ├── StumbleGuysClone.cpp         # module entry point
│       ├── StumbleGuysClone.h
│       ├── StumbleGameModeBase.cpp      # base game mode (filled Day 10+)
│       └── StumbleGameModeBase.h
├── Content/                    # (empty — assets added from Day 2)
├── PLAN.md                     # day-by-day 30-day roadmap
├── README.md                   # this file
└── .gitignore
```

## Target studios (Ontario)

- **Ubisoft Toronto** — Assassin's Creed, Far Cry 6, Splinter Cell
- **Rockstar Toronto** — GTA support, porting
- **Digital Extremes** — Warframe (London ON)
- **Certain Affinity** — Halo, CoD support
- **Torn Banner Studios** — Chivalry 2
- **Capybara Games** — Below, Cuphead support
- **DrinkBox Studios** — Nobody Saves the World, Guacamelee

## How to follow progress

- **Commit log** — every day, even tiny commits
- **Devlog** — weekly retro posts in `/devlog/` (Week 1 done by Day 7)
- **Releases** — v0.1 vertical slice at Day 14, v1.0 at Day 30
- **Builds** — packaged Windows + macOS binaries in GitHub Releases

## License

Built for portfolio. Free to study, fork, and learn from. Don't redistribute commercially.

## Contact

- GitHub: [@m4sternoob](https://github.com/m4sternoob)
- LinkedIn: (link TBD)
- Email: (link TBD)
