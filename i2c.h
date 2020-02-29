#ifndef _I2C_AGV_H_
#define _I2C_AGV_H_

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <linux/i2c-dev.h>
#ifndef I2C_M_RD
#include <linux/i2c.h>
#endif

#define SLAVE_ADDR 0x35

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

// Returns a new file descriptor for communicating with the I2C bus:
int i2c_init(void);

void i2c_close(void);

// Write to an I2C slave device's register:
int i2c_write(u8 slave_addr, u8 reg, u8 data1, u8 data2, u8 data3, u8 data4, u8 data5, u8 data6, u8 data7, u8 data8);

// Sends a message set, returns 0 if true;
int32_t i2c_write_msg_set(struct i2c_rdwr_ioctl_data* msgset);

int i2c_send_msg(uint16_t address, uint16_t data_length, uint8_t* buffer);

int i2c_send_motor_control_message(uint16_t address, int32_t linear_velocity, int32_t angular_velocity);
void i2c_clear_msg(struct i2c_msg* msg);

void i2c_clear_msg_set(struct i2c_rdwr_ioctl_data* msg_set);

// Read the given I2C slave device's register and return the read value in
// `*result`:
int i2c_read(u8 slave_addr, u8 reg, u8* result);

#endif
