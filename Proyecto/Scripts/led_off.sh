#!/bin/sh

STATE_LED="/tmp/led0_state"
echo 0 > "$STATE_LED"

# Apagar LED0 (LEDOUT0 = 0b00)
i2cset -y 2 0x60 0x02 0x00
