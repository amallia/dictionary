
#ifndef DICT_BENCH_H
#define DICT_BENCH_H

#include <cstdio>

// useful for basic info (0)
static inline void native_cpuid(unsigned int *eax, unsigned int *ebx,
                                unsigned int *ecx, unsigned int *edx) {
    __asm volatile("cpuid"
                   : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                   : "0"(*eax), "2"(*ecx));
}

// this is quite imperfect, but can be handy
static inline const char *guessprocessor() {
    unsigned eax = 1, ebx = 0, ecx = 0, edx = 0;
    native_cpuid(&eax, &ebx, &ecx, &edx);
    const char *codename;
    switch (eax >> 4) {
    case 0x506E:
        codename = "Skylake";
        break;
    case 0x406C:
        codename = "CherryTrail";
        break;
    case 0x306D:
        codename = "Broadwell";
        break;
    case 0x306C:
        codename = "Haswell";
        break;
    case 0x306A:
        codename = "IvyBridge";
        break;
    case 0x206A:
    case 0x206D:
        codename = "SandyBridge";
        break;
    case 0x2065:
    case 0x206C:
    case 0x206F:
        codename = "Westmere";
        break;
    case 0x106E:
    case 0x106A:
    case 0x206E:
        codename = "Nehalem";
        break;
    case 0x1067:
    case 0x106D:
        codename = "Penryn";
        break;
    case 0x006F:
    case 0x1066:
        codename = "Merom";
        break;
    case 0x0066:
        codename = "Presler";
        break;
    case 0x0063:
    case 0x0064:
        codename = "Prescott";
        break;
    case 0x006D:
        codename = "Dothan";
        break;
    case 0x0366:
        codename = "Cedarview";
        break;
    case 0x0266:
        codename = "Lincroft";
        break;
    case 0x016C:
        codename = "Pineview";
        break;
    default:
        codename = "UNKNOWN";
        break;
    }
    return codename;
}

static inline void tellmeall() {

#ifdef __arm__
    printf("ARM processor detected\n");
#else
    printf("Intel processor:  %s\t", guessprocessor());
#endif

#ifdef __VERSION__
    printf(" compiler version: %s\t", __VERSION__);
#endif
#ifndef __AVX2__
    printf("AVX2 is NOT available.\n");
#else
    printf("\tAVX2 is available.");
#endif
    printf("\n");

}


#define RDTSC_START(cycles)                                                   \
    do {                                                                      \
        register unsigned cyc_high, cyc_low;                                  \
        __asm volatile(                                                       \
            "cpuid\n\t"                                                       \
            "rdtsc\n\t"                                                       \
            "mov %%edx, %0\n\t"                                               \
            "mov %%eax, %1\n\t"                                               \
            : "=r"(cyc_high), "=r"(cyc_low)::"%rax", "%rbx", "%rcx", "%rdx"); \
        (cycles) = ((uint64_t)cyc_high << 32) | cyc_low;                      \
    } while (0)

#define RDTSC_FINAL(cycles)                                                   \
    do {                                                                      \
        register unsigned cyc_high, cyc_low;                                  \
        __asm volatile(                                                       \
            "rdtscp\n\t"                                                      \
            "mov %%edx, %0\n\t"                                               \
            "mov %%eax, %1\n\t"                                               \
            "cpuid\n\t"                                                       \
            : "=r"(cyc_high), "=r"(cyc_low)::"%rax", "%rbx", "%rcx", "%rdx"); \
        (cycles) = ((uint64_t)cyc_high << 32) | cyc_low;                      \
    } while (0)



/*
 * Prints the best number of operations per cycle where
 * test is the function call, answer is the expected answer generated by
 * test, repeat is the number of times we should repeat and size is the
 * number of operations represented by test.
 */
#define BEST_TIME(test, answer, repeat, size)                         \
    do {                                                              \
        printf("%40s: ", #test);                                        \
        fflush(NULL);                                                 \
        uint64_t cycles_start, cycles_final, cycles_diff;             \
        uint64_t min_diff = (uint64_t)-1;                             \
        int wrong_answer = 0;                                         \
        for (int i = 0; i < repeat; i++) {                            \
            __asm volatile("" ::: /* pretend to clobber */ "memory"); \
            RDTSC_START(cycles_start);                                \
            if (test != answer) wrong_answer = 1;                     \
            RDTSC_FINAL(cycles_final);                                \
            cycles_diff = (cycles_final - cycles_start);              \
            if (cycles_diff < min_diff) min_diff = cycles_diff;       \
        }                                                             \
        uint64_t S = (uint64_t)size;                                  \
        float cycle_per_op = (min_diff) / (float)S;                   \
        printf(" %.2f cycles per decoded value", cycle_per_op);       \
        if (wrong_answer) printf(" [ERROR]");                         \
        printf("\n");                                                 \
        fflush(NULL);                                                 \
    } while (0)

#endif