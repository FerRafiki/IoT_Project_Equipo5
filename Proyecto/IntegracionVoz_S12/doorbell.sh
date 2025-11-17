#!/bin/bash
card_num=$(grep -m 1 "TFA9912" /proc/asound/cards | awk '{print $1}')


./playback timbre.wav plughw:$card_num


