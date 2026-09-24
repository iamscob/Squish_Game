## Jelly

A multiplayer party game built in Unreal Engine 5 with C++ and Blueprints.

Jelly is a fast-paced multiplayer party game for 4–6 players, where players control jelly characters and compete across short round-based matches.

One player becomes the Chaser and must catch another player before the round ends. The Chaser role can be transferred through player interaction, while pickups, throwable objects, stuns and knockback create chaotic situations during each round.

The project is primarily focused on multiplayer gameplay programming, Unreal Engine networking and gameplay system architecture.

Tech Stack
Unreal Engine 5
C++
Blueprints
Unreal Multiplayer / Replication
Key Systems
Round-based multiplayer game flow
Server-authoritative Chaser system
Chaser role selection and transfer
GameMode / GameState / PlayerState architecture
Replicated gameplay state
Server RPCs and RepNotify
Component-based combat and status systems
Pickups and throwable objects
Stun and knockback mechanics
Player-specific replicated visual feedback

Project Status:

In active development

The current goal is to build a polished multiplayer vertical slice demonstrating the complete gameplay loop and core networking systems and publish it on Steam :)


## Gameplay Architecture

* **GameMode** controls server-side match rules, including round flow and Chaser selection.
* **GameState** stores shared match information such as the current round, match phase and timers.
* **PlayerState** stores persistent player-specific state such as the Chaser role and penalty data.
* **Character** handles player-controlled gameplay such as movement, interactions and combat-related actions.
* **Actor Components** are used to separate systems such as status effects, combat and inventory/equipment.

