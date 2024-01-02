#include "combo_key.h"

typedef struct {
  KEY key[KEY_NUM_MAX];
  KEY_EVENT event[KEY_NUM_MAX];
  int num;
} KEY_LIST;

static KEY_LIST key_list;

KEY *key_list_get(int *num) {
  *num = key_list.num;
  return key_list.key;
}

KEY_EVENT *key_event_get(int *num) {
  *num = key_list.num;
  return key_list.event;
}

uint16_t key_id_get(int index) {
  return key_list.key[index].id;
}

uint16_t key_combo_count(int index) {
  return key_list.key[index].press_time;
}

int8_t key_register(KEY_VALUE (*get)(void), void *custom_data, uint16_t interval) {
  if (key_list.num >= KEY_NUM_MAX) {
    return -1;
  }

  key_list.key[key_list.num].id = key_list.num;
  key_list.key[key_list.num].interval = interval;
  key_list.key[key_list.num].status = KS_NONE;
  key_list.key[key_list.num].press_cnt = 0;
  key_list.key[key_list.num].release_cnt = 0;
  key_list.key[key_list.num].press_time = 0;
  key_list.key[key_list.num].release_time = 0;
  key_list.key[key_list.num].get = get;
  key_list.key[key_list.num].custom_data = custom_data;
  key_list.num += 1;

  return 0;
}

void combo_key_check(void) {
  KEY *key;

  for (int i = 0; i < key_list.num; ++i) {
    key = &key_list.key[i];
    key_list.event[i] = KE_NONE;
    switch (key->status) {
      case KS_NONE: {
        if (key->get() == K_PRESS) {
          key->press_cnt = 0;
          key->release_cnt = 0;
          key->press_time = 0;
          key->release_time = 0;
          key->status = KS_SHAKE;
        }
      } break;
      case KS_SHAKE: {
        if (key->get() == K_PRESS) {
          key->press_cnt += 1;
          if (key->press_cnt > 20) {
            key->press_time += 1;
            key->status = KS_PRESS;
            if (key->press_time == 1) {
              key_list.event[i] = KE_PRESS;
            } else if (key->press_time > 1) {
              key_list.event[i] = KE_COMBO;
            }
          }
        } else {
          key->status = KS_NONE;
        }
      } break;
      case KS_PRESS: {
        if (key->get() == K_PRESS) {
          key->press_cnt += 1;
          if (key->press_cnt == key_list.key[i].interval) {
            key_list.event[i] = KE_LONG_PRESS;
          }
        } else {
          key->release_time += 1;
          key->status = KS_RELEASE;
        }
      } break;
      case KS_RELEASE: {
        if (key->press_cnt >= key_list.key[i].interval) {
          key_list.event[i] = KE_LONG_RELEASE;
          key->status = KS_NONE;
          break;
        }
        if (key->get() == K_PRESS) {
          key->press_cnt = 0;
          key->release_cnt = 0;
          key->status = KS_SHAKE;
        } else {
          key->release_cnt += 1;
          if (key->release_cnt == key_list.key[i].interval) {
            if (key->release_time == 1) {
              key_list.event[i] = KE_RELEASE;
            }
            key->status = KS_NONE;
          }
        }
      } break;
    }
  }
}
