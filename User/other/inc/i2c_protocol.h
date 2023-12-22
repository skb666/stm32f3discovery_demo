#ifndef __I2C_PROTOCOL_H__
#define __I2C_PROTOCOL_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* register read callback */
void reg_read_cb_version(void);
void reg_read_cb_led(void);

/* register write callback */
void reg_write_cb_led(void);

#ifdef __cplusplus
}
#endif

#endif
