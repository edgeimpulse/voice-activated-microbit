#ifndef CODAL_MPU6050_H
#define CODAL_MPU6050_H

#include "CodalConfig.h"
#include "CodalComponent.h"
#include "CoordinateSystem.h"
#include "I2C.h"
#include "Accelerometer.h"
#include "Gyroscope.h"

#define MPU6050_DEFAULT_ADDR    0x68
#define MPU6050_WHOAMI          0x75

#define MPU6050_WHOAMI_VAL      0x34

namespace codal{

    class MPU6050 : public Accelerometer
    {
        I2C& i2c;
        Pin             &int1;              // Data ready interrupt.
        uint16_t        address;            // I2C address of this accelerometer.
        int16_t         temp;
        Sample3D        gyro;             // The gyro value

    public:

        MPU6050(I2C &_i2c, Pin &_int1, CoordinateSpace &coordinateSpace, uint16_t address = MPU6050_DEFAULT_ADDR,  uint16_t id = DEVICE_ID_ACCELEROMETER);

        /**
          * Attempts to read the 8 bit ID from the accelerometer, this can be used for
          * validation purposes.
          *
          * @return the 8 bit ID returned by the accelerometer, or DEVICE_I2C_ERROR if the request fails.
          *
          * @code
          * accelerometer.whoAmI();
          * @endcode
          */
        int whoAmI();

        /**
          * A periodic callback invoked by the fiber scheduler idle thread.
          *
          * Internally calls updateSample().
          */
        virtual void idleCallback() override;

        /**
          * Configures the accelerometer for G range and sample rate defined
          * in this object. The nearest values are chosen to those defined
          * that are supported by the hardware. The instance variables are then
          * updated to reflect reality.
          *
          * @return DEVICE_OK on success, DEVICE_I2C_ERROR if the accelerometer could not be configured.
          */
        virtual int configure() override;

        /**
         * Poll to see if new data is available from the hardware. If so, update it.
         * n.b. it is not necessary to explicitly call this function to update data
         * (it normally happens in the background when the scheduler is idle), but a check is performed
         * if the user explicitly requests up to date data.
         *
         * @return DEVICE_OK on success, DEVICE_I2C_ERROR if the update fails.
         *
         * @note This method should be overidden by the hardware driver to implement the requested
         * changes in hardware.
         */
        virtual int requestUpdate() override;

        virtual int setSleep(bool sleepMode);
    };

}

#endif