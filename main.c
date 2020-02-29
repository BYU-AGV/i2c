// #include "joystick.hh"
#ifdef __cplusplus
extern "C" {
#endif
#include "i2c.h"
#ifdef __cplusplus
}
#endif

#include "joystick.cc"

int main(int argc, char** argv) {
    Joystick joystick("/dev/input/js0");

    // Ensure that it was found and that we can use it
    if (!joystick.isFound())
    {
        printf("open failed.\n");
        exit(1);
    }

    i2c_init();
    while (1) {
        // i2c_write(0x42, 0, 100, 13);
        i2c_send_motor_control_message(SLAVE_ADDR, 130, -10);
        // int32_buffer lin_buff;
        // lin_buff.value = 100;

        // int32_buffer ang_buff;
        // ang_buff.value = -100;

        // i2c_write(0x42, 0, lin_buff.buffer[0], lin_buff.buffer[1], lin_buff.buffer[2], lin_buff.buffer[3], ang_buff.buffer[0], ang_buff.buffer[1], ang_buff.buffer[2], ang_buff.buffer[3]);
    }
    return 0;
}