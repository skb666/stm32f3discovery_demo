#include "xcmd_default_cmds.h"
#include "xcmd.h"
#include "xcmd_default_confg.h"
#include <stdlib.h>

static int cmd_clear(int argc, char* argv[]) {
  xcmd_print("\033c");
  return 0;
}

static int cmd_help(int argc, char* argv[]) {
  xcmd_t* p = NULL;
  XCMD_CMD_FOR_EACH(p) {
    xcmd_print("%-20s %s\r\n", p->name, p->help);
  }
  return 0;
}

static int cmd_keys(int argc, char* argv[]) {
  xcmd_key_t* p = NULL;
  XCMD_KEY_FOR_EACH(p) {
    xcmd_print("0x%08X\t", p->key);
    xcmd_print("%s\r\n", p->help);
  }
  return 0;
}

static int cmd_logo(int argc, char* argv[]) {
  char* log =
      "\
 _  _  ___  __  __  ____  \r\n\
( \\/ )/ __)(  \\/  )(  _ \\ \r\n\
 )  (( (__  )    (  )(_) )\r\n\
(_/\\_)\\___)(_/\\/\\_)(____/\r\n ";
  xcmd_print("%s", log);
  xcmd_print("\r\n%-10s %s %s\r\n", "Build", __DATE__, __TIME__);
  xcmd_print("%-10s %s\r\n", "Version", VERSION);
  return 0;
}

XCMD_EXPORT_CMD(clear, cmd_clear, "clear screen")
XCMD_EXPORT_CMD(help, cmd_help, "show this list")
XCMD_EXPORT_CMD(keys, cmd_keys, "show keys")
XCMD_EXPORT_CMD(logo, cmd_logo, "show logo")

static xcmd_t cmds[] = {
#ifndef ENABLE_XCMD_EXPORT
    {"clear", cmd_clear, "clear screen", NULL},
    {"help", cmd_help, "show this list", NULL},
    {"keys", cmd_keys, "show keys", NULL},
    {"logo", cmd_logo, "show logo", NULL},
#endif
};

void default_cmds_init(void) {
  xcmd_cmd_register(cmds, sizeof(cmds) / sizeof(xcmd_t));
}
