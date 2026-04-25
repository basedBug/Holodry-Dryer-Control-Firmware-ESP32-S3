# ESP32 Modular Control System Architecture

## Overview

This project implements a modular control system on the ESP32 with strict separation between:

-   Sensor acquisition
-   Actuator control
-   System state and decision logic
-   External communication (JSON over Serial)

The system is designed for:

-   Deterministic behavior
-   Maintainability and scalability
-   Fault tolerance
-   Clear debugging and observability

---

## High-Level Design

The architecture follows a **centralized state management model**, where all control decisions are made exclusively by the **State Manager**.

```
          ┌────────────────────┐
          │ Communication Layer│
          └─────────┬──────────┘
                    │
                    ▼
          ┌────────────────────┐
          │    State Manager   │
          └─────────┬──────────┘
            ┌───────┴────────┐
            ▼                ▼
     ┌────────────┐   ┌────────────┐
     │   Sensors   │   │ Actuators  │
     └────────────┘   └────────────┘
```

---

## Core Principles

### 1. Single Source of Truth

All system state is owned and maintained by the **State Manager**.

### 2. Strict Control Flow

-   Sensors **never** control actuators directly
-   Communication **never** controls actuators directly
-   Only the State Manager issues actuator commands

### 3. Deterministic Execution

System behavior depends only on:

-   Latest sensor data
-   Current system state
-   Valid external commands

### 4. Separation of Responsibilities

Each module is isolated and interacts only through defined interfaces.

---

## System Guarantees

-   All actuator outputs originate from the State Manager
-   Communication layer cannot modify hardware state directly
-   Invalid or malformed input does not affect system stability
-   System continues operating under partial data failure (best effort)
-   State transitions are explicit and traceable

---

## Modules

### Sensor Manager

**Responsibility:**
Acquire and preprocess sensor data.

**Features:**

-   Periodic sampling
-   Filtering / calibration
-   Fault detection (timeouts, invalid readings)

**Interface:**

-   `init()`
-   `update()`
-   `getData() -> SensorData`

---

### Actuator Manager

**Responsibility:**
Control physical outputs safely.

**Features:**

-   GPIO / PWM control
-   Command validation
-   Safe fallback states

**Interface:**

-   `init()`
-   `applyCommands(ActuatorCommand)`
-   `getStatus() -> ActuatorState`

---

### State Manager

**Responsibility:**
Central system logic and state ownership.

**Features:**

-   Maintains full system state
-   Handles mode transitions (AUTO / MANUAL)
-   Merges sensor data and external commands
-   Produces actuator commands

**Interface:**

-   `updateSensors(SensorData)`
-   `updateCommands(CommandData)`
-   `compute()`
-   `getActuatorCommands()`
-   `getFullState()`

---

### Communication Manager

**Responsibility:**
Handle external communication via Serial using JSON.

**Features:**

-   Non-blocking serial reads
-   JSON parsing and validation
-   Outgoing state reporting
-   Error reporting

**Interface:**

-   `init(baudrate)`
-   `read() -> CommandData`
-   `send(SystemState)`
-   `sendError(ErrorMessage)`

---

## Data Flow

### Control Flow (Strict)

```
Sensors → State Manager → Actuators
Comm    → State Manager → Actuators
State   → Communication
```

### Telemetry (Read-Only)

```
Sensors   → Communication
Actuators → Communication
```

> ⚠️ Telemetry paths must never influence control decisions.

---

## System State Model

```cpp
struct SensorData {
    float temperature;
    float humidity;
    bool valid;
};

struct ActuatorState {
    bool fan;
    float valve;
    bool fault;
};

struct CommandData {
    String mode; // AUTO / MANUAL
    ActuatorState requestedActuators;
    bool valid;
};

struct SystemState {
    SensorData sensors;
    ActuatorState actuators;
    String mode;
    uint32_t timestamp;
};
```

---

## Timing Model

The system operates on fixed update intervals:

-   Sensor update rate: **10–100 ms**
-   State computation rate: **10–50 ms**
-   Communication update rate: **50–200 ms**

### Rules

-   State computation must always use the **latest available data**
-   Communication must not block control execution
-   Sensor delays must not stall the system

---

## Execution Model

### Option A: Single Loop (Simple)

```cpp
loop() {
    sensorManager.update();

    CommandData cmd = comm.read();

    stateManager.updateSensors(sensorManager.getData());
    stateManager.updateCommands(cmd);

    stateManager.compute();

    actuatorManager.applyCommands(
        stateManager.getActuatorCommands()
    );

    comm.send(stateManager.getFullState());
}
```

---

### Option B: FreeRTOS Tasks (Recommended)

#### Tasks

-   `SensorTask` (periodic)
-   `StateTask` (core logic)
-   `ActuatorTask` (output control)
-   `CommTask` (I/O handling)

#### Queue Ownership Rules

-   SensorTask → writes → SensorQueue
-   CommTask → writes → CommandQueue
-   StateTask → reads both queues
-   StateTask → writes → ActuatorQueue
-   ActuatorTask → consumes ActuatorQueue

#### Constraints

-   No shared mutable state without mutex
-   Prefer message passing over shared memory
-   All queues must be non-blocking or time-bounded

---

## JSON Protocol

### Incoming Command

```json
{
	"type": "command",
	"mode": "MANUAL",
	"actuators": {
		"fan": true,
		"valve": 0.5
	}
}
```

---

### Outgoing State

```json
{
	"type": "state",
	"timestamp": 12345678,
	"mode": "AUTO",
	"sensors": {
		"temperature": 24.5
	},
	"actuators": {
		"fan": true
	}
}
```

---

### Error Message

```json
{
	"type": "error",
	"message": "Invalid command format"
}
```

---

## Failure Handling

-   Invalid JSON → discarded + error reported
-   Sensor failure → flagged (`valid = false`)
-   Missing data → last known value or safe default
-   Actuator fault → reported in state
-   Communication timeout → system continues autonomously

---

## State Transitions

### Modes

-   `AUTO`: State Manager controls actuators based on sensors
-   `MANUAL`: External commands control actuators

### Rules

-   Mode changes only via valid commands
-   Transition must be explicit
-   On invalid command → ignore change

---

## Folder Structure

```
/src
  /sensors
  /actuators
  /state
  /communication
  main.cpp

/include
  types.h
```

---

## Design Constraints

-   No blocking I/O in control paths
-   No direct actuator access outside Actuator Manager
-   All commands validated before use
-   JSON protocol must be versionable

---

## Future Improvements

-   Message bus abstraction
-   Persistent configuration storage
-   OTA updates
-   Advanced diagnostics and logging
-   Protocol versioning

---

## Summary

This architecture provides:

-   Deterministic and safe control flow
-   Strong separation of concerns
-   Robust handling of invalid or missing data
-   Scalable structure for future expansion

The **State Manager acts as the central authority**, ensuring all system behavior is controlled, predictable, and traceable.
