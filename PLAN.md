# 30-Day Plan — StumbleGuysClone

Solo build, ~2-4 hrs/day, daily commit. Targeting portfolio-quality single-player + LAN multiplayer demo.

## Daily non-negotiables

- **1 git commit** (even tiny)
- **1 LinkedIn action** (comment, connect, or follow)
- **15 min reading** the next section of UE 5.6 docs / C++ tutorial

## Week 1 — Foundations (Days 1-7)

| Day | Focus | Deliverable |
|---|---|---|
| 1 | Project setup, repo, `.gitignore`, first commit | empty project builds in UE |
| 2 | `AStumbleCharacter` class: capsule + sphere mesh, WASD + jump | character moves around empty level |
| 3 | `AStumbleArena` actor: flat box floor + 4 invisible walls | character stays on platform |
| 4 | Top-down camera that follows the player with smoothing | camera tracks player smoothly |
| 5 | `AObstacleMovingPlatform` — moves back and forth on a timer | player gets knocked off when standing on it; **+ apply to 5 jobs** |
| 6 | Physics materials: bouncy players, friction tuned for chaos | collisions feel chaotic-good |
| 7 | First devlog post (500 words + 30-sec screen recording); **+ deploy portfolio to GitHub Pages** | week 1 retro public |

## Week 2 — Core Mechanics (Days 8-14)

| Day | Focus | Deliverable |
|---|---|---|
| 8 | `AObstacleSpinner` — rotating bar | hitting it knocks player back |
| 9 | Kill zone below arena + 3s respawn | falling = eliminated + back in |
| 10 | Round system: 60s timer, last-player-standing wins, win screen | full round loop works |
| 11 | Lobby + 4 character slots + circle spawn | pre-round character pick |
| 12 | Dumb bot AI wander | non-human players move around |
| 13 | Particles on hit + eliminated; **+ apply to 5 more jobs** | juice pass |
| 14 | **Vertical slice build**, 1-min gameplay video, devlog #2 | portfolio centerpiece shipped |

## Week 3 — Multiplayer (Days 15-21)

| Day | Focus | Deliverable |
|---|---|---|
| 15 | Replicate character movement server→client | clients see other players move |
| 16 | Server-authoritative physics | cheat-resistant movement |
| 17 | Replicate obstacle positions | obstacles sync across clients |
| 18 | Replicate win condition + timer | clients see same round state |
| 19 | LAN session browser (host/join UI) | 2 PCs can find each other |
| 20 | 2 humans + 2 bots playtest, fix worst bugs | demoable to a friend |
| 21 | Devlog #3; **+ apply to 5 more jobs** | multiplayer milestone post |

## Week 4 — Polish & Ship (Days 22-30)

| Day | Focus | Deliverable |
|---|---|---|
| 22 | UI upgrade — fonts, colors, animations | menus feel modern |
| 23 | Sound pass — jump, hit, win, lose | audio mixed |
| 24 | Settings menu (volume, keybinds) | configurable |
| 25 | 60-sec trailer video, upload YouTube as unlisted | trailer live |
| 26 | Package build for Windows + macOS | installer ready |
| 27 | Final README + screenshots + how-to-play | docs clean |
| 28 | **Release v1.0** on GitHub; **+ apply to 10 more jobs** | public launch |
| 29 | DM 5 recruiters, ask 3 community members to playtest + quote | outreach blitz |
| 30 | Final retro + next-phase decision; **+ follow up on all applications** | wrap & decide |

## Risk mitigation

- **Multiplayer is the biggest risk.** If Day 20 multiplayer is still broken, ship single-player only with "LAN multiplayer coming soon" in the README. Recruiters won't dock points — single-player polished + networked code in repo looks great.
- **Bot AI can be dumb.** Wandering in random directions is fine for Day 12. Smart AI is not the point.
- **Juice > features.** Polish on existing mechanics beats new broken mechanics every time.

## After Day 30 (if I want to keep going)

- More maps (2-3)
- Cosmetics / character skins
- Online (Steam) multiplayer
- In-app leaderboard
- Port to mobile

But that's later. Day 30 first.
