#include "node.h"

// dogovor oko vrijednosti; ako nema dogovorene vrijednosti, 80000 (80kW) za pocetak
static const uint16_t MAX_POWER_W = 80000;

// pct10: 0..1000 (0.1% steps) -> W
uint16_t pct10_to_powerW(uint16_t pct10)
{
  uint32_t p = (uint32_t)MAX_POWER_W * (uint32_t)pct10;
  return (uint16_t)(p / 1000u);
}

// limiter: ne smije preko BMS limita
uint16_t applyPowerLimit(uint16_t requestedPowerW, uint16_t powerLimitW)
{
  return (requestedPowerW > powerLimitW) ? powerLimitW : requestedPowerW;
}

uint8_t powerW_to_pwm(uint16_t powerW)
{
  if (powerW >= MAX_POWER_W) return 255;
  return (uint32_t)powerW * 255u / (uint32_t)MAX_POWER_W;
}
