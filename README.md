# Maine Client

Native Windows Minecraft launcher and performance platform.

Initial architecture:
- Core: paths, instances, configuration and launch lifecycle.
- Minecraft: version manifest handling.
- Optimization: hardware-aware performance profiles.
- UI: bootstrap Windows interface.

Planned services:
- Mojang version metadata and downloads.
- SHA-1 verified assets and libraries.
- Java runtime manager.
- Microsoft authentication.
- Vanilla/Fabric/Forge/NeoForge.
- Modrinth integration.
- Maine Performance Engine with version-specific optimization adapters.

Performance target:
Maine Client will optimize frame time and CPU/GPU workload rather than promise an arbitrary FPS number. Extremely high FPS is possible only when the hardware and scene can physically produce that frame time.

Build requirements:
Windows 10/11 x64, CMake 3.25+, Visual Studio 2022.

Build:
cmake -S . -B build
cmake --build build --config Release
