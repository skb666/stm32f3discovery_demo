#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "xcmd.h"

int shell_usart_cmd(int argc, char *argv[]) {
  if (argc < 4) {
    xcmd_print("usart need more arguments!\r\n");
    return 0;
  }

  switch (atoi(argv[1])) {
    case 1: {
    
    } break;
    case 3: {
    
    } break;
    default: {
    } break;
  }

  return 0;
}
