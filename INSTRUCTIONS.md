# RuleC Compiler — User Instructions

## What is RuleC?

RuleC is a domain-specific language (DSL) for expressing IoT sensor trigger rules. You write simple, human-readable rule files (`.rc`), and the compiler translates them into efficient C99 code that can run on microcontrollers or any C-capable platform.

---

## Requirements

| Tool | Minimum Version | Notes |
|------|----------------|-------|
| Python | 3.6+ | No external packages needed |
| GCC | Any C99-capable version | Clang also works |
| make | Any | Optional — all commands can be run manually |

---

## Writing a RuleC Program

### 1. Declare your sensors

```rc
SENSOR temp: float;
SENSOR humidity: float;
SENSOR motion: int;
```

Supported types: `float`, `int`, `bool`

### 2. Write rules

```rc
RULE overheat_alert:
  IF temp > 30.0 AND humidity < 20.0
  THEN alert("High temp, low humidity");
```

### 3. Available operators

| Category | Operators |
|----------|-----------|
| Comparison | `>`, `<`, `>=`, `<=`, `==`, `!=` |
| Logical | `AND`, `OR`, `NOT` |
| Grouping | `( )` |

### 4. Available actions

| Action | Arguments | Example |
|--------|-----------|---------|
| `alert` | 1 (message string) | `alert("Fire!")` |
| `log` | 1 (message string) | `log("Normal range")` |
| `actuate` | 2 (device, state) | `actuate(fan, ON)` |

### 5. Comments

```rc
// This is a single-line comment
SENSOR co2: float;  // inline comments are also fine
```

---

## Running the Compiler

### Compile a `.rc` file to C

```bash
python3 compiler.py <input.rc> -o <output_directory>
```

**Example:**
```bash
python3 compiler.py overheat.rc -o .
```

This produces:
- `evaluate_rules.h` — the header with the `SensorState` struct and function prototype
- `evaluate_rules.c` — the implementation of `evaluate_rules()`

The compiler prints each phase as it runs:
```
[1/4] Lexing overheat.rc ...
      -> 71 tokens
[2/4] Parsing ...
      -> 3 sensor declarations, 3 rules
[3/4] Semantic analysis ...
      -> symbol table: {'temp': 'float', 'humidity': 'float', 'motion': 'int'}
      -> no errors
[4/4] Code generation ...
      -> wrote ./evaluate_rules.h
      -> wrote ./evaluate_rules.c

Compilation successful.
```

---

## Building and Running the Demo

### Using Make (recommended)

```bash
make        # compile overheat.rc → C, then build rulec_demo binary
make test   # run the complete test suite
make clean  # remove generated files
```

### Manual build

```bash
# Step 1: Compile the RuleC source
python3 compiler.py overheat.rc -o .

# Step 2: Build the demo binary
gcc -Wall -Wextra -std=c99 -o rulec_demo test_harness.c evaluate_rules.c

# Step 3: Run it
./rulec_demo
```

---

## Integrating the Generated Code into Your Own Application

1. **Compile** your `.rc` file to get `evaluate_rules.c` and `evaluate_rules.h`.

2. **Implement the three action callbacks** in your application code:
   ```c
   #include "evaluate_rules.h"

   void action_alert(const char *message) {
       // e.g., sound a buzzer or send a network alert
   }

   void action_log(const char *message) {
       // e.g., write to SD card or UART
   }

   void action_actuate(const char *device, const char *state) {
       // e.g., toggle a GPIO pin
   }
   ```

3. **Call `evaluate_rules()`** in your sensor-reading loop:
   ```c
   SensorState readings = {
       .temp     = read_temperature(),
       .humidity = read_humidity(),
       .motion   = read_motion_sensor(),
   };
   evaluate_rules(&readings);
   ```

4. **Compile everything together:**
   ```bash
   gcc -Wall -std=c99 -o my_app my_app.c evaluate_rules.c
   ```

---

## Understanding Semantic Errors

The compiler detects and reports errors **before** generating any code.

| Error | Cause | Example |
|-------|-------|---------|
| `undeclared sensor 'X'` | Used in a rule but never declared | Using `pressure` without `SENSOR pressure: float;` |
| `sensor 'X' declared more than once` | Duplicate `SENSOR` declaration | Two `SENSOR temp: float;` lines |
| `duplicate rule name 'X'` | Two rules share a name | Two `RULE overheat_alert:` |
| `operator '>' requires a numeric sensor` | Relational op on `bool` | `status > 1` where `status` is `bool` |
| `contradictory condition on 'X'` | Rule can never fire | `temp > 30.0 AND temp < 10.0` |
| `unknown action 'X'` | Action not in the known set | `beep("alert")` |
| `wrong argument count for 'X'` | Wrong number of args | `alert()` or `alert("a", "b")` |

When an error occurs the compiler prints the error and exits with code `1`. No output files are written.

---

## Example: Full Session

```bash
$ python3 compiler.py overheat.rc -o .
[1/4] Lexing overheat.rc ...
      -> 71 tokens
[2/4] Parsing ...
      -> 3 sensor declarations, 3 rules
[3/4] Semantic analysis ...
      -> symbol table: {'temp': 'float', 'humidity': 'float', 'motion': 'int'}
      -> no errors
[4/4] Code generation ...
      -> wrote ./evaluate_rules.h
      -> wrote ./evaluate_rules.c

Compilation successful.

$ gcc -Wall -std=c99 -o rulec_demo test_harness.c evaluate_rules.c

$ ./rulec_demo
=== RuleC Phase 1 Prototype — Simulation ===

Test 1: Hot & dry room (should trigger overheat_alert)
  Readings: temp=33.5 humidity=15.0 motion=0
  [ALERT]   High temp, low humidity

Test 2: Comfortable room (should trigger comfort_zone)
  Readings: temp=22.0 humidity=45.0 motion=0
  [LOG]     Temperature within comfort range

Test 3: Motion detected, safe temp (should trigger intruder_check)
  Readings: temp=25.0 humidity=50.0 motion=1
  [LOG]     Temperature within comfort range
  [ACTUATE] light -> ON

Test 4: Motion detected, but too hot (intruder_check should NOT fire)
  Readings: temp=45.0 humidity=50.0 motion=1

Test 5: Cold room (no rule should fire)
  Readings: temp=5.0 humidity=60.0 motion=0

=== Simulation complete: 5 test cases run ===
```

---

## Running the Test Cases

```bash
# TC-01 through TC-03 and TC-06: valid programs — should exit 0
python3 compiler.py tests/tc_01_overheat.rc      -o /tmp/out
python3 compiler.py tests/tc_02_comfort.rc        -o /tmp/out
python3 compiler.py tests/tc_03_intruder.rc       -o /tmp/out
python3 compiler.py tests/tc_06_multi_rule_or.rc  -o /tmp/out

# TC-04 and TC-05: error programs — should exit 1 with a clear COMPILE ERROR message
python3 compiler.py tests/tc_04_contradiction.rc      -o /tmp/out
python3 compiler.py tests/tc_05_undeclared_sensor.rc  -o /tmp/out
```

Or simply:
```bash
make test
```
