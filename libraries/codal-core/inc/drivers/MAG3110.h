/*
The MIT License (MIT)

Copyright (c) 2016 British Broadcasting Corporation.
This software is provided by Lancaster University by arrangement with the BBC.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef CODAL_MAG3110_H
#define CODAL_MAG3110_H

#include "CodalConfig.h"
#include "I2C.h"
#include "Compass.h"
#include "Accelerometer.h"

/**
  * I2C constants
  */
#define MAG3110_DEFAULT_ADDR    0x1D

/**
  * MAG3110 Register map
  */
#define MAG_DR_STATUS 0x00
#define MAG_OUT_X_MSB 0x01
#define MAG_OUT_X_LSB 0x02
#define MAG_OUT_Y_MSB 0x03
#define MAG_OUT_Y_LSB 0x04
#define MAG_OUT_Z_MSB 0x05
#define MAG_OUT_Z_LSB 0x06
#define MAG_WHOAMI    0x07
#define MAG_SYSMOD    0x08
#define MAG_OFF_X_MSB 0x09
#define MAG_OFF_X_LSB 0x0A
#define MAG_OFF_Y_MSB 0x0B
#define MAG_OFF_Y_LSB 0x0C
#define MAG_OFF_Z_MSB 0x0D
#define MAG_OFF_Z_LSB 0x0E
#define MAG_DIE_TEMP  0x0F
#define MAG_CTRL_REG1 0x10
#define MAG_CTRL_REG2 0x11

/**
  * Configuration options
  */
struct MAG3110SampleRateConfig
{
    uint32_t        sample_period;
    uint8_t         ctrl_reg1;
};

extern const MAG3110SampleRateConfig MAG3110SampleRate[];

#define MAG3110_SAMPLE_RATES                    11

/**
  * Term to convert sample data into SI units
  */
#define MAG3110_NORMALIZE_SAMPLE(x) (100*x)

/**
  * MAG3110 MAGIC ID value
  * Returned from the MAG_WHO_AM_I register for ID purposes.
  */
#define MAG3110_WHOAMI_VAL 0xC4

namespace codal
{
      /**
      * Class definition for a MAG3110 Compass.
      *
      * Represents an implementation of the Freescale MAG3110 I2C Magnetmometer.
      */
    class MAG3110 : public Compass
    {
        uint16_t                address;                  // I2C address of the magnetmometer.
        Pin&                    int1;                     // Data ready interrupt.
        I2C&		                i2c;                      // The I2C interface the sensor is connected to.

        public:

        MAG3110(I2C& _i2c, Pin& int1, Accelerometer& _accelerometer, CoordinateSpace &coordinateSpace, uint16_t address = MAG3110_DEFAULT_ADDR, uint16_t id = DEVICE_ID_COMPASS);

        int whoAmI();

        /**
          * Configures the compass for the sample rate defined in this object.
          * The nearest values are chosen to those defined that are supported by the hardware.
          * The instance variables are then updated to reflect reality.
          *
          * @return DEVICE_OK or DEVICE_I2C_ERROR if the magnetometer could not be configured.
          */
        virtual int configure() override;

        virtual int requestUpdate() override;

        virtual void idleCallback() override;
    };
}


#endif
