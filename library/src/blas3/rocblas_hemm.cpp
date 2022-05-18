/* ************************************************************************
 * Copyright (C) 2016-2022 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell cop-
 * ies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IM-
 * PLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNE-
 * CTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * ************************************************************************ */
#include "logging.hpp"
#include "rocblas_symm_hemm.hpp"
#include "utility.hpp"

namespace
{
    template <typename>
    constexpr char rocblas_hemm_name[] = "unknown";
    template <>
    constexpr char rocblas_hemm_name<rocblas_float_complex>[] = "rocblas_chemm";
    template <>
    constexpr char rocblas_hemm_name<rocblas_double_complex>[] = "rocblas_zhemm";

    template <typename T>
    rocblas_status rocblas_hemm_impl(rocblas_handle handle,
                                     rocblas_side   side,
                                     rocblas_fill   uplo,
                                     rocblas_int    m,
                                     rocblas_int    n,
                                     const T*       alpha,
                                     const T*       A,
                                     rocblas_int    lda,
                                     const T*       B,
                                     rocblas_int    ldb,
                                     const T*       beta,
                                     T*             C,
                                     rocblas_int    ldc)
    {
        if(!handle)
            return rocblas_status_invalid_handle;

        RETURN_ZERO_DEVICE_MEMORY_SIZE_IF_QUERIED(handle);

        auto layer_mode     = handle->layer_mode;
        auto check_numerics = handle->check_numerics;
        if(layer_mode
           & (rocblas_layer_mode_log_trace | rocblas_layer_mode_log_bench
              | rocblas_layer_mode_log_profile))
        {
            auto side_letter = rocblas_side_letter(side);
            auto uplo_letter = rocblas_fill_letter(uplo);

            if(layer_mode & rocblas_layer_mode_log_trace)
                log_trace(handle,
                          rocblas_hemm_name<T>,
                          side,
                          uplo,
                          m,
                          n,
                          LOG_TRACE_SCALAR_VALUE(handle, alpha),
                          A,
                          lda,
                          B,
                          ldb,
                          LOG_TRACE_SCALAR_VALUE(handle, beta),
                          C,
                          ldc);

            if(layer_mode & rocblas_layer_mode_log_bench)
                log_bench(handle,
                          "./rocblas-bench -f hemm -r",
                          rocblas_precision_string<T>,
                          "--side",
                          side_letter,
                          "--uplo",
                          uplo_letter,
                          "-m",
                          m,
                          "-n",
                          n,
                          LOG_BENCH_SCALAR_VALUE(handle, alpha),
                          "--lda",
                          lda,
                          "--ldb",
                          ldb,
                          LOG_BENCH_SCALAR_VALUE(handle, beta),
                          "--ldc",
                          ldc);

            if(layer_mode & rocblas_layer_mode_log_profile)
                log_profile(handle,
                            rocblas_hemm_name<T>,
                            "side",
                            side_letter,
                            "uplo",
                            uplo_letter,
                            "M",
                            m,
                            "N",
                            n,
                            "lda",
                            lda,
                            "ldb",
                            ldb,
                            "ldc",
                            ldc);
        }

        static constexpr rocblas_stride offset_C = 0, offset_A = 0, offset_B = 0;
        static constexpr rocblas_int    batch_count = 1;
        static constexpr rocblas_stride stride_C = 0, stride_A = 0, stride_B = 0;

        // equivalent argument constraints for symm and hemm
        rocblas_status arg_status = rocblas_symm_arg_check(handle,
                                                           side,
                                                           uplo,
                                                           m,
                                                           n,
                                                           alpha,
                                                           A,
                                                           offset_A,
                                                           lda,
                                                           stride_A,
                                                           B,
                                                           offset_B,
                                                           ldb,
                                                           stride_B,
                                                           beta,
                                                           C,
                                                           offset_C,
                                                           ldc,
                                                           stride_C,
                                                           batch_count);
        if(arg_status != rocblas_status_continue)
            return arg_status;

        static constexpr bool Hermetian = true;

        if(check_numerics)
        {
            bool           is_input = true;
            rocblas_status hemm_check_numerics_status
                = rocblas_hemm_symm_check_numerics<Hermetian>(rocblas_hemm_name<T>,
                                                              handle,
                                                              side,
                                                              uplo,
                                                              m,
                                                              n,
                                                              A,
                                                              lda,
                                                              stride_A,
                                                              B,
                                                              ldb,
                                                              stride_B,
                                                              C,
                                                              ldc,
                                                              stride_C,
                                                              batch_count,
                                                              check_numerics,
                                                              is_input);

            if(hemm_check_numerics_status != rocblas_status_success)
                return hemm_check_numerics_status;
        }

        rocblas_status status = rocblas_status_success;
        status                = rocblas_internal_symm_template<Hermetian>(handle,
                                                           side,
                                                           uplo,
                                                           m,
                                                           n,
                                                           alpha,
                                                           A,
                                                           offset_A,
                                                           lda,
                                                           stride_A,
                                                           B,
                                                           offset_B,
                                                           ldb,
                                                           stride_B,
                                                           beta,
                                                           C,
                                                           offset_C,
                                                           ldc,
                                                           stride_C,
                                                           batch_count);
        if(status != rocblas_status_success)
            return status;

        if(check_numerics)
        {
            bool           is_input = false;
            rocblas_status hemm_check_numerics_status
                = rocblas_hemm_symm_check_numerics<Hermetian>(rocblas_hemm_name<T>,
                                                              handle,
                                                              side,
                                                              uplo,
                                                              m,
                                                              n,
                                                              A,
                                                              lda,
                                                              stride_A,
                                                              B,
                                                              ldb,
                                                              stride_B,
                                                              C,
                                                              ldc,
                                                              stride_C,
                                                              batch_count,
                                                              check_numerics,
                                                              is_input);

            if(hemm_check_numerics_status != rocblas_status_success)
                return hemm_check_numerics_status;
        }
        return status;
    }
}
/*
 * ===========================================================================
 *    C wrapper
 * ===========================================================================
 */

extern "C" {

#ifdef IMPL
#error IMPL ALREADY DEFINED
#endif

#define IMPL(routine_name_, T_)                                                                  \
    rocblas_status routine_name_(rocblas_handle handle,                                          \
                                 rocblas_side   side,                                            \
                                 rocblas_fill   uplo,                                            \
                                 rocblas_int    m,                                               \
                                 rocblas_int    n,                                               \
                                 const T_*      alpha,                                           \
                                 const T_*      A,                                               \
                                 rocblas_int    lda,                                             \
                                 const T_*      B,                                               \
                                 rocblas_int    ldb,                                             \
                                 const T_*      beta,                                            \
                                 T_*            C,                                               \
                                 rocblas_int    ldc)                                             \
    try                                                                                          \
    {                                                                                            \
        return rocblas_hemm_impl(handle, side, uplo, m, n, alpha, A, lda, B, ldb, beta, C, ldc); \
    }                                                                                            \
    catch(...)                                                                                   \
    {                                                                                            \
        return exception_to_rocblas_status();                                                    \
    }

IMPL(rocblas_chemm, rocblas_float_complex);
IMPL(rocblas_zhemm, rocblas_double_complex);

#undef IMPL

} // extern "C"
