#include <stdio.h>
#include <string.h>
#include "minunit.h"
#include "../include/demo.h"

int tests_run = 0;

static char* test_demo_parse() {
    Demo* demo = new_demo("test/resources/Portal_5135.dem");
    int tick = demo_parse(demo);
    mu_assert("[FAILED] Portal_5135.dem: wrong measured ticks.", tick == 1247);
    mu_assert("[FAILED] Portal_5135.dem: wrong file name.", strcmp(demo->file_name, "Portal_5135.dem") == 0);
    mu_assert("[FAILED] Portal_5135.dem: wrong map name.", strcmp(demo->header.map_name, "testchmb_a_11") == 0);
    demo_free(demo);
    return 0;
}

static char* all_tests() {
    mu_run_test(test_demo_parse);
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