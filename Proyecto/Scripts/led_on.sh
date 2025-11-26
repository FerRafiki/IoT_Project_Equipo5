#!/bin/sh

STATE_LED="/tmp/led0_state"
echo 1 > "$STATE_LED"

# Despertar y modo normal
i2cset -y 2 0x60 0x00 0x00      # MODE1: SLEEP=0
i2cset -y 2 0x60 0x01 0x04      # MODE2: normal

# Asegurar corriente en el canal 0
i2cset -y 2 0x60 0x40 0xFF

# Poner LED0 en modo PWM individual
i2cset -y 2 0x60 0x02 0x02
