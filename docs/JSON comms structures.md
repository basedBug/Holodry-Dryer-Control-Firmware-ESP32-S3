# System Data JSON Structures

## Enum Definitions

### SystemStatus Enum

| Value             | Description                 |
| ----------------- | --------------------------- |
| IDLE              | System is idle              |
| TUNING            | PID tuning mode             |
| DRYING            | Active drying cycle         |
| VENTING           | Venting cycle               |
| EMERGENCY_VENTING | Emergency venting activated |
| MANUAL_MODE       | Manual actuator control     |
| SHOWCASE_MODE     | Showcase/demo mode          |
| UNKNOWN           | Invalid/unknown state       |

### FanStatus Enum

| Value   | Description           |
| ------- | --------------------- |
| OFF     | Fan is deactivated    |
| ON      | Fan is activated      |
| UNKNOWN | Invalid/unknown state |

### VentsStatus Enum

| Value   | Description           |
| ------- | --------------------- |
| CLOSED  | Vents are closed      |
| OPEN    | Vents are open        |
| UNKNOWN | Invalid/unknown state |

## Outgoing JSON Structure (Device → Client)

```json
{
	"systemState": {
		"status": {
			"mainStatus": "IDLE|TUNING|DRYING|VENTING|EMERGENCY_VENTING|MANUAL_MODE|SHOWCASE_MODE",
			"exteriorAbsHum": "number (float)",
			"interiorAbsHum": "number (float)",
			"heaterSetpoint": "number (float)",
			"fan": "OFF|ON",
			"vents": "CLOSED|OPEN"
		},
		"systemConfig": {
			"targetChamberTemp": "number (float)",
			"maxAllowedFilamentTemp": "number (float)",
			"targetRelHum": "number (float)"
		},
		"chamberState": {
			"chamberTemp": {
				"max": "number (float)",
				"avg": "number (float)"
			},
			"chamberHum": {
				"max": "number (float)",
				"avg": "number (float)"
			}
		},
		"ambientState": {
			"temp": "number (float)",
			"hum": "number (float)"
		},
		"filamentState": {
			"filamentTemp": {
				"max": "number (float)",
				"avg": "number (float)"
			}
		}
	},
	"rawSensorTelemetry": {
		"chamber": {
			"sht31_0_temp": "number (float)",
			"sht31_0_hum": "number (float)",
			"sht31_1_temp": "number (float)",
			"sht31_1_hum": "number (float)",
			"sht31_2_temp": "number (float)",
			"sht31_2_hum": "number (float)",
			"ds18b20_temp": "number (float)"
		},
		"ambient": {
			"sht31_3_temp": "number (float)",
			"sht31_3_hum": "number (float)"
		},
		"filament": {
			"mlx90614_0_temp": "number (float)",
			"mlx90614_1_temp": "number (float)"
		}
	}
}
```

## Incoming JSON Structure (Client → Device)

```json
{
	"systemControl": {
		"systemStatus": "IDLE|TUNING|DRYING|VENTING|EMERGENCY_VENTING|MANUAL_MODE|SHOWCASE_MODE",
		"targetChamberTemp": "number (float, >= 0)",
		"maxAllowedFilamentTemp": "number (float, >= 0)",
		"targetRelHum": "number (float, >= 0)",
		"actuators": {
			"heaterSetpointTemp": "number (float, >= 0)",
			"fan": "OFF|ON",
			"vents": "CLOSED|OPEN"
		}
	},
	"showcaseControl": {
		"showcaseMode": "boolean",
		"simulatedSensors": {
			"chamberSensorTemp_0": "number (float)",
			"chamberSensorTemp_1": "number (float)",
			"chamberSensorTemp_2": "number (float)",
			"chamberSensorHum_0": "number (float)",
			"chamberSensorHum_1": "number (float)",
			"chamberSensorHum_2": "number (float)",
			"ambientSensorTemp": "number (float)",
			"ambientSensorHum": "number (float)",
			"filamentSurfaceTemp_0": "number (float)",
			"filamentSurfaceTemp_1": "number (float)",
			"heaterSensorTemp": "number (float)"
		}
	}
}
```

## Example Outgoing JSON

```json
{
	"systemState": {
		"status": {
			"mainStatus": "DRYING",
			"exteriorAbsHum": 12.5,
			"interiorAbsHum": 8.3,
			"heaterSetpoint": 45.0,
			"fan": "ON",
			"vents": "CLOSED"
		},
		"systemConfig": {
			"targetChamberTemp": 45.0,
			"maxAllowedFilamentTemp": 60.0,
			"targetRelHum": 30.0
		},
		"chamberState": {
			"chamberTemp": {
				"max": 44.8,
				"avg": 43.2
			},
			"chamberHum": {
				"max": 32.5,
				"avg": 29.8
			}
		},
		"ambientState": {
			"temp": 22.5,
			"hum": 55.0
		},
		"filamentState": {
			"filamentTemp": {
				"max": 58.3,
				"avg": 56.7
			}
		}
	},
	"rawSensorTelemetry": {
		"chamber": {
			"sht31_0_temp": 44.8,
			"sht31_0_hum": 32.5,
			"sht31_1_temp": 43.2,
			"sht31_1_hum": 29.8,
			"sht31_2_temp": 42.5,
			"sht31_2_hum": 28.0,
			"ds18b20_temp": 46.2
		},
		"ambient": {
			"sht31_3_temp": 22.5,
			"sht31_3_hum": 55.0
		},
		"filament": {
			"mlx90614_0_temp": 58.3,
			"mlx90614_1_temp": 56.7
		}
	}
}
```

## Example Incoming JSON

```json
{
	"systemControl": {
		"systemStatus": "DRYING",
		"targetChamberTemp": 45.5,
		"maxAllowedFilamentTemp": 60.0,
		"targetRelHum": 30.0,
		"actuators": {
			"heaterSetpointTemp": 50.0,
			"fan": "ON",
			"vents": "OPEN"
		}
	},
	"showcaseControl": {
		"showcaseMode": true,
		"simulatedSensors": {
			"chamberSensorTemp_0": 44.8,
			"chamberSensorTemp_1": 43.2,
			"chamberSensorTemp_2": 42.5,
			"chamberSensorHum_0": 32.5,
			"chamberSensorHum_1": 29.8,
			"chamberSensorHum_2": 28.0,
			"ambientSensorTemp": 22.5,
			"ambientSensorHum": 55.0,
			"filamentSurfaceTemp_0": 58.3,
			"filamentSurfaceTemp_1": 56.7,
			"heaterSensorTemp": 46.2
		}
	}
}
```

## Field Descriptions

### systemState.status

| Field          | Type           | Valid Values                                                                 | Description                             |
| -------------- | -------------- | ---------------------------------------------------------------------------- | --------------------------------------- |
| mainStatus     | string         | IDLE, TUNING, DRYING, VENTING, EMERGENCY_VENTING, MANUAL_MODE, SHOWCASE_MODE | Current system operational status       |
| exteriorAbsHum | number (float) | Any float                                                                    | Exterior absolute humidity              |
| interiorAbsHum | number (float) | Any float                                                                    | Interior absolute humidity              |
| heaterSetpoint | number (float) | Any float                                                                    | Current heater PID setpoint temperature |
| fan            | string         | OFF, ON                                                                      | Fan status                              |
| vents          | string         | CLOSED, OPEN                                                                 | Vents status                            |

### systemState.systemConfig

| Field                  | Type           | Valid Values | Description                          |
| ---------------------- | -------------- | ------------ | ------------------------------------ |
| targetChamberTemp      | number (float) | Any float    | Target chamber temperature           |
| maxAllowedFilamentTemp | number (float) | Any float    | Maximum allowed filament temperature |
| targetRelHum           | number (float) | Any float    | Target relative humidity             |

### systemState.chamberState

| Field           | Type           | Valid Values | Description                         |
| --------------- | -------------- | ------------ | ----------------------------------- |
| chamberTemp.max | number (float) | Any float    | Maximum chamber temperature reading |
| chamberTemp.avg | number (float) | Any float    | Average chamber temperature reading |
| chamberHum.max  | number (float) | Any float    | Maximum chamber humidity reading    |
| chamberHum.avg  | number (float) | Any float    | Average chamber humidity reading    |

### systemState.ambientState

| Field | Type           | Valid Values | Description         |
| ----- | -------------- | ------------ | ------------------- |
| temp  | number (float) | Any float    | Ambient temperature |
| hum   | number (float) | Any float    | Ambient humidity    |

### systemState.filamentState

| Field            | Type           | Valid Values | Description                  |
| ---------------- | -------------- | ------------ | ---------------------------- |
| filamentTemp.max | number (float) | Any float    | Maximum filament temperature |
| filamentTemp.avg | number (float) | Any float    | Average filament temperature |

### rawSensorTelemetry.chamber

| Field        | Type           | Valid Values | Description                       |
| ------------ | -------------- | ------------ | --------------------------------- |
| sht31_0_temp | number (float) | Any float    | SHT31 sensor 0 temperature        |
| sht31_0_hum  | number (float) | Any float    | SHT31 sensor 0 humidity           |
| sht31_1_temp | number (float) | Any float    | SHT31 sensor 1 temperature        |
| sht31_1_hum  | number (float) | Any float    | SHT31 sensor 1 humidity           |
| sht31_2_temp | number (float) | Any float    | SHT31 sensor 2 temperature        |
| sht31_2_hum  | number (float) | Any float    | SHT31 sensor 2 humidity           |
| ds18b20_temp | number (float) | Any float    | DS18B20 heater temperature sensor |

### rawSensorTelemetry.ambient

| Field        | Type           | Valid Values | Description                |
| ------------ | -------------- | ------------ | -------------------------- |
| sht31_3_temp | number (float) | Any float    | Ambient temperature sensor |
| sht31_3_hum  | number (float) | Any float    | Ambient humidity sensor    |

### rawSensorTelemetry.filament

| Field           | Type           | Valid Values | Description                           |
| --------------- | -------------- | ------------ | ------------------------------------- |
| mlx90614_0_temp | number (float) | Any float    | Filament surface temperature sensor 0 |
| mlx90614_1_temp | number (float) | Any float    | Filament surface temperature sensor 1 |

### Incoming systemControl

| Field                  | Type           | Valid Values                                                                 | Description                              |
| ---------------------- | -------------- | ---------------------------------------------------------------------------- | ---------------------------------------- |
| systemStatus           | string         | IDLE, TUNING, DRYING, VENTING, EMERGENCY_VENTING, MANUAL_MODE, SHOWCASE_MODE | Command system operational status        |
| targetChamberTemp      | number (float) | >= 0                                                                         | Set target chamber temperature           |
| maxAllowedFilamentTemp | number (float) | >= 0                                                                         | Set maximum allowed filament temperature |
| targetRelHum           | number (float) | >= 0                                                                         | Set target relative humidity             |

### Incoming systemControl.actuators

| Field              | Type           | Valid Values | Description                        |
| ------------------ | -------------- | ------------ | ---------------------------------- |
| heaterSetpointTemp | number (float) | >= 0         | Direct heater temperature setpoint |
| fan                | string         | OFF, ON      | Direct fan control                 |
| vents              | string         | CLOSED, OPEN | Direct vents control               |

### Incoming showcaseControl

| Field        | Type    | Valid Values | Description                  |
| ------------ | ------- | ------------ | ---------------------------- |
| showcaseMode | boolean | true, false  | Enable/disable showcase mode |

### Incoming showcaseControl.simulatedSensors

| Field                 | Type           | Valid Values | Description                             |
| --------------------- | -------------- | ------------ | --------------------------------------- |
| chamberSensorTemp_0   | number (float) | Any float    | Simulated chamber temperature sensor 0  |
| chamberSensorTemp_1   | number (float) | Any float    | Simulated chamber temperature sensor 1  |
| chamberSensorTemp_2   | number (float) | Any float    | Simulated chamber temperature sensor 2  |
| chamberSensorHum_0    | number (float) | Any float    | Simulated chamber humidity sensor 0     |
| chamberSensorHum_1    | number (float) | Any float    | Simulated chamber humidity sensor 1     |
| chamberSensorHum_2    | number (float) | Any float    | Simulated chamber humidity sensor 2     |
| ambientSensorTemp     | number (float) | Any float    | Simulated ambient temperature sensor    |
| ambientSensorHum      | number (float) | Any float    | Simulated ambient humidity sensor       |
| filamentSurfaceTemp_0 | number (float) | Any float    | Simulated filament temperature sensor 0 |
| filamentSurfaceTemp_1 | number (float) | Any float    | Simulated filament temperature sensor 1 |
| heaterSensorTemp      | number (float) | Any float    | Simulated heater temperature sensor     |

## Validation Rules

### Incoming Data Validation

- **systemStatus**: Must match one of the defined SystemStatus enum values. `UNKNOWN` is rejected.
- **Fan control**: Must match `OFF` or `ON`. `UNKNOWN` is rejected.
- **Vents control**: Must match `CLOSED` or `OPEN`. `UNKNOWN` is rejected.
- **Temperature/Humidity values**: Must be >= 0 to be accepted. Negative values are ignored.
- **All fields**: Optional - only present fields are processed. Missing fields leave current state unchanged

### Mode Change Triggers

- Setting any actuator value (heaterSetpointTemp, fan, or vents) automatically switches system to `MANUAL_MODE`
- Setting simulatedSensors automatically enables `showcaseMode` (true) and sets status to `SHOWCASE_MODE`
- `showcaseMode` can also be set independently via the boolean field
- `showcaseMode` can be deactivated by the boolean field and by manually switching to another system mode

### Outgoing Data Notes

- All sensor data is sent in both processed (systemState) and raw (rawSensorTelemetry) formats
- System configuration is included in every state update
- Actuator status reflects current operational state
- Status strings are always valid enum values (never UNKNOWN)
