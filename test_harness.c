// test_harness.c — Phase 1 demo driver
// Feeds mock sensor readings through the compiler-generated evaluate_rules()
// and prints which rules fire, proving the generated C code behaves correctly.

#include <stdio.h>
#include "evaluate_rules.h"

// ---- Action callback implementations (in a real deployment these would
//      drive a buzzer, write to a log file, or toggle a GPIO pin) ----

void action_alert(const char *message) {
    printf("  [ALERT]   %s\n", message);
}

void action_log(const char *message) {
    printf("  [LOG]     %s\n", message);
}

void action_actuate(const char *device, const char *state) {
    printf("  [ACTUATE] %s -> %s\n", device, state);
}

typedef struct {
    const char *label;
    SensorState state;
} TestCase;

int main(void) {
    TestCase cases[] = {
        { "Hot & dry room (should trigger overheat_alert)",
          { .temp = 33.5f, .humidity = 15.0f, .motion = 0 } },
        { "Comfortable room (should trigger comfort_zone)",
          { .temp = 22.0f, .humidity = 45.0f, .motion = 0 } },
        { "Motion detected, safe temp (should trigger intruder_check)",
          { .temp = 25.0f, .humidity = 50.0f, .motion = 1 } },
        { "Motion detected, but too hot (intruder_check should NOT fire)",
          { .temp = 45.0f, .humidity = 50.0f, .motion = 1 } },
        { "Cold room (no rule should fire)",
          { .temp = 5.0f, .humidity = 60.0f, .motion = 0 } },
    };
    int n = sizeof(cases) / sizeof(cases[0]);

    printf("=== RuleC Phase 1 Prototype — Simulation ===\n\n");
    for (int i = 0; i < n; i++) {
        printf("Test %d: %s\n", i + 1, cases[i].label);
        printf("  Readings: temp=%.1f humidity=%.1f motion=%d\n",
               cases[i].state.temp, cases[i].state.humidity, cases[i].state.motion);
        evaluate_rules(&cases[i].state);
        printf("\n");
    }
    printf("=== Simulation complete: %d test cases run ===\n", n);
    return 0;
}
