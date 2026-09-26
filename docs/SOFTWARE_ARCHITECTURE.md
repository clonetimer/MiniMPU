# Software Architecture

## Boundary
The GUI depends only on `Snapshot`. Backend-specific objects normalize their state before it reaches a page.

```
Tkinter Pages
     |
 Snapshot
  /      \
Architecture   RTL Replay
 C++ bridge     CSV evidence
```

## Why a subprocess for the C++ model
The C++ reference model remains a native component and is not rewritten in Python. A tiny line-oriented bridge keeps simulator lifetime/state in one process and returns JSON snapshots. This prevents the GUI toolkit from becoming part of the simulator API.

## Why replay instead of live Verilator in V0.1
Live Verilator adds compilation, tool discovery, long-running process control and platform-specific build concerns. V3.3 already supplies a measured 63,409-cycle execution with the required microarchitectural probes. V0.1 uses that evidence to validate the interaction model. A live RTL backend is deferred, not forgotten.

## Snapshot contract
Common fields include PC, privilege, cycle/instret, GPRs, CSR map, stopped/exit status and optional MMU/TLB/PTW observations. Architecture-only or RTL-only observations remain optional and are displayed as unavailable rather than fabricated.
