# xlyrx
Hobby RC 2.4GHz receiver based on cc2500 and STM32, supporting FrSky 2 Way D-series protocol

This project is another opensource receiver for cc2500 based radios.

## Features
- FrSky 2-way (telemetry) protocol for D-series
- based on ARM MCU (STM32)
- Hardware generated high resolution ppm signal (~4 times higher, than FrSky can tranfer)
- bind time RX-TX frequency offset elimination
- high power power amplifier and high sensitivity low noise amplifier cc2500 module
- alternative power input for channels 5,6,7,8
- 2 analog inputs

*ready, but hardcoded for now*
- configurable ppm frequency
	- channels 1,2,5,6 are driven by one HW timer, outputs 4,5,7,8 are driven by another
	- ppm frequency is defined by 3rd timer, all channels have same frequency for now, to be able to have different ppm frequency for each group, packet timeout timer must be changed from regular timer to systick, then we'll have free HW timer to drive second group of outputs with different frequency

*HW ready, SW not yet written*
- UART (3,3V) for loading new firmware (bootloader) and RX user configuration
- UART (3,3V) for external telemetry configuration (raw or FrSky format)

### Known issues
- if TX and RX are too close during binding, binding may not be successful (stuck in binding mode)
	- maybe some AGC tuning could help
	- workaround would be binding procedure restart


## Why?
If you sum up all that time, materials and work, why (I) build this and not buy regular FrSky receiver? For fun! To explore HW, skills... I like the possibility to modify things to way I like them. Easily add new features and make them better than regular ones...
