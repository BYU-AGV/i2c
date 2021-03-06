#include "i2c.h"

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

    // NOTE we do not call ioctl with I2C_SLAVE hi2c_fdere because we always use the
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
int i2c_write(u8 slave_addr, u8 reg, u8 data1, u8 data2, u8 data3, u8 data4, u8 data5, u8 data6, u8 data7, u8 data8) {
    int retval;
    u8 outbuf[9];

    struct i2c_msg msgs[1];
    struct i2c_rdwr_ioctl_data msgset[1];

    outbuf[0] = reg;
    outbuf[1] = data1;
    outbuf[2] = data2;
    outbuf[3] = data3;
    outbuf[4] = data4;
    outbuf[5] = data5;
    outbuf[6] = data6;
    outbuf[7] = data7;
    outbuf[8] = data8;

    msgs[0].addr = slave_addr;
    msgs[0].flags = 0;
    msgs[0].len = 10;
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
    struct i2c_msg msg[1];
    struct i2c_rdwr_ioctl_data msgset[1];

    msg[0].addr = address;
    msg[0].flags = 0;
    msg[0].len = data_length;
    msg[0].buf = buffer;

    msgset[0].msgs = msg;
    msgset[0].nmsgs = 1;

    if (ioctl(i2c_fd, I2C_RDWR, &msgset)) {
        perror("iotcl(I2C_RDWR) failed to send message set");
        return -1;
    }

    return 0;

    // return i2c_write_msg_set(&msgset);
}

int i2c_send_motor_control_message(uint16_t address, int32_t linear_velocity, int32_t angular_velocity) {
    int retval;
    u8 outbuf[9];

    int32_buffer lin_buff;
    lin_buff.value = linear_velocity;

    int32_buffer ang_buff;
    ang_buff.value = angular_velocity;

    struct i2c_msg msgs[1];
    struct i2c_rdwr_ioctl_data msgset[1];

    outbuf[0] = 0;
    outbuf[1] = lin_buff.buffer[0];
    outbuf[2] = lin_buff.buffer[1];
    outbuf[3] = lin_buff.buffer[2];
    outbuf[4] = lin_buff.buffer[3];
    outbuf[5] = ang_buff.buffer[0];
    outbuf[6] = ang_buff.buffer[1];
    outbuf[7] = ang_buff.buffer[2];
    outbuf[8] = ang_buff.buffer[3];

    msgs[0].addr = address;
    msgs[0].flags = 0;
    msgs[0].len = 10;
    msgs[0].buf = outbuf;

    msgset[0].msgs = msgs;
    msgset[0].nmsgs = 1;

    if (ioctl(i2c_fd, I2C_RDWR, &msgset) < 0) {
        perror("ioctl(I2C_RDWR) in i2c_send_motor_control_message");
        return -1;
    }

    return 0;

    // return i2c_send_msg(address, 9, data_buffer);
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
