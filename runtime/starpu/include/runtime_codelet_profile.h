/**
 *
 * @copyright (c) 2009-2014 The University of Tennessee and The University
 *                          of Tennessee Research Foundation.
 *                          All rights reserved.
 * @copyright (c) 2012-2014 Inria. All rights reserved.
 * @copyright (c) 2012-2014 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria, Univ. Bordeaux. All rights reserved.
 *
 **/

/**
 *
 * @file runtime_codelet_profile.h
 *
 *  MORSE codelets kernel
 *  MORSE is a software package provided by Univ. of Tennessee,
 *  Univ. of California Berkeley and Univ. of Colorado Denver,
 *  and INRIA Bordeaux Sud-Ouest
 *
 * @version 0.9.0
 * @author Cedric Augonnet
 * @author Mathieu Faverge
 * @author Cedric Castagnede
 * @date 2011-06-01
 *
 **/

#ifndef __CODELET_PROFILE_H__
#define __CODELET_PROFILE_H__

#include <math.h>

#define CHAMELEON_CL_CB(name, _m, _n, _k, _nflops)			\
    static measure_t name##_perf[STARPU_NMAXWORKERS];                                          \
    void cl_##name##_callback()                                                                \
    {                                                                                          \
        struct starpu_task *task = starpu_task_get_current();                                  \
        /* XXX we assume square tiles here ! */                                                \
        __attribute__ ((unused)) double M = (double)(_m);                                      \
        __attribute__ ((unused)) double N = (double)(_n);                                      \
        __attribute__ ((unused)) double K = (double)(_k);                                      \
        double flops = (_nflops);                                                              \
        struct starpu_profiling_task_info *info = task->profiling_info;                        \
        double duration = starpu_timing_timespec_delay_us(&info->start_time, &info->end_time); \
        double speed = flops/(1000.0*duration);                                                \
        name##_perf[info->workerid].sum  += speed;                                             \
        name##_perf[info->workerid].sum2 += speed*speed;                                       \
        name##_perf[info->workerid].n    += 1;                                                 \
    }                                                                                          \
    void profiling_display_##name##_info(void)                                                 \
    {                                                                                          \
        unsigned worker;                                                                       \
        int header = 0;                                                                        \
        for (worker = 0; worker < starpu_worker_get_count(); worker++)                         \
            {                                                                                  \
                if (name##_perf[worker].n > 0)                                                 \
                    {                                                                          \
                        if ( !header ) {                                                       \
                            fprintf(stderr, "Performance for kernel " #name "\n");             \
                            fprintf(stderr, "\tWorker  Gflop/s  delta  Nb\n");                 \
                            header = 1;                                                        \
                        }                                                                      \
                        char workername[128];                                                  \
                        starpu_worker_get_name(worker, workername, 128);                       \
                                                                                               \
                        long   n    = name##_perf[worker].n;                                   \
                        double sum  = name##_perf[worker].sum;                                 \
                        double sum2 = name##_perf[worker].sum2;                                \
                                                                                               \
                        double avg = sum / n;                                                  \
                        double sd  = sqrt((sum2 - (sum*sum)/n)/n);                             \
                                                                                               \
                        fprintf(stderr, "\t%s\t%.2lf\t%.2lf\t%ld\n", workername, avg, sd, n);  \
                    }                                                                          \
            }                                                                                  \
    }                                                                                          \
    void estimate_##name##_sustained_peak(double *res_peak)                                    \
    {                                                                                          \
        /* We use a heuristic where we assume that all GPUs have some work, and                \
         * that some CPUs may not have been involved. This may not be                          \
         * applicable to small problems where only a subset of the processing                  \
         * units are used. We assume that all CPUs are the same, so we multiply                \
         * the best performance obtained on a CPU by the number of CPUs, and we                \
         * add this to the sum of the performance obtained by the GPUs. */                     \
        double peak = 0.0;                                                                     \
        double best_cpu = 0.0;                                                                 \
        unsigned ncpus = 0;                                                                    \
                                                                                               \
        unsigned worker;                                                                       \
        for (worker = 0; worker < starpu_worker_get_count(); worker++)                         \
            {                                                                                  \
                unsigned cpu_worker = (starpu_worker_get_type(worker) == STARPU_CPU_WORKER);   \
                if (cpu_worker)                                                                \
                    ncpus++;                                                                   \
                                                                                               \
                if (name##_perf[worker].n > 0)                                                 \
                    {                                                                          \
                        long   n    = name##_perf[worker].n;                                   \
                        double sum  = name##_perf[worker].sum;                                 \
                        double avg = sum / n;                                                  \
                                                                                               \
                        if (cpu_worker)                                                        \
                            {                                                                  \
                                if (avg > best_cpu)                                            \
                                    best_cpu = avg;                                            \
                            }                                                                  \
                        else                                                                   \
                            {                                                                  \
                                peak += avg;                                                   \
                            }                                                                  \
                    }                                                                          \
            }                                                                                  \
                                                                                               \
        peak += ncpus * best_cpu;                                                              \
                                                                                               \
        *res_peak = peak;                                                                      \
    }                                                                                          \

#define CHAMELEON_CL_CB_HEADER(name)                        \
    extern struct starpu_perfmodel*cl_##name##_save;    \
    extern struct starpu_perfmodel cl_##name##_fake;    \
    void cl_##name##_callback();                        \
    void profiling_display_##name##_info(void);         \
    void estimate_##name##_sustained_peak(double *res)

#endif /* __CODELET_PROFILE_H__ */
