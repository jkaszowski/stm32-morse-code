# STM32 morse code analyzer
This project contains software for STM32 based morse code receiver, done as part of optoelectronics project at PWr.

In order to receive morse code sent by another board, an opto transistor is connected into ADC input of BlackPill.
The algorithm samples the input in real time, decodes the received signal in morse code form into human readable text.
