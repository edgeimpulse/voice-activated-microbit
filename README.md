# Voice activated micro:bit

A demo application that makes your micro:bit 2 respond to your voice, built with Edge Impulse. This demo uses Machine Learning to analyze the audio feed coming from the microphone, then showing a smiley on screen when it hears "microbit".

## How to build

1. Install [CMake](https://cmake.org), [Python 2.7](https://www.python.org) and the [GNU ARM Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm). Make sure `arm-none-eabi-gcc` is in your PATH.
1. Clone this repository:

    ```
    $ git clone https://github.com/edgeimpulse/voice-activated-microbit
    ```

1. Build the project:

    ```
    $ python build.py
    ```

1. And flash the binary to your micro:bit, by dragging `MICROBIT.hex` onto the `MICROBIT` disk drive.

## How to change the keyword

You can build new models using [Edge Impulse](https://docs.edgeimpulse.com/docs). Once you've trained a model go to **Deployment**, and select **C++ Library**. Then:

1. Remove `source/edge-impulse-sdk`, `source/model-parameters` and `source/tflite-model`.
1. Drag the content of the ZIP file into the `source` folder.
1. If you've picked a different keyword, change this in [source/MicrophoneInferenceTest.cpp](source/MicrophoneInferenceTest.cpp).
1. Rebuild your application.
1. Your micro:bit now responds to your own keyword 🚀.
