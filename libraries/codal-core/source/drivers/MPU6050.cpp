#include "CodalConfig.h"
#include "MPU6050.h"
#include "ErrorNo.h"
#include "CodalCompat.h"
#include "CodalFiber.h"

#include "CodalDmesg.h"

using namespace codal;


MPU6050::MPU6050(I2C& _i2c, Pin &_int1, CoordinateSpace &coordinateSpace, uint16_t address,  uint16_t id) : Accelerometer(coordinateSpace, id), i2c(_i2c), int1(_int1)
{
    // Store our identifiers.
    this->id = id;
    this->status = 0;
    this->address = address<<1;

    // Update our internal state for 50Hz at +/- 2g (50Hz has a period af 20ms).
    this->samplePeriod = 20;
    this->sampleRange = 2;

    // Configure and enable the accelerometer.
    configure();
}

int MPU6050::configure()
{
    i2c.writeRegister(address, 0x6B, 0x80);
    fiber_sleep(20);
    i2c.writeRegister(address, 0x6B, 0x00);  /* PWR_MGMT_1    -- SLEEP 0; CYCLE 0; TEMP_DIS 0; CLKSEL 3 (PLL with Z Gyro reference) */
    i2c.writeRegister(address, 0x1A, 0x01);  /* CONFIG        -- EXT_SYNC_SET 0 (disable input pin for data sync) ; default DLPF_CFG = 0 => ACC bandwidth = 260Hz  GYRO bandwidth = 256Hz) */
    i2c.writeRegister(address, 0x1B, 0x18);  /* GYRO_CONFIG   -- FS_SEL = 3: Full scale set to 2000 deg/sec */
    i2c.writeRegister(address, 0x19, 32);

    i2c.writeRegister(address, 0x37, 0x30); // enable interrupt latch; also enable clear of pin by any read
    i2c.writeRegister(address, 0x38, 0x01); // enable raw data interrupt

    DMESG("MPU6050 init %x", whoAmI());
    return DEVICE_OK;
}

int MPU6050::whoAmI()
{
    uint8_t data;
    int result;
    // the default whoami should return 0x68
    result = i2c.readRegister(address, MPU6050_WHOAMI, &data, 1);
    if (result !=0)
        return 0xffff;

    return (data>>1) & 0x3f;
}

int MPU6050::requestUpdate()
{
    int result;
    uint8_t i2cData[16];

    status |= DEVICE_COMPONENT_STATUS_IDLE_TICK;

    if(int1.getDigitalValue() == 1) {
        result = i2c.readRegister(address, 0x3B, (uint8_t *) i2cData, 14);

        if (result != 0)
            return DEVICE_I2C_ERROR;

        sample.x = ((i2cData[0] << 8) | i2cData[1]);
        sample.y = ((i2cData[2] << 8) | i2cData[3]);
        sample.z = ((i2cData[4] << 8) | i2cData[5]);

        gyro.x = (((i2cData[8] << 8) | i2cData[9]));
        gyro.y = (((i2cData[10] << 8) | i2cData[11]));
        gyro.z = (((i2cData[12] << 8) | i2cData[13]));

        int16_t t = (((i2cData[6] << 8) | i2cData[7]));
        temp = t * 10 / 34 + 3653;

        sample.x /= 16;
        sample.y /= 16;
        sample.z /= 16;

        sampleENU = sample;
        update();
    }
    return DEVICE_OK;
};

void MPU6050::idleCallback()
{
    requestUpdate();
}

int MPU6050::setSleep(bool sleepMode)
{
    if (sleepMode)
        return i2c.writeRegister(address, 0x6B, 0x40);
    else
        return configure();
}
