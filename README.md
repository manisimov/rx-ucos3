# rx-ucos3

My repository for playing with uC-OS3 RTOS on RX62N microcontroller.
I use [YRDKRX62N](https://www.renesas.com/en/design-resources/boards-kits/yrdkrx62n) development board.

Toolchain is [here](https://llvm-gcc-renesas.com/):

Segger software is [here](https://www.segger.com/downloads/jlink/)

To start after clone:

        git submodule update --init --recursive
        cd 01_template
        mkdir build
        cd build
        cmake ../
        make

To flash the board.
In one terminal:

        make gdbser

In another terminal:

        make gdbl
