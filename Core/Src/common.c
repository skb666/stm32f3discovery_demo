#include "common.h"

#include <stdint.h>

#include "main.h"

static uint32_t s_irq_count = 0;

void reset_global_irq(void) {
  s_irq_count = 0;
}

void disable_global_irq(void) {
  __disable_irq();
  s_irq_count++;
  return;
}

void enable_global_irq(void) {
  if (s_irq_count != 0) {
    s_irq_count--;
  }

  if (0 == s_irq_count) {
    __enable_irq();
  }
  return;
}
