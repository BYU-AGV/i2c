#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <linux/i2c-dev.h>
// Terrible portability hack between arm-linux-gnueabihf-gcc on Mac OS X and
// native gcc on raspbian.
#ifndef I2C_M_RD
#include <linux/i2c.h>
#endif

#define SLAVE_ADDR 0x42

typedef unsigned char u8;

typedef enum { MOTOR_CONTROL_MSG } MSG_TYPE;

typedef struct i2c_sample_data {
    uint8_t linear_velocity;
    uint8_t angular_velocity;
} i2c_sample_data;

typedef union uint16_buffer {
    uint16_t value;
    uint8_t buffer[2];
} uint16_buffer;

typedef union uint32_buffer {
    uint32_t value;
    uint8_t buffer[4];
} uint32_buffer;

typedef union uint64_buffer {
    uint64_t value;
    uint8_t buffer[8];
} uint64_buffer;

typedef union int16_buffer {
    int16_t value;
    uint8_t buffer[2];
} int16_buffer;

typedef union int32_buffer {
    int32_t value;
    uint8_t buffer[4];
} int32_buffer;

typedef union int64_buffer {
    int64_t value;
    uint8_t buffer[8];
} int64_buffer;

// Global file descriptor used to talk to the I2C bus:
int i2c_fd = -1;
// Default RPi B device name for the I2C bus exposed on GPIO2,3 pins (GPIO2=SDA,
// GPIO3=SCL):
const char* i2c_fname = "/dev/i2c-1";

// Returns a new file descriptor for communicating with the I2C bus:
int i2c_init(void) {
    if ((i2c_fd = open(i2c_fname, O_RDWR)) < 0) {
        char err[200];
        sprintf(err, "open('%s') in i2c_init", i2c_fname);
        perror(err);
        return -1;
    } else {
        printf("Opened i2c port\n");
    }

    // NOTE we do not call ioctl with I2C_SLAVE here because we always use the
    // I2C_RDWR ioctl operation to do writes, reads, and combined write-reads.
    // I2C_SLAVE would be used to set the I2C slave address to communicate with.
    // With I2C_RDWR operation, you specify the slave address every time. There
    // is no need to use normal write() or read() syscalls with an I2C device
    // which does not support SMBUS protocol. I2C_RDWR is much better especially
    // for reading device registers which requires a write first before reading
    // the response.

    return i2c_fd;
}

void i2c_close(void) { close(i2c_fd); }

// Write to an I2C slave device's register:
int i2c_write(u8 slave_addr, u8 reg, u8 data1, u8 data2) {
    int retval;
    u8 outbuf[3];

    struct i2c_msg msgs[1];
    struct i2c_rdwr_ioctl_data msgset[1];

    outbuf[0] = reg;
    outbuf[1] = data1;
    outbuf[2] = data2;

    msgs[0].addr = slave_addr;
    msgs[0].flags = 0;
    msgs[0].len = 4;
    msgs[0].buf = outbuf;

    msgset[0].msgs = msgs;
    msgset[0].nmsgs = 1;

    if (ioctl(i2c_fd, I2C_RDWR, &msgset) < 0) {
        perror("ioctl(I2C_RDWR) in i2c_write");
        return -1;
    }

    return 0;
}

// Sends a message set, returns 0 if true;
int32_t i2c_write_msg_set(struct i2c_rdwr_ioctl_data* msgset) {
    if (ioctl(i2c_fd, I2C_RDWR, &msgset)) {
        perror("iotcl(I2C_RDWR) failed to send message set");
        return -1;
    }

    return 0;
}

int i2c_send_msg(uint16_t address, uint16_t data_length, uint8_t* buffer) {
    struct i2c_msg msg;
    struct i2c_rdwr_ioctl_data msgset;

    msg.addr = address;
    msg.flags = 0;
    msg.len = data_length;
    msg.buf = buffer;

    msgset.msgs = &msg;
    msgset.nmsgs = 1;

    return i2c_write_msg_set(&msgset);
}

int i2c_send_motor_control_message(uint16_t address, int32_t linear_velocity, int32_t angular_velocity) {
    uint8_t data_buffer[9];

    int32_buffer lin_buff;
    lin_buff.value = linear_velocity;

    int32_buffer ang_buff;
    ang_buff.value = angular_velocity;

    data_buffer[0] = MOTOR_CONTROL_MSG;
    data_buffer[1] = lin_buff.buffer[0];
    data_buffer[2] = lin_buff.buffer[1];
    data_buffer[3] = lin_buff.buffer[2];
    data_buffer[4] = lin_buff.buffer[3];
    data_buffer[5] = ang_buff.buffer[0];
    data_buffer[6] = ang_buff.buffer[1];
    data_buffer[7] = ang_buff.buffer[2];
    data_buffer[8] = ang_buff.buffer[3];

    return i2c_send_msg(address, 9, data_buffer);
}

void i2c_clear_msg(struct i2c_msg* msg) { free(msg); }

void i2c_clear_msg_set(struct i2c_rdwr_ioctl_data* msg_set) { free(msg_set); }

// Read the given I2C slave device's register and return the read value in
// `*result`:
int i2c_read(u8 slave_addr, u8 reg, u8* result) {
    int retval;
    u8 outbuf[1], inbuf[1];
    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data msgset[1];

    msgs[0].addr = slave_addr;
    msgs[0].flags = 0;
    msgs[0].len = 1;
    msgs[0].buf = outbuf;

    msgs[1].addr = slave_addr;
    msgs[1].flags = I2C_M_RD | I2C_M_NOSTART;
    msgs[1].len = 1;
    msgs[1].buf = inbuf;

    msgset[0].msgs = msgs;
    msgset[0].nmsgs = 2;

    outbuf[0] = reg;

    inbuf[0] = 0;

    *result = 0;
    if (ioctl(i2c_fd, I2C_RDWR, &msgset) < 0) {
        perror("ioctl(I2C_RDWR) in i2c_read");
        return -1;
    }

    *result = inbuf[0];
    return 0;
}

int main(int argc, char** argv) {
    i2c_init();
    while (1) {
        // i2c_write(0x42, 0, 100, 13);
        i2c_send_motor_control_message(SLAVE_ADDR, 100, -110);
    }
    return 0;
}
