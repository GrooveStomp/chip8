/******************************************************************************
  File: graphics_test.c
  Created: 2019-07-23
  Updated: 2019-07-23
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/
#include <stdio.h>

#include "gstest.h"

#include "../graphics.h"
#include "../graphics.c"

int GSTestNumTestsRun = 0;
char GSTestErrMsg[GSTestErrMsgSize];

static char *RunAllTests() {
        return NULL;
}

int main(int argC, char **argV) {
        char *result = RunAllTests();
        if (result != NULL) {
                printf("%s\n", result);
        } else {
                printf("ALL TESTS PASSED\n");
        }
        printf("opcode_test tests run: %d\n", GSTestNumTestsRun);

        return result != NULL;
}
