#include "commands.h"
#include "RobotState.h"
#include "mock_arduino.h"
#include <cstdarg>
#include <cstdio>

// ---- Stand-ins for the real globals (normally in your .ino / RobotBLE.cpp) ----
RobotState state = {};
calData imu_cal_data = {};

// Stand-in for the real notify() — instead of sending over BLE, just prints
// to the console so you can see exactly what the Android app would receive.
void notify(const char* fmt, ...) {
    char buffer[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    printf("%s", buffer);
}

// Mimics what MyCallbacks::onWrite() does: split "command arg1 arg2" into
// a command word and the remaining args, then route it exactly like a real
// BLE write would.
void simulate_ble_write(const char* raw_command) {
    char buffer[64];
    strncpy(buffer, raw_command, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char* save;
    char* command = strtok_r(buffer, " \r\n", &save);
    char* args = save;

    printf("\n> %s\n", raw_command);
    if (command == NULL) {
        printf("unknown command\n");
        return;
    }
    handle_command(command, args);
}

int main() {
    printf("=== Simulated Android BLE commands ===\n");

    simulate_ble_write("k p 1.5");        // set kp
    simulate_ble_write("k p");            // read kp back
    simulate_ble_write("speed forward 120");
    simulate_ble_write("log dist 200");   // toggle distance logging on, 200ms interval
    simulate_ble_write("reverse_drive");  // toggle
    simulate_ble_write("start");          // begin countdown
    simulate_ble_write("state");          // dump full state
    simulate_ble_write("nonsense");       // unknown command
    simulate_ble_write("help");           // list all commands

    return 0;
}