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

#ifndef CODAL_MIXER_H
#define CODAL_MIXER_H

#include "DataStream.h"

namespace codal
{

class MixerChannel
{
private:
    MixerChannel *next;
    DataStream *stream;
    friend class Mixer;

public:
    uint16_t volume;
    bool isSigned;
};

class Mixer : public DataSource, public DataSink
{
    MixerChannel *channels;
    DataSink *downStream;

public:
    /**
     * Default Constructor.
     * Creates an empty Mixer.
     */
    Mixer();

    /**
     * Destructor.
     * Removes all resources held by the instance.
     */
    ~Mixer();

    MixerChannel *addChannel(DataStream &stream);

    /**
     * Provide the next available ManagedBuffer to our downstream caller, if available.
     */
    virtual ManagedBuffer pull();

    /**
     * Deliver the next available ManagedBuffer to our downstream caller.
     */
    virtual int pullRequest();

    /**
     * Define a downstream component for data stream.
     *
     * @sink The component that data will be delivered to, when it is available
     */
    virtual void connect(DataSink &sink);
};

} // namespace codal

#endif
