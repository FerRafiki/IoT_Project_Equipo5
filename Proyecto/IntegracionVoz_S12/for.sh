for i in $(seq 0 15); do
  addr=$(printf "0x%X" $((0x40 + i)))
  i2cset -y 2 0x60 $addr 0xFF
done
