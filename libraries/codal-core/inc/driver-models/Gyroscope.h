/*
The MIT License (MIT)

Copyright (c) 2017 Lancaster University.
Copyright (c) 2018 Paul ADAM, Europe.

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

#ifndef CODAL_GYROSCOPE_H
#define CODAL_GYROSCOPE_H

#include "CodalConfig.h"
#include "CodalComponent.h"
#include "Pin.h"
#include "CoordinateSystem.h"
#include "CodalUtil.h"

/**
  * Status flags
  */
#define GYROSCOPE_IMU_DATA_VALID               0x02

/**
  * Gyroscope events
  */
#define GYROSCOPE_EVT_DATA_UPDATE              1

namespace codal
{

    /**
     * Class definition for Gyroscope.
     */
    class Gyroscope : public CodalComponent
    {
        protected:

        uint16_t        samplePeriod;       // The time between samples, in milliseconds.
        uint8_t         sampleRange;        // The sample range of the gyroscope in g.
        Sample3D        sample;             // The last sample read, in the coordinate system specified by the coordinateSpace variable.
        Sample3D        sampleENU;          // The last sample read, in raw ENU format (stored in case requests are made for data in other coordinate spaces)
        CoordinateSpace &coordinateSpace;   // The coordinate space transform (if any) to apply to the raw data from the hardware.

        public:

        /**
          * Constructor.
          * Create a software abstraction of an gyroscope.
          *
          * @param coordinateSpace the orientation of the sensor. Defaults to: SIMPLE_CARTESIAN
          * @param id the unique EventModel id of this component. Defaults to: DEVICE_ID_GYROSCOPE
          *
         */
        Gyroscope(CoordinateSpace &coordinateSpace, uint16_t id = DEVICE_ID_GYROSCOPE);

        /**
          * Attempts to set the sample rate of the gyroscope to the specified value (in ms).
          *
          * @param period the requested time between samples, in milliseconds.
          * @return DEVICE_OK on success, DEVICE_I2C_ERROR is the request fails.
          *
          * @note The requested rate may not be possible on the hardware. In this case, the
          * nearest lower rate is chosen.
          *
          * @note This method should be overriden (if supported) by specific gyroscope device drivers.
          */
        virtual int setPeriod(int period);

        /**
          * Reads the currently configured sample rate of the gyroscope.
          *
          * @return The time between samples, in milliseconds.
          */
        virtual int getPeriod();

        /**
          * Attempts to set the sample range of the gyroscope to the specified value (in dps).
          *
          * @param range The requested sample range of samples, in dps.
          *
          * @return DEVICE_OK on success, DEVICE_I2C_ERROR is the request fails.
          *
          * @note The requested range may not be possible on the hardware. In this case, the
          * nearest lower range is chosen.
          *
          * @note This method should be overriden (if supported) by specific gyroscope device drivers.
          */
        virtual int setRange(int range);

        /**
          * Reads the currently configured sample range of the gyroscope.
          *
          * @return The sample range, in g.
          */
        virtual int getRange();

        /**
         * Configures the gyroscope for dps range and sample rate defined
         * in this object. The nearest values are chosen to those defined
         * that are supported by the hardware. The instance variables are then
         * updated to reflect reality.
         *
         * @return DEVICE_OK on success, DEVICE_I2C_ERROR if the gyroscope could not be configured.
         *
         * @note This method should be overidden by the hardware driver to implement the requested
         * changes in hardware.
         */
        virtual int configure();

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
        virtual int requestUpdate();

        /**
         * Stores data from the gyroscope sensor in our buffer, and perform gesture tracking.
         *
         * On first use, this member function will attempt to add this component to the
         * list of fiber components in order to constantly update the values stored
         * by this object.
         *
         * This lazy instantiation means that we do not
         * obtain the overhead from non-chalantly adding this component to fiber components.
         *
         * @return DEVICE_OK on success, DEVICE_I2C_ERROR if the read request fails.
         */
        virtual int update(Sample3D s);

        /**
          * Reads the last gyroscope value stored, and provides it in the coordinate system requested.
          *
          * @param coordinateSpace The coordinate system to use.
          * @return The force measured in each axis, in dps.
          */
        Sample3D getSample(CoordinateSystem coordinateSystem);

        /**
          * Reads the last gyroscope value stored, and in the coordinate system defined in the constructor.
          * @return The force measured in each axis, in dps.
          */
        Sample3D getSample();

        /**
          * reads the value of the x axis from the latest update retrieved from the gyroscope,
          * using the default coordinate system as specified in the constructor.
          *
          * @return the force measured in the x axis, in dps.
          */
        int getX();

        /**
          * reads the value of the y axis from the latest update retrieved from the gyroscope,
          * using the default coordinate system as specified in the constructor.
          *
          * @return the force measured in the y axis, in dps.
          */
        int getY();

        /**
          * reads the value of the z axis from the latest update retrieved from the gyroscope,
          * using the default coordinate system as specified in the constructor.
          *
          * @return the force measured in the z axis, in dps.
          */
        int getZ();

        /**
          * Destructor.
          */
        ~Gyroscope();

        private:

        /**
          * A service function.
          * It calculates the current angular velocity of the device (x^2 + y^2 + z^2).
          * It does not, however, square root the result, as this is a relatively high cost operation.
          *
          * This is left to application code should it be needed.
          *
          * @return the sum of the square of the angular velocity of the device across all axes.
          */
        uint32_t instantaneousAccelerationSquared();

    };
}

#endif
