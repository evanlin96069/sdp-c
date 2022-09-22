#include "../src/utils/ansi-style.h"

#define SUCCESS(text) "["SET_FG(GRN) "SUCCESS" RESET "] " text
#define FAILED(text) "["SET_FG(RED) "FAILED" RESET "] " text

#define mu_assert(message, test) do { if (!(test)) return FAILED(message); } while (0)
#define mu_run_test(test) do { char *message = test(); tests_run++; \
                                if (message) return message; } while (0)
extern int tests_run;
