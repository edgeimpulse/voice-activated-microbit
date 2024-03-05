/*
The MIT License (MIT)

Copyright (c) 2017 Lancaster University.

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

#ifndef LSM303_M_H
#define LSM303_M_H

#include "CodalConfig.h"
#include "CodalComponent.h"
#include "CodalUtil.h"
#include "CoordinateSystem.h"
#include "Pin.h"
#include "I2C.h"
#include "Compass.h"


/**
  * Term to convert sample data into SI units. 
  */
#define LSM303_M_NORMALIZE_SAMPLE(x) (150 * (int)(x))

/**
  * LSM303_M MAGIC ID value
  * Returned from the MAG_WHO_AM_I register for ID purposes.
  */
#define LSM303_M_WHOAMI_VAL         0x40

/**
  * I2C constants
  */
#define LSM303_M_DEFAULT_ADDR       0x3C

/**
  * LSM303_M Register map
  */
#define LSM303_OFFSET_X_REG_L_M			0x45
#define LSM303_OFFSET_X_REG_H_M			0x46
#define LSM303_OFFSET_Y_REG_L_M			0x47
#define LSM303_OFFSET_Y_REG_H_M			0x48
#define LSM303_OFFSET_Z_REG_L_M			0x49
#define LSM303_OFFSET_Z_REG_H_M			0x4A
#define LSM303_WHO_AM_I_M				    0x4F
#define LSM303_CFG_REG_A_M				  0x60
#define LSM303_CFG_REG_B_M				  0x61
#define LSM303_CFG_REG_C_M				  0x62
#define LSM303_INT_CRTL_REG_M			  0x63
#define LSM303_INT_SOURCE_REG_M			0x64
#define LSM303_INT_THS_L_REG_M			0x65
#define LSM303_INT_THS_H_REG_M			0x66
#define LSM303_STATUS_REG_M				  0x67
#define LSM303_OUTX_L_REG_M				  0x68
#define LSM303_OUTX_H_REG_M				  0x69
#define LSM303_OUTY_L_REG_M				  0x6A
#define LSM303_OUTY_H_REG_M				  0x6B
#define LSM303_OUTZ_L_REG_M				  0x6C
#define LSM303_OUTZ_H_REG_M				  0x6D

/**
  * LSM303_M constants
  */
#define LSM303_M_STATUS_DATA_READY    0x08

/**
 * LSM303 Status flags
 */
#define LSM303_M_STATUS_ENABLED       0x0100
#define LSM303_M_STATUS_SLEEPING      0x0200

/**
 * Class definition for LSM303_M.
 *
 * This class provides the low level driver implementation for the LSM303_M Magnetometer
 *
 */
namespace codal
{
  class LSM303Magnetometer : public Compass
  {
      I2C&            i2c;                    // The I2C interface to use.
      Pin&            int1;                   // Data ready interrupt.
      uint16_t        address;                // I2C address of this compass.

      public:

      /**
       * Constructor.
       * Create a software abstraction of an compass.
       *
       * @param coordinateSpace The orientation of the sensor. Defaults to: SIMPLE_CARTESIAN
       * @param id The unique EventModel id of this component. Defaults to: DEVICE_ID_ACCELEROMETER
       *
       */
      LSM303Magnetometer(I2C& _i2c, Pin &_int1, CoordinateSpace &coordinateSpace, uint16_t address = LSM303_M_DEFAULT_ADDR, uint16_t id = DEVICE_ID_COMPASS);

      /**
       * Configures the magnetometer for the sample rate defined
       * in this object. The nearest values are chosen to those defined
       * that are supported by the hardware. The instance variables are then
       * updated to reflect reality.
       *
       * @return DEVICE_OK on success, DEVICE_I2C_ERROR if the magnetometer could not be configured.
       *
       * @note This method should be overidden by the hardware driver to implement the requested
       * changes in hardware.
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

      /**
       * A periodic callback invoked by the fiber scheduler idle thread.
       *
       * Internally calls updateSample().
       */
      virtual void idleCallback() override;

      /**
       * Puts the component in (or out of) sleep (low power) mode.
       */
      virtual int setSleep(bool doSleep) override;

      /**
       * Attempts to read the 8 bit WHO_AM_I value from the accelerometer
       *
       * @return true if the WHO_AM_I value is succesfully read. false otherwise.
       */
      static int isDetected(I2C &i2c, uint16_t address = LSM303_M_DEFAULT_ADDR);

      /**
       * Destructor.
       */
      ~LSM303Magnetometer();

  };
}
#endif

