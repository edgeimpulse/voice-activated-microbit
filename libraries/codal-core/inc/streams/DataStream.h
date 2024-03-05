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

#ifndef CODAL_DATA_STREAM_H
#define CODAL_DATA_STREAM_H

#include "ManagedBuffer.h"
#include "MessageBus.h"

#define DATASTREAM_MAXIMUM_BUFFERS      1

// Define valid data representation formats supplied by a DataSource.
// n.b. MUST remain in strict monotically increasing order of sample size.
#define DATASTREAM_FORMAT_UNKNOWN           0 
#define DATASTREAM_FORMAT_8BIT_UNSIGNED     1
#define DATASTREAM_FORMAT_8BIT_SIGNED       2
#define DATASTREAM_FORMAT_16BIT_UNSIGNED    3
#define DATASTREAM_FORMAT_16BIT_SIGNED      4
#define DATASTREAM_FORMAT_24BIT_UNSIGNED    5
#define DATASTREAM_FORMAT_24BIT_SIGNED      6
#define DATASTREAM_FORMAT_32BIT_UNSIGNED    7
#define DATASTREAM_FORMAT_32BIT_SIGNED      8

#define DATASTREAM_FORMAT_BYTES_PER_SAMPLE(x) ((x+1)/2)

namespace codal
{
    /**
     * Interface definition for a DataSource.
     */
    class DataSink
    {
    	public:

    	virtual int pullRequest();
    };

    /**
    * Interface definition for a DataSource.
    */
    class DataSource
    {
    	public:

    	virtual ManagedBuffer pull();
    	virtual void connect(DataSink &sink);
        virtual void disconnect();
        virtual int getFormat();
        virtual int setFormat(int format);
    };

    /**
      * Class definition for DataStream.
      * A Datastream holds a number of ManagedBuffer references, provides basic flow control through a push/pull mechanism
      * and byte level access to the datastream, even if it spans different buffers.
      */
    class DataStream : public DataSource, public DataSink
    {
        ManagedBuffer stream[DATASTREAM_MAXIMUM_BUFFERS];
        int bufferCount;
        int bufferLength;
        int preferredBufferSize;
        int writers;
        uint16_t spaceAvailableEventCode;
        uint16_t pullRequestEventCode;
        bool isBlocking;

        DataSink *downStream;
        DataSource *upStream;

        public:

        /**
          * Default Constructor.
          * Creates an empty DataStream.
          *
          * @param upstream the component that will normally feed this datastream with data.
          */
        DataStream(DataSource &upstream);

        /**
          * Destructor.
          * Removes all resources held by the instance.
          */
        ~DataStream();

        /**
          * Determines the value of the given byte in the buffer.
          *
          * @param position The index of the byte to read.
          * @return The value of the byte at the given position, or DEVICE_INVALID_PARAMETER.
          */
        int get(int position);

        /**
          * Sets the byte at the given index to value provided.
          * @param position The index of the byte to change.
          * @param value The new value of the byte (0-255).
          * @return DEVICE_OK, or DEVICE_INVALID_PARAMETER.
          *
          */
        int set(int position, uint8_t value);

        /**
          * Gets number of bytes that are ready to be consumed in this data stream.
          * @return The size in bytes.
          */
        int length();

        /**
         * Determines if any of the data currently flowing through this stream is held in non-volatile (FLASH) memory.
         * @return true if one or more of the ManagedBuffers in this stream reside in FLASH memory, false otherwise.
         */
    	  bool isReadOnly();

        /**
         * Define a downstream component for data stream.
         *
         * @sink The component that data will be delivered to, when it is available
         */
        virtual void connect(DataSink &sink) override;

        /**
         * Define a downstream component for data stream.
         *
         * @sink The component that data will be delivered to, when it is available
         */
        virtual void disconnect() override;

        /**
         *  Determine the data format of the buffers streamed out of this component.
         */
        virtual int getFormat() override;

        /**
         * Determine the number of bytes that are currnetly buffered before blocking subsequent push() operations.
         * @return the current preferred buffer size for this DataStream
         */
        int getPreferredBufferSize();

        /**
         * Define the number of bytes that should be buffered before blocking subsequent push() operations.
         * @param size The number of bytes to buffer.
         */
        void setPreferredBufferSize(int size);

        /**
         * Determines if this stream acts in a synchronous, blocking mode or asynchronous mode. In blocking mode, writes to a full buffer
         * will result int he calling fiber being blocked until space is available. Downstream DataSinks will also attempt to process data
         * immediately as it becomes available. In non-blocking asynchronpus mode, writes to a full buffer are dropped and downstream Datasinks will
         * be processed in a new fiber.
         */
        void setBlocking(bool isBlocking);

        /**
         * Determines if a buffer of the given size can be added to the buffer.
         *
         * @param size The number of bytes to add to the buffer.
         * @return true if there is space for "size" bytes in the buffer. false otherwise.
         */
        bool canPull(int size = 0);

        /**
         * Determines if the DataStream can accept any more data.
         *
         * @return true if there if the buffer is ful, and can accept no more data at this time. False otherwise.
         */
        bool full();

    	/**
    	 * Provide the next available ManagedBuffer to our downstream caller, if available.
    	 */
    	virtual ManagedBuffer pull();

    	/**
    	 * Deliver the next available ManagedBuffer to our downstream caller.
    	 */
    	virtual int pullRequest();

        private:
        /**
         * Issue a deferred pull request to our downstream component, if one has been registered.
         */
        void onDeferredPullRequest(Event);

    };
}

#endif
