#include <stdio.h>
#include <string.h>
#include "minunit.h"
#include "../include/demo.h"

#define TEST_DEMO(name, ticks, type)                                                        \
do {                                                                                        \
    Demo* demo = new_demo("test/resources/" name);                                          \
    int result = demo_parse(demo, -1, false);                                               \
    mu_assert(name ": parsing not success.", result == MEASURED_SUCCESS);                   \
    mu_assert(name ": wrong measured ticks.", demo->measured_ticks == ticks);               \
    mu_assert(name ": wrong game detection.", demo->game == type);                          \
    demo_free(demo);                                                                        \
    tests_run++;                                                                            \
} while (0)

int tests_run = 0;

static char* all_tests() {
    TEST_DEMO("hl2_oe_2153.dem", 2299, HL2_OE_2153);
    TEST_DEMO("hl2_oe_2187.dem", 2232, HL2_OE_2187);
    TEST_DEMO("DMoMM.dem", 876, DMOMM);
    TEST_DEMO("hl2_oe_4044.dem", 2342, HL2_OE_4044);
    TEST_DEMO("Portal_3258.dem", 852, PORTAL_3258);
    TEST_DEMO("Portal_3420.dem", 255, PORTAL_3420);
    TEST_DEMO("Portal_3740.dem", 843, PORTAL_3740);
    TEST_DEMO("Portal_3943.dem", 875, PORTAL_3740);
    TEST_DEMO("Portal_4295.dem", 907, SOURCE_UNPACK);
    TEST_DEMO("Portal_5135.dem", 1247, SOURCE_UNPACK);
    TEST_DEMO("hl2_5135.dem", 2133, SOURCE_UNPACK);
    TEST_DEMO("Portal_1910503.dem", 752, STEAMPIPE);
    TEST_DEMO("Portal_7197370.dem", 908, STEAMPIPE);
    TEST_DEMO("Portal2.dem", 732, PORTAL_2);
    return 0;
}

int main(void) {
    printf("Testing demo.test\n");
    char* result = all_tests();
    if (result != 0) {
        printf("%s\n", result);
    }
    else {
        printf(SUCCESS("All Test passed.\n"));
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}
