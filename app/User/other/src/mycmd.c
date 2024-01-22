#include "mycmd.h"

#include "xcmd.h"

extern int shell_led_cmd(int argc, char *argv[]);
extern int shell_i2cdetect_cmd(int argc, char *argv[]);
extern int shell_i2ctransfer_cmd(int argc, char *argv[]);
extern int shell_usart_cmd(int argc, char *argv[]);
extern int shell_update_cmd(int argc, char *argv[]);

#define HELP_LED ("led [all|3~10] [val]")
#define HELP_I2CDETECT ("i2cdetect [-a]")
#define HELP_I2CTRANSFER ("i2ctransfer { rLENGTH[@ADDR] | wLENGTH[@ADDR] DATA...}...")
#define HELP_USART ("usart {1|3} { {-r|-t [val]} | {-s|-x DATA...} }")
#define HELP_UPDATE ("jump to bld for update")

XCMD_EXPORT_CMD(led, shell_led_cmd, HELP_LED)
XCMD_EXPORT_CMD(i2cdetect, shell_i2cdetect_cmd, HELP_I2CDETECT)
XCMD_EXPORT_CMD(i2ctransfer, shell_i2ctransfer_cmd, HELP_I2CTRANSFER)
XCMD_EXPORT_CMD(usart, shell_usart_cmd, HELP_USART)
XCMD_EXPORT_CMD(update, shell_update_cmd, HELP_UPDATE)

static xcmd_t cmds[] = {
#ifndef ENABLE_XCMD_EXPORT
    {"led", shell_led_cmd, HELP_LED, NULL},
    {"i2cdetect", shell_i2cdetect_cmd, HELP_I2CDETECT, NULL},
    {"i2ctransfer", shell_i2ctransfer_cmd, HELP_I2CTRANSFER, NULL},
    {"usart", shell_usart_cmd, HELP_USART, NULL},
    {"update", shell_update_cmd, HELP_UPDATE, NULL},
#endif
};

void mycmd_init(void) {
  xcmd_cmd_register(cmds, sizeof(cmds) / sizeof(xcmd_t));
}
