#ifndef MINUNIT_H
#define MINUNIT_H
#include <stdio.h>

#ifdef DEBUG
#define DEBUG_PRINT(...) do{ fprintf( stderr, __VA_ARGS__ ); } while( false )
#else
#define DEBUG_PRINT(...) do{ } while ( false )
#endif


#define mu_suite_start() char *message = NULL
#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_run_test(test) do { char *message = test(); tests_run++; \
                                    if (message) return message; } while (0)

#define RUN_TESTS(name) int main(int argc, char *argv[]) {\
        argc = 1; \
        printf("----\nRUNNING: %s\n", argv[0]);\
        char *result = name();\
        if (result != 0) {\
                        printf("FAILED: %s\n", result);\
                    }\
        else {\
                        printf("ALL TESTS PASSED\n");\
                    }\
        printf("Tests run: %d\n", tests_run);\
        return result != 0;\
}

int tests_run;

#endif
