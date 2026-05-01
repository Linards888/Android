#pragma once
#include <Arduino.h>

namespace mot1 {
  uint8_t MA1 = 0;
  uint8_t MA2 = 1;
}


namespace pinT {
  uint8_t Front = 0;
  uint8_t Right = 1;
  uint8_t Left = 3;
  uint8_t right_side = 4;
  uint8_t left_side = 5;
}


namespace pinS {
  uint8_t SFront = 0;
  uint8_t SRight = 1;
  uint8_t SLeft = 3;
  uint8_t S_right_side = 4;
  uint8_t S_left_side = 5;
}

namespace pin {
  uint8_t sda = 8;
  uint8_t scl = 9;
}