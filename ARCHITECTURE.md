# Maine Client Architecture

## Layers

Core owns filesystem layout, configuration, hardware detection and process lifecycle.

Minecraft owns version metadata, dependency resolution and launch command construction.

Optimization owns hardware-aware profiles and version-specific performance adapters.

UI is presentation only and must not contain launcher or optimization logic.

## Performance Engine

The performance engine is designed around frame-time reduction. It will measure CPU/GPU bottlenecks and apply only compatible optimizations.

Planned components:
- renderer path
- visibility and entity culling
- chunk scheduling
- worker-thread coordination
- allocation reduction
- Java/runtime tuning
- per-version adapters
- third-party optimization compatibility

## Compatibility

Minecraft versions must be isolated. A version-specific adapter may change implementation details without changing the launcher core.

No optimization is allowed to corrupt saves, break mod compatibility or alter game semantics merely to increase an FPS counter.
