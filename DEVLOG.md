# StumbleGuysClone — Week 1 Devlog (Days 1–6)

**Date:** September 2026  
**Repo:** [github.com/m4sternoob/StumbleGuysClone](https://github.com/m4sternoob/StumbleGuysClone)  
**Engine:** Unreal Engine 5.6  
**Language:** C++ (no Blueprints for gameplay)

---

## TL;DR

Six days. Six commits. One playable prototype with replicated multiplayer, physics-based obstacles, and camera smoothing. All in C++. Portfolio deployed. Now building the actual game loop.

---

## Day 1 — Project Setup

**Goal:** Empty UE 5.6 project, Git repo, first commit.

- Created `StumbleGuysClone.uproject` targeting UE 5.6
- Configured `StumbleGuysClone.Build.cs` with all needed modules:
  `Core`, `CoreUObject`, `Engine`, `InputCore`, `EnhancedInput`, `PhysicsCore`, `NetCore`, `GameplayTags`, `GameplayTasks`, `NavigationSystem`, `AIModule`, `UMG`
- Set C++20 standard
- Pushed to GitHub: `m4sternoob/StumbleGuysClone`

**Lesson:** UE module setup is 90% of Day 1. Get it right once, never touch again.

---

## Day 2 — Character (`AStumbleCharacter`)

**Goal:** Capsule + sphere head, WASD + jump, replicated movement.

**Architecture:**
- Inherits `ACharacter` — leverages `CharacterMovementComponent` for built-in network prediction
- Enhanced Input system: `IA_Move` (Axis2D), `IA_Jump` (Digital Bool)
- Camera-relative movement (uses follow camera yaw, not controller)
- Spring arm + camera with configurable lag

**Replication:**
- `bReplicates = true`, `SetReplicateMovement(true)`
- `NetUpdateFrequency = 60`, `MinNetUpdateFrequency = 30`
- Visual color replication via `FLinearColor PlayerColor` (4 distinct colors)

**Code highlights:**
```cpp
// Camera-relative movement
FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
FVector Right   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
AddMovementInput(Forward, Input.Y);
AddMovementInput(Right, Input.X);
```

**Lesson:** Don't fight `CharacterMovementComponent`. It handles prediction/correction better than custom code.

---

## Day 3 — Arena (`AStumbleArena`)

**Goal:** Floor, 4 walls, kill zone. Server-authoritative.

**Components:**
- Floor: scaled cube, `BlockAll` collision
- 4 Walls: invisible barriers, `BlockAll`
- Kill Zone: large box below arena, `OverlapAll` → triggers elimination

**Physics materials (Day 6):**
- Floor: Friction 0.4, Restitution 0.1
- Walls: Friction 0.3, Restitution 0.3 (fun bounces)

**Kill zone flow:**
```
Overlap → Cast to AStumbleCharacter → GameMode::EliminatePlayer() → Disable collision/input → Check alive count
```

**Lesson:** Keep arena static. `SetReplicateMovement(false)` saves bandwidth.

---

## Day 4 — Camera Smoothing

**Goal:** Smooth follow camera with lag compensation.

**Implementation in `AStumbleCharacter::Tick()`:**
```cpp
void UpdateCameraLag(float DeltaTime)
{
    SmoothCameraRotation(DeltaTime);   // RInterpTo
    UpdateCameraLagPosition(DeltaTime); // VInterpTo + max distance clamp
}
```

**Features:**
- Rotation lag: `RInterpTo` with `CameraLagSpeed * 0.5`
- Position lag: `VInterpTo` with `CameraLagSpeed`, clamped to `CameraLagMaxDistance`
- Only runs on locally controlled pawn
- Initialization guard prevents first-frame jitter

**Configurable via UPROPERTY:**
- `CameraLagSpeed` (default 10, clamped 1–20)
- `CameraLagMaxDistance` (default 200)

**Lesson:** Camera lag feels "right" when it's subtle. Too much = motion sickness. Too little = robotic.

---

## Day 5 — Obstacle Spawning System

**Goal:** Dynamic obstacles each round — moving platforms + spinners.

**`AStumbleGameMode::SpawnObstacles()`:**
- 7 spawn points (center + 6 in circle, radius 60% of safe arena area)
- Shuffled each round for variety
- **3–4 Moving Platforms:** random axis (X/Y), distance 800–1500, speed 300–500, ping-pong
- **2–3 Spinners:** rotation 60–150°/s, random direction, bar length 800–1400, thickness 60–100

**Obstacle classes:**
- `AStumbleObstacleMovingPlatform` — ping-pong or loop motion
- `AStumbleObstacleSpinner` — constant Z rotation

**Lifecycle:**
```
StartRound() → SpawnObstacles() → Track in SpawnedObstacles[] → EndRound() → Destroy all
```

**Replication:** Server spawns, replicates movement/rotation to clients.

**Lesson:** Spawn on server, track array, clean up in `EndRound()`. No memory leaks.

---

## Day 6 — Physics Materials (All C++)

**Goal:** "Chaotic-good" collisions without Blueprint assets.

| Actor | Friction | Restitution | Combine Mode | Feel |
|-------|----------|-------------|--------------|------|
| Character capsule | 0.1 | 0.6 | Avg/Min | Slides, bounces off everything |
| Arena floor | 0.4 | 0.1 | Average | Grounded, not slippery |
| Arena walls (4×) | 0.3 | 0.3 | Average | Fun wall bounces |
| Obstacles (base) | 0.2 | 0.8 | Max/Min | High knockback, low grip |

**All created in C++ constructors:**
```cpp
UPhysicalMaterial* PhysMat = CreateDefaultSubobject<UPhysicalMaterial>(TEXT("Name"));
PhysMat->Friction = 0.1f;
PhysMat->Restitution = 0.6f;
PhysMat->RestitutionCombineMode = EPhysicalMaterialCombineMode::Average;
Component->SetPhysMaterialOverride(PhysMat);
```

**No Blueprint assets. No content browser. Pure code.**

**Lesson:** Physics feel is 80% material params. Iterate in code, test in PIE, commit.

---

## What's Working (Playable Now)

| Feature | Status |
|---------|--------|
| Character movement + jump | ✅ |
| Camera smoothing | ✅ |
| Arena + kill zone | ✅ |
| Round timer (60s) | ✅ |
| Player spawn + color assignment | ✅ |
| Elimination + win detection | ✅ |
| Moving platforms (3–4/round) | ✅ |
| Spinners (2–3/round) | ✅ |
| Physics materials tuned | ✅ |
| LAN multiplayer (listen server) | ✅ |
| Obstacle spawn/cleanup per round | ✅ |

---

## What's Next (Week 2)

| Day | Task |
|-----|------|
| 8 | `AObstacleSpinner` polish (visuals, sound) |
| 9 | Kill zone + 3s respawn with invincibility frames |
| 10 | Round system — win screen, last-player-standing |
| 11 | Lobby UI — 4 character slots, circle spawn |
| 12 | Bot AI (wander + avoid) |
| 13 | Juice pass — particles, screen shake, hit feedback |
| 14 | **Vertical slice build** + 1-min gameplay video |

---

## Technical Debt / Known Issues

1. **No win screen UI** — `EndRound()` just logs. Need `StumbleWinWidget` integration.
2. **No respawn logic** — eliminated players stay dead until round end.
3. **No lobby UI flow** — `StumbleLobbyWidget` exists but not hooked to session buttons.
4. **Bot AI missing** — Day 12.
5. **No particles/sound** — Day 13 juice pass.
6. **No devlog screen recording** — need 30-sec capture for Day 7 deliverable.

---

## Portfolio

**Live:** [m4sternoob.github.io/portfolio](https://m4sternoob.github.io/portfolio)  
**Repo:** [github.com/m4sternoob/StumbleGuysClone](https://github.com/m4sternoob/StumbleGuysClone)

Built with vanilla HTML/CSS/JS — no framework bloat. Features project cards with detail modals, tech depth section, responsive design.

---

## Metrics

| Metric | Value |
|--------|-------|
| Lines of C++ | ~2,500 |
| Files | 24 |
| Commits | 7 (1/day) |
| Blueprint assets | 0 (gameplay) |
| Physics materials | 7 (all C++) |
| Replicated actors | Character, GameState, PlayerState, Obstacles |

---

## Reflection

Six days. Zero Blueprints for gameplay. Every system — movement, camera, obstacles, physics, spawning, replication — written in C++. That was the constraint, and it forced clean architecture.

The prototype is playable. Two players on LAN can run, jump, get knocked off platforms, fall into kill zones, and win rounds. Physics feel chaotic in the right way.

Week 2 is about turning "playable prototype" into "vertical slice." Win screen, respawn, lobby, bots, juice. Then ship Day 14.

**See you at Day 14.**

---

*Built for portfolio. Free to study, fork, and learn from.*