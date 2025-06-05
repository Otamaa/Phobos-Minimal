# TeamRetaliate Enhanced Configuration

## Overview

The TeamRetaliate feature has been completely rewritten and enhanced with new configuration options for better control over AI team behavior when attacked.

## Basic Configuration

Add these settings to your `rulesmd.ini` file under the `[General]` section:

```ini
[General]
TeamRetaliate=yes                                    ; Enable/disable team retaliation (default: no)
TeamRetaliate.CheckWeaponCompatibility=yes          ; Check if units can engage target type (default: yes)
TeamRetaliate.InterruptCriticalMissions=no          ; Allow interrupting repair/construction missions (default: no)
TeamRetaliate.MaxRetaliationDistance=-1             ; Maximum retaliation distance in cells, -1 = unlimited (default: -1)
```

## Configuration Options Explained

### TeamRetaliate
- **Type**: Boolean (yes/no)
- **Default**: no
- **Description**: Master switch for team retaliation. When enabled, AI teams will automatically switch targets to attackers when certain conditions are met.

### TeamRetaliate.CheckWeaponCompatibility
- **Type**: Boolean (yes/no)
- **Default**: yes
- **Description**: When enabled, units will only retaliate if they have appropriate weapons to engage the target (e.g., anti-air weapons for aircraft, anti-ground for ground units).

### TeamRetaliate.InterruptCriticalMissions
- **Type**: Boolean (yes/no)
- **Default**: no
- **Description**: When enabled, units will interrupt critical missions (repair, construction, selling, entering) to retaliate. When disabled, units performing these missions will not be assigned to attack the attacker.

### TeamRetaliate.MaxRetaliationDistance
- **Type**: Integer
- **Default**: -1
- **Description**: Maximum distance in cells that a team will retaliate against attackers. Set to -1 for unlimited range. This prevents teams from chasing attackers across the entire map.

## Behavior Changes

### Original Issues Fixed
1. **Units not actually attacking**: Previously only the team's target was changed, but individual units didn't receive attack orders
2. **Null pointer crashes**: Added comprehensive validation to prevent crashes
3. **Ally targeting**: Fixed cases where teams would target allied units
4. **Aircraft handling**: Improved handling of aircraft and aircraft-like units
5. **Open-topped transport handling**: Properly handles units attacking from inside open-topped transports
6. **Distance calculation logic**: Improved team-based distance checking for consistency with AI systems
7. **Team leader detection**: Fixed team leader finding to use proper designated leader instead of first unit
8. **Naval unit targeting**: Added proper validation for naval units attacking land targets
9. **Cloaked unit exploitation**: Prevents teams from retaliating against cloaked units they shouldn't see
10. **Dead target assignment**: Added validation to prevent targeting dead or invalid units
11. **Temporal/teleport targeting**: Prevents targeting units that are being teleported or in temporal
12. **Thread safety**: Added validation for units that may become invalid during processing

### New Enhancements
1. **Smart weapon checking**: Units only retaliate if they can actually damage the target
2. **Distance limiting**: Prevents teams from being pulled away from their objectives by distant attackers
3. **Mission priority**: Configurable behavior for critical missions
4. **Performance optimization**: Faster processing with reduced memory allocations

## Example Configurations

### Conservative Setting (Default behavior, safe for most mods)
```ini
[General]
TeamRetaliate=yes
TeamRetaliate.CheckWeaponCompatibility=yes
TeamRetaliate.InterruptCriticalMissions=no
TeamRetaliate.MaxRetaliationDistance=15
```

### Aggressive Setting (Teams respond quickly to any threat)
```ini
[General]
TeamRetaliate=yes
TeamRetaliate.CheckWeaponCompatibility=no
TeamRetaliate.InterruptCriticalMissions=yes
TeamRetaliate.MaxRetaliationDistance=-1
```

### Defensive Setting (Teams only respond to nearby threats)
```ini
[General]
TeamRetaliate=yes
TeamRetaliate.CheckWeaponCompatibility=yes
TeamRetaliate.InterruptCriticalMissions=no
TeamRetaliate.MaxRetaliationDistance=8
```

## Technical Details

### When Retaliation Triggers
- Team has a current target that is either invalid, unarmed, or close to completion
- Attacker is not an aircraft (aircraft are handled separately)
- Attacker is not an ally
- Attacker is not in limbo (unless in open-topped transport)
- Team has at least one unit capable of retaliation

### Unit Selection for Retaliation
- Unit must be alive and have health > 0
- Unit must be armed
- Unit must not be an aircraft (handled separately)
- Optionally: Unit must have appropriate weapons for target type
- Optionally: Unit must not be performing critical missions
- Optionally: Unit must be within maximum retaliation distance

### Mission Assignment
1. Individual units receive attack orders for the attacker
2. Team's centralized target is updated
3. Units coordinate their attack through the team system

## Compatibility

This enhanced implementation is backward compatible with existing configurations. If you only set `TeamRetaliate=yes`, the behavior will be similar to the original with improved reliability and performance.

## Performance Impact

The enhanced implementation is optimized for performance with:
- Early exit conditions to minimize processing
- Cached data to avoid repeated calculations
- Efficient iteration patterns
- Reduced memory allocations

The performance impact should be minimal and actually improved compared to the original implementation due to optimizations. 