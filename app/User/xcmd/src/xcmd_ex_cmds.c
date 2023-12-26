#include "xcmd_ex_cmds.h"

#include <stdlib.h>
#include <string.h>

#include "xcmd.h"

#define HELP_RUN ("Run cmd. Usage: run cmd")
#define HELP_HISTORY ("show history list")
#define HELP_EXAMPLE ("example [-f|-i|-s] [val]")
#define HELP_DELCMD ("delete cmd [val]")
#define HELP_DELKEY ("delete key [val]")
#define HELP_COLOR ("printf color text")

static uint8_t param_check(int need, int argc, char* argv[]) {
  uint8_t i, ret = 0;
  if (need < (argc)) {
    ret = 1;
  } else {
    xcmd_print("err need %d but input %d:\r\n", need, argc - 1);
    xcmd_print("input= ");
    for (i = 0; i < argc; i++) {
      if (argv[i] != NULL) {
        xcmd_print("%s ", argv[i]);
      }
    }
    xcmd_print("\r\n");
    ret = 0;
  }
  return ret;
}

static int cmd_run(int argc, char* argv[]) {
  if (argc >= 2) {
    xcmd_exec(argv[1]);
  } else {
    xcmd_print("%s\r\n", HELP_RUN);
  }
  return 0;
}

static int cmd_example(int argc, char* argv[]) {
  uint8_t i;
  if (param_check(1, argc, argv)) {
    if (strcmp(argv[1], "-s") == 0) {
      for (i = 2; i < argc; i++) {
        xcmd_print("%s\r\n", argv[i]);
      }
    }
    if (strcmp(argv[1], "-i") == 0) {
      for (i = 2; i < argc; i++) {
        xcmd_print("%d\r\n", atoi(argv[i]));
      }
    }
    if (strcmp(argv[1], "-f") == 0) {
      for (i = 2; i < argc; i++) {
        xcmd_print("%f\r\n", atof(argv[i]));
      }
    }
  }
  return 0;
}

static int cmd_history(int argc, char* argv[]) {
  uint16_t len = xcmd_history_len();

  do {
    xcmd_history_next();
  } while (len--);

  while (1) {
    char* line = xcmd_history_prev();
    if (line) {
      xcmd_print("%s\r\n", line);
    } else {
      break;
    }
  }
  return 0;
}

static int cmd_delete_cmd(int argc, char* argv[]) {
  int res = 0;
  if (argc == 2) {
    res = xcmd_unregister_cmd(argv[1]);
    if (res) {
      goto error;
    }
  }
  return 0;
error:
  xcmd_print("Too many parameters are entered or there is no command\r\n");
  return -1;
}

static int cmd_delete_key(int argc, char* argv[]) {
  int res = 0;
  if (argc == 2) {
    res = xcmd_unregister_key(argv[1]);
    if (res) {
      goto error;
    }
  }
  return 0;
error:
  xcmd_print("Too many parameters are entered or there is no command\r\n");
  return -1;
}

static int cmd_print_color(int argc, char* argv[]) {
  xcmd_print(TX_DEF "txt_color = DEF    \r\n" TX_DEF);
  xcmd_print(TX_RED "txt_color = RED    \r\n" TX_DEF);
  xcmd_print(TX_BLACK "txt_color = BLACK  \r\n" TX_DEF);
  xcmd_print(TX_GREEN "txt_color = GREEN  \r\n" TX_DEF);
  xcmd_print(TX_YELLOW "txt_color = YELLOW \r\n" TX_DEF);
  xcmd_print(TX_BLUE "txt_color = BLUE   \r\n" TX_DEF);
  xcmd_print(TX_WHITE "txt_color = WHITE  \r\n" TX_DEF);
  xcmd_print(TX_WHITE "txt_color = WHITE  \r\n" TX_DEF);

  xcmd_print(BK_DEF "background_color = BK_DEF" BK_DEF "\r\n");
  xcmd_print(BK_BLACK "background_color = BK_BLACK" BK_DEF "\r\n");
  xcmd_print(BK_RED "background_color = BK_RED" BK_DEF "\r\n");
  xcmd_print(BK_GREEN "background_color = BK_GREEN" BK_DEF "\r\n");
  xcmd_print(BK_YELLOW "background_color = BK_YELLOW" BK_DEF "\r\n");
  xcmd_print(BK_BLUE "background_color = BK_BLUE" BK_DEF "\r\n");
  xcmd_print(BK_WHITE "background_color = BK_WHITE" BK_DEF "\r\n");
  return 0;
}

XCMD_EXPORT_CMD(run, cmd_run, HELP_RUN)
XCMD_EXPORT_CMD(history, cmd_history, HELP_HISTORY)
XCMD_EXPORT_CMD(example, cmd_example, HELP_EXAMPLE)
XCMD_EXPORT_CMD(delcmd, cmd_delete_cmd, HELP_DELCMD)
XCMD_EXPORT_CMD(delkey, cmd_delete_key, HELP_DELKEY)
XCMD_EXPORT_CMD(color, cmd_print_color, HELP_COLOR)

static xcmd_t cmds[] = {
#ifndef ENABLE_XCMD_EXPORT
    {"run", cmd_run, HELP_RUN, NULL},
    {"history", cmd_history, HELP_HISTORY, NULL},
    {"example", cmd_example, HELP_EXAMPLE, NULL},
    {"delcmd", cmd_delete_cmd, HELP_DELCMD, NULL},
    {"delkey", cmd_delete_key, HELP_DELKEY, NULL},
    {"color", cmd_print_color, HELP_COLOR, NULL},
#endif
};

void ex_cmds_init(void) {
  xcmd_cmd_register(cmds, sizeof(cmds) / sizeof(xcmd_t));
}
