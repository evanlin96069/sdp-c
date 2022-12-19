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
    TEST_DEMO("hl2_oe_2153.dem", 2300, HL2_OE_2153);
    TEST_DEMO("hl2_oe_2187.dem", 2233, HL2_OE_2187);
    TEST_DEMO("DMoMM.dem", 877, DMOMM);
    TEST_DEMO("hl2_oe_4044.dem", 2343, HL2_OE_4044);
    TEST_DEMO("Portal_3258.dem", 853, PORTAL_3258);
    TEST_DEMO("Portal_3420.dem", 256, PORTAL_3420);
    TEST_DEMO("Portal_3740.dem", 844, PORTAL_3740);
    TEST_DEMO("Portal_3943.dem", 876, PORTAL_3740);
    TEST_DEMO("Portal_4295.dem", 908, SOURCE_UNPACK);
    TEST_DEMO("Portal_5135.dem", 1248, SOURCE_UNPACK);
    TEST_DEMO("hl2_5135.dem", 2134, SOURCE_UNPACK);
    TEST_DEMO("Portal_1910503.dem", 753, STEAMPIPE);
    TEST_DEMO("Portal_7197370.dem", 909, STEAMPIPE);
    TEST_DEMO("Portal2.dem", 733, PORTAL_2);
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
