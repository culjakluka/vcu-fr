#include "node.h"

// 65535 = 2^16 - 1
static const uint16_t MAX_POWER_W = 65535;

// pct10: 0..1000 (0.1% steps) -> W
uint16_t pct10_to_powerW(uint16_t pct10)
{
  uint32_t p = (uint32_t)MAX_POWER_W * (uint32_t)pct10;
  return (uint16_t)(p / 1000u);
}

// limiter: ne smije preko BMS limita
uint16_t applyPowerLimit(uint16_t requestedPowerW)
{
  return (requestedPowerW > 65535) ? 65535 : requestedPowerW;
}

uint8_t powerW_to_pwm(uint16_t powerW)
{
  if (powerW >= MAX_POWER_W) return 255;
  return (uint32_t)powerW * 255u / (uint32_t)MAX_POWER_W;
}




