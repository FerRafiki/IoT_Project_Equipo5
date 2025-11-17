# Despertar y modo normal
i2cset -y 2 0x60 0x00 0x00      # MODE1: SLEEP=0
i2cset -y 2 0x60 0x01 0x04      # MODE2: normal (totem-pole)

# Asegurar corriente en el canal 0
i2cset -y 2 0x60 0x40 0xFF      # IREF0 = 0xFF (si IREF=0 no verás luz)

# Poner SOLO LED0 en modo PWM individual (LEDOUT0 bits [1:0] = 10b)
i2cset -y 2 0x60 0x02 0x02      # LEDOUT0 = 0b00000010
