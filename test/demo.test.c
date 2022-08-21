#include <stdio.h>
#include <string.h>
#include "minunit.h"
#include "../include/demo.h"

int tests_run = 0;

static char* test_demo_dmomm() {
    Demo* demo = new_demo("test/resources/DMoMM.dem");
    int result = demo_parse(demo, -1, false);
    mu_assert("[FAILED] DMoMM.dem: parsing not success.", result == MEASURED_SUCCESS);
    mu_assert("[FAILED] DMoMM.dem: wrong measured ticks.", demo->measured_ticks == 876);
    mu_assert("[FAILED] DMoMM.dem: wrong game detection.", demo->game == DMOMM);
    demo_free(demo);
    return 0;
}

static char* test_demo_3420() {
    Demo* demo = new_demo("test/resources/Portal_3420.dem");
    int result = demo_parse(demo, -1, false);
    mu_assert("[FAILED] Portal_3420.dem: parsing not success.", result == MEASURED_SUCCESS);
    mu_assert("[FAILED] Portal_3420.dem: wrong measured ticks.", demo->measured_ticks == 255);
    mu_assert("[FAILED] Portal_3420.dem: wrong game detection.", demo->game == PORTAL_3420);
    demo_free(demo);
    return 0;
}

static char* test_demo_5135() {
    Demo* demo = new_demo("test/resources/Portal_5135.dem");
    int result = demo_parse(demo, -1, false);
    mu_assert("[FAILED] Portal_5135.dem: parsing not success.", result == MEASURED_SUCCESS);
    mu_assert("[FAILED] Portal_5135.dem: wrong measured ticks.", demo->measured_ticks == 1247);
    mu_assert("[FAILED] Portal_5135.dem: wrong game detection.", demo->game == PORTAL_5135);
    demo_free(demo);
    return 0;
}

static char* test_demo_portal2() {
    Demo* demo = new_demo("test/resources/Portal2.dem");
    int result = demo_parse(demo, -1, false);
    mu_assert("[FAILED] Portal2.dem: parsing not success.", result == MEASURED_SUCCESS);
    mu_assert("[FAILED] Portal2.dem: wrong measured ticks.", demo->measured_ticks == 732);
    mu_assert("[FAILED] Portal2.dem: wrong game detection.", demo->game == PORTAL_2);
    demo_free(demo);
    return 0;
}

static char* all_tests() {
    mu_run_test(test_demo_dmomm);
    mu_run_test(test_demo_3420);
    mu_run_test(test_demo_5135);
    mu_run_test(test_demo_portal2);
    return 0;
}

int main(void) {
    printf("Testing demo.test\n");
    char* result = all_tests();
    if (result != 0) {
        printf("%s\n", result);
    }
    else {
        printf("[SUCCESS] All Test passed.\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}