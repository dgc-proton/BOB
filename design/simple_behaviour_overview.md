# B.O.B. Behaviour Overview


```mermaid
stateDiagram-v2

    state "Startup Health Check" as health_check
    state "Check for Opponent" as check
    state "Turn Towards Opponent\n(sensor distance min.)" as attack
    state "Check for Lines" as line_detect
    state "Steer Away from Line(s)" as line_avoid
    state "Maximum Power to Motors!" as steer_power
    state "Low Power to Motors" as go_slow
    state "Turn in One Direction" as turn_to_find

    [*] --> health_check: Power On
    health_check --> line_detect: Button Pressed
    line_detect --> check: No Line Detected
    line_detect --> line_avoid: Line Detected
    line_avoid --> line_detect
    check --> attack: Opponent Detected
    attack --> steer_power
    steer_power --> line_detect
    check --> go_slow: No Opponent Detected
    go_slow --> turn_to_find
    turn_to_find --> line_detect
```

# Navigation

### Inputs

* line sensors (IR x 2)
* object sensors (ultrasonic x 4)


### Map

Not used in the simple behaviour model.

### Path

Not used in the simple behaviour model.
