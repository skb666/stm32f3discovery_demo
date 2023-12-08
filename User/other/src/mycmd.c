#include "mycmd.h"

#include "xcmd.h"

extern int shell_led_cmd(int argc, char *argv[]);

#define HELP_LED ("led control")

XCMD_EXPORT_CMD(led, shell_led_cmd, HELP_LED)

static xcmd_t cmds[] = {
#ifndef ENABLE_XCMD_EXPORT
    {"led", shell_led_cmd, HELP_LED, NULL},
#endif
};

void mycmd_init(void) {
  xcmd_cmd_register(cmds, sizeof(cmds) / sizeof(xcmd_t));
}
