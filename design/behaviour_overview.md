# B.O.B. Behaviour Overview

```mermaid
stateDiagram-v2

    state "Startup Health Check" as health_check
    note left of line_detect: An alternative would be to\nuse an Interupt Service\nRoutine for line detection
    state "Check for Opponent" as check
    state "Update Map & Path:\nDrive at Opponent /\nSearch Pattern" as attack
    state "Check for Lines" as line_detect
    state "Update Map & Path:\nAvoid Line (top priority)" as line_avoid
    state "Update Steering &\nAdjust Motor Power" as steering
    state "Update Steering &\nAdjust Motor Power" as steer_power

    [*] --> health_check: Power On
    health_check --> line_detect: Button Pressed
    line_detect --> check: No Line
    line_detect --> line_avoid: Line Detected
    line_avoid --> steering
    steering --> check
    check --> attack
    attack --> steer_power
    steer_power --> line_detect
```

# Navigation

### Inputs

* wheel encoders using ISR
* line sensors (IR x 4)
* object sensors (ultrasonic x ?)

### Map

Array that tracks the location of the bot, opponent (or last known location of opponent) and lines. As time passes localization gets less accurate, so should we remove or de-value data as it gets older? The location of the bot can be tracked by array indices giving the cell that the centre of the robot is in, and a vector for the direction it is facing.

### Path

Algorithm that decides which way the bot should be facing and the speed that it should be going at, based on avoiding lines (highest priority) and finding or attacking the opponent.
