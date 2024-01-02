#ifndef __COMBO_KEY_H__
#define __COMBO_KEY_H__

#include <stdint.h>

#define KEY_NUM_MAX 1

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  K_RELEASE,
  K_PRESS,
} KEY_VALUE;

typedef enum {
  KS_NONE = 0,
  KS_RELEASE,
  KS_SHAKE,
  KS_PRESS,
} KEY_STATUS;

typedef enum {
  KE_PRESS,
  KE_RELEASE,
  KE_LONG_PRESS,
  KE_LONG_RELEASE,
  KE_COMBO,
  KE_NONE,
} KEY_EVENT;

typedef struct {
  uint16_t id;
  uint16_t interval;
  KEY_STATUS status;
  uint16_t press_cnt;
  uint16_t release_cnt;
  uint16_t press_time;
  uint16_t release_time;
  KEY_VALUE (*get)(void);
  void *custom_data;
} KEY;

KEY *key_list_get(int *num);
KEY_EVENT *key_event_get(int *num);
uint16_t key_id_get(int index);
uint16_t key_combo_count(int index);

int8_t key_register(KEY_VALUE (*get)(void), void *custom_data, uint16_t interval);
void combo_key_check(void);

#ifdef __cplusplus
}
#endif

#endif
