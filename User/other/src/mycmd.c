#include "mycmd.h"

#include "xcmd.h"

extern int shell_led_cmd(int argc, char *argv[]);
extern int shell_i2cdetect_cmd(int argc, char *argv[]);
extern int shell_usart_cmd(int argc, char *argv[]);

#define HELP_LED ("led control")
#define HELP_I2CDETECT ("i2c slave detect")
#define HELP_USART ("usart control")

XCMD_EXPORT_CMD(led, shell_led_cmd, HELP_LED)
XCMD_EXPORT_CMD(i2cdetect, shell_i2cdetect_cmd, HELP_I2CDETECT)

static xcmd_t cmds[] = {
#ifndef ENABLE_XCMD_EXPORT
    {"led", shell_led_cmd, HELP_LED, NULL},
    {"i2cdetect", shell_i2cdetect_cmd, HELP_I2CDETECT, NULL},
    {"usart", shell_usart_cmd, HELP_USART, NULL},
#endif
};

void mycmd_init(void) {
  xcmd_cmd_register(cmds, sizeof(cmds) / sizeof(xcmd_t));
}
