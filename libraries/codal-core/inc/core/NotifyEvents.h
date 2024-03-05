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

#ifndef NOTIFY_EVENTS_H
#define NOTIFY_EVENTS_H

/**
  * This file contains events used on the general purpose Eventing channel
  * DEVICE_ID_NOTIFY, new events should be added here, to prevent duplication.
  */
#define DISPLAY_EVT_FREE                  1
#define CODAL_SERIAL_EVT_TX_EMPTY         2
#define BLE_EVT_SERIAL_TX_EMPTY           3
#define ARCADE_PLAYER_JOIN_RESULT         4

// Any values after 1024 are available for application use
#define DEVICE_NOTIFY_USER_EVENT_BASE     1024

#endif
