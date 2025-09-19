
# Future Composer Player on STM32

This is a Future Composer 1.4 player ported to STM32. The sole purpose of this project is to play the tune of Keil uVision keygen by EDGE team.

# Building

This project uses MDK6 (or Keil Studio Pack) in VSCode to build.

# How to use

By default, this runs on a classic blue-pill STM32F103C8T6. Uses an 8 MHz HSE. Audio is output on PA8 pin as PWM wave.

As a practice on C++ compile time features, I've used C++20 compile time feature to invert the byte order of FC14 file header metadata when compiling. The smart pointers of libfc14audiodecoder and all heap operations are removed. This project doesn't use a byte of heap.

Modifications to libfc14audiodecoder are surrounded by conditional compilation ifdefs. Define `FC14_USE_OG` to use the original codebase.

# License

GPL-3.0
