/* ************************************************************************
 * Copyright 2018-2021 Advanced Micro Devices, Inc.
 * ************************************************************************ */

#pragma once

#include "cblas_interface.hpp"
#include "flops.hpp"
#include "near.hpp"
#include "norm.hpp"
#include "rocblas.hpp"
#include "rocblas_datatype2string.hpp"
#include "rocblas_init.hpp"
#include "rocblas_math.hpp"
#include "rocblas_random.hpp"
#include "rocblas_test.hpp"
#include "rocblas_vector.hpp"
#include "unit.hpp"
#include "utility.hpp"

template <typename T>
void testing_trmm_strided_batched_bad_arg(const Arguments& arg)
{
#if BUILD_WITH_TENSILE
    auto rocblas_trmm_strided_batched_fn = arg.fortran ? rocblas_trmm_strided_batched<T, true>
                                                       : rocblas_trmm_strided_batched<T, false>;
#else
    auto rocblas_trmm_strided_batched_fn = rocblas_trmm_strided_batched<T, false>;
#endif

    const rocblas_int M           = 100;
    const rocblas_int N           = 100;
    const rocblas_int lda         = 100;
    const rocblas_int ldb         = 100;
    const rocblas_int batch_count = 5;
    const T           alpha       = 1.0;
    const T           zero        = 0.0;

    const rocblas_side      side   = rocblas_side_left;
    const rocblas_fill      uplo   = rocblas_fill_upper;
    const rocblas_operation transA = rocblas_operation_none;
    const rocblas_diagonal  diag   = rocblas_diagonal_non_unit;

    rocblas_local_handle handle{arg};

    rocblas_int          K        = side == rocblas_side_left ? M : N;
    const rocblas_stride stride_a = lda * K;
    const rocblas_stride stride_b = ldb * N;
    size_t               size_A   = batch_count * stride_a;
    size_t               size_B   = batch_count * stride_b;

    // allocate memory on device
    device_vector<T> dA(size_A);
    device_vector<T> dB(size_B);
    CHECK_DEVICE_ALLOCATION(dA.memcheck());
    CHECK_DEVICE_ALLOCATION(dB.memcheck());

    EXPECT_ROCBLAS_STATUS(rocblas_trmm_strided_batched_fn(handle,
                                                          side,
                                                          uplo,
                                                          transA,
                                                          diag,
                                                          M,
                                                          N,
                                                          &alpha,
                                                          nullptr,
                                                          lda,
                                                          stride_a,
                                                          dB,
                                                          ldb,
                                                          stride_b,
                                                          batch_count),
                          rocblas_status_invalid_pointer);

    EXPECT_ROCBLAS_STATUS(rocblas_trmm_strided_batched_fn(handle,
                                                          side,
                                                          uplo,
                                                          transA,
                                                          diag,
                                                          M,
                                                          N,
                                                          &alpha,
                                                          dA,
                                                          lda,
                                                          stride_a,
                                                          nullptr,
                                                          ldb,
                                                          stride_b,
                                                          batch_count),
                          rocblas_status_invalid_pointer);

    EXPECT_ROCBLAS_STATUS(rocblas_trmm_strided_batched_fn(handle,
                                                          side,
                                                          uplo,
                                                          transA,
                                                          diag,
                                                          M,
                                                          N,
                                                          nullptr,
                                                          dA,
                                                          lda,
                                                          stride_a,
                                                          dB,
                                                          ldb,
                                                          stride_b,
                                                          batch_count),
                          rocblas_status_invalid_pointer);

    EXPECT_ROCBLAS_STATUS(rocblas_trmm_strided_batched_fn(nullptr,
                                                          side,
                                                          uplo,
                                                          transA,
                                                          diag,
                                                          M,
                                                          N,
                                                          &alpha,
                                                          dA,
                                                          lda,
                                                          stride_a,
                                                          dB,
                                                          ldb,
                                                          stride_b,
                                                          batch_count),
                          rocblas_status_invalid_handle);

    // When batch_count==0, all pointers may be nullptr without error
    EXPECT_ROCBLAS_STATUS(rocblas_trmm_strided_batched_fn(handle,
                                                          side,
                                                          uplo,
                                                          transA,
                                                          diag,
                                                          M,
                                                          N,
                                                          nullptr,
                                                          nullptr,
                                                          lda,
                                                          stride_a,
                                                          nullptr,
                                                          ldb,
                                                          stride_b,
                                                          0),
                          rocblas_status_success);

    // When M==0, all pointers may be nullptr without error
    EXPECT_ROCBLAS_STATUS(rocblas_trmm_strided_batched_fn(handle,
                                                          side,
                                                          uplo,
                                                          transA,
                                                          diag,
                                                          0,
                                                          N,
                                                          nullptr,
                                                          nullptr,
                                                          lda,
                                                          stride_a,
                                                          nullptr,
                                                          ldb,
                                                          stride_b,
                                                          batch_count),
                          rocblas_status_success);

    // When N==0, all pointers may be nullptr without error
    EXPECT_ROCBLAS_STATUS(rocblas_trmm_strided_batched_fn(handle,
                                                          side,
                                                          uplo,
                                                          transA,
                                                          diag,
                                                          M,
                                                          0,
                                                          nullptr,
                                                          nullptr,
                                                          lda,
                                                          stride_a,
                                                          nullptr,
                                                          ldb,
                                                          stride_b,
                                                          batch_count),
                          rocblas_status_success);

    // When alpha==0, A may be nullptr without error
    EXPECT_ROCBLAS_STATUS(rocblas_trmm_strided_batched_fn(handle,
                                                          side,
                                                          uplo,
                                                          transA,
                                                          diag,
                                                          M,
                                                          N,
                                                          &zero,
                                                          nullptr,
                                                          lda,
                                                          stride_a,
                                                          dB,
                                                          ldb,
                                                          stride_b,
                                                          batch_count),
                          rocblas_status_success);
}

template <typename T>
void testing_trmm_strided_batched(const Arguments& arg)
{
#if BUILD_WITH_TENSILE
    auto rocblas_trmm_strided_batched_fn = arg.fortran ? rocblas_trmm_strided_batched<T, true>
                                                       : rocblas_trmm_strided_batched<T, false>;
#else
    auto rocblas_trmm_strided_batched_fn = rocblas_trmm_strided_batched<T, false>;
#endif

    rocblas_int M           = arg.M;
    rocblas_int N           = arg.N;
    rocblas_int lda         = arg.lda;
    rocblas_int ldb         = arg.ldb;
    rocblas_int stride_a    = arg.stride_a;
    rocblas_int stride_b    = arg.stride_b;
    rocblas_int batch_count = arg.batch_count;

    char char_side   = arg.side;
    char char_uplo   = arg.uplo;
    char char_transA = arg.transA;
    char char_diag   = arg.diag;
    T    alpha       = arg.get_alpha<T>();

    rocblas_side      side   = char2rocblas_side(char_side);
    rocblas_fill      uplo   = char2rocblas_fill(char_uplo);
    rocblas_operation transA = char2rocblas_operation(char_transA);
    rocblas_diagonal  diag   = char2rocblas_diagonal(char_diag);

    rocblas_int K = side == rocblas_side_left ? M : N;

    if(stride_a < lda * K)
    {
        rocblas_cout << "WARNING: setting stride_a = lda * (side == rocblas_side_left ? M : N)"
                     << std::endl;
        stride_a = lda * (side == rocblas_side_left ? M : N);
    }
    if(stride_b < ldb * N)
    {
        rocblas_cout << "WARNING: setting stride_b = ldb * N" << std::endl;
        stride_b = ldb * N;
    }
    size_t size_A = batch_count * stride_a;
    size_t size_B = batch_count * stride_b;

    rocblas_local_handle handle{arg};

    // ensure invalid sizes and quick return checked before pointer check
    bool invalid_size = M < 0 || N < 0 || lda < K || ldb < M || batch_count < 0;
    if(M == 0 || N == 0 || batch_count == 0 || invalid_size)
    {
        EXPECT_ROCBLAS_STATUS(rocblas_trmm_strided_batched_fn(handle,
                                                              side,
                                                              uplo,
                                                              transA,
                                                              diag,
                                                              M,
                                                              N,
                                                              nullptr,
                                                              nullptr,
                                                              lda,
                                                              stride_a,
                                                              nullptr,
                                                              ldb,
                                                              stride_b,
                                                              batch_count),
                              invalid_size ? rocblas_status_invalid_size : rocblas_status_success);
        return;
    }

    // Naming: dK is in GPU (device) memory. hK is in CPU (host) memory
    host_vector<T> h_alpha(1);
    host_vector<T> hA(size_A);
    host_vector<T> hB(size_B);
    host_vector<T> hB_1(size_B);
    host_vector<T> hB_2(size_B);
    host_vector<T> cpuB(size_B);

    CHECK_HIP_ERROR(h_alpha.memcheck());
    CHECK_HIP_ERROR(hA.memcheck());
    CHECK_HIP_ERROR(hB.memcheck());
    CHECK_HIP_ERROR(hB_1.memcheck());
    CHECK_HIP_ERROR(hB_2.memcheck());
    CHECK_HIP_ERROR(cpuB.memcheck());

    double gpu_time_used, cpu_time_used;
    gpu_time_used = cpu_time_used = 0.0;
    double rocblas_error          = 0.0;

    // allocate memory on device
    device_vector<T> dA(size_A);
    device_vector<T> dB(size_B);
    device_vector<T> d_alpha(1);

    CHECK_DEVICE_ALLOCATION(dA.memcheck());
    CHECK_DEVICE_ALLOCATION(dB.memcheck());
    CHECK_DEVICE_ALLOCATION(d_alpha.memcheck());

    //  initialize full random matrix hA and hB
    h_alpha[0] = alpha;
    rocblas_seedrand();

    if(arg.alpha_isnan<T>())
    {
        rocblas_init_nan<T>(hA, K, K, lda, stride_a, batch_count);
        rocblas_init_nan<T>(hB, M, N, ldb, stride_b, batch_count);
    }
    else
    {
        rocblas_init<T>(hA);
        rocblas_init<T>(hB);
    }

    hB_1 = hB; // hXorB <- B
    hB_2 = hB; // hXorB <- B
    cpuB = hB; // cpuB <- B

    // copy data from CPU to device
    CHECK_HIP_ERROR(dA.transfer_from(hA));

    if(arg.unit_check || arg.norm_check)
    {
        // calculate dB <- A^(-1) B   rocblas_device_pointer_host
        CHECK_ROCBLAS_ERROR(rocblas_set_pointer_mode(handle, rocblas_pointer_mode_host));
        CHECK_HIP_ERROR(dB.transfer_from(hB_1));

        CHECK_ROCBLAS_ERROR(rocblas_trmm_strided_batched_fn(handle,
                                                            side,
                                                            uplo,
                                                            transA,
                                                            diag,
                                                            M,
                                                            N,
                                                            &h_alpha[0],
                                                            dA,
                                                            lda,
                                                            stride_a,
                                                            dB,
                                                            ldb,
                                                            stride_b,
                                                            batch_count));

        CHECK_HIP_ERROR(hB_1.transfer_from(dB));

        CHECK_ROCBLAS_ERROR(rocblas_set_pointer_mode(handle, rocblas_pointer_mode_device));
        CHECK_HIP_ERROR(dB.transfer_from(hB_2));
        CHECK_HIP_ERROR(d_alpha.transfer_from(h_alpha));

        CHECK_ROCBLAS_ERROR(rocblas_trmm_strided_batched_fn(handle,
                                                            side,
                                                            uplo,
                                                            transA,
                                                            diag,
                                                            M,
                                                            N,
                                                            d_alpha,
                                                            dA,
                                                            lda,
                                                            stride_a,
                                                            dB,
                                                            ldb,
                                                            stride_b,
                                                            batch_count));

        CHECK_HIP_ERROR(hB_2.transfer_from(dB));

        // CPU BLAS
        if(arg.timing)
        {
            cpu_time_used = get_time_us_no_sync();
        }

        for(int i = 0; i < batch_count; i++)
        {
            cblas_trmm<T>(side,
                          uplo,
                          transA,
                          diag,
                          M,
                          N,
                          alpha,
                          hA + i * stride_a,
                          lda,
                          cpuB + i * stride_b,
                          ldb);
        }

        if(arg.timing)
        {
            cpu_time_used = get_time_us_no_sync() - cpu_time_used;
        }

        if(arg.unit_check)
        {
            if(std::is_same<T, rocblas_half>{} && K > 10000)
            {
                // For large K, rocblas_half tends to diverge proportional to K
                // Tolerance is slightly greater than 1 / 1024.0
                const double tol = K * sum_error_tolerance<T>;
                near_check_general<T>(M, N, ldb, stride_b, cpuB, hB_1, batch_count, tol);
                near_check_general<T>(M, N, ldb, stride_b, cpuB, hB_2, batch_count, tol);
            }
            else
            {
                unit_check_general<T>(M, N, ldb, stride_b, cpuB, hB_1, batch_count);
                unit_check_general<T>(M, N, ldb, stride_b, cpuB, hB_2, batch_count);
            }
        }

        if(arg.norm_check)
        {
            auto err1 = std::abs(
                norm_check_general<T>('F', M, N, ldb, stride_b, cpuB, hB_1, batch_count));
            auto err2 = std::abs(
                norm_check_general<T>('F', M, N, ldb, stride_b, cpuB, hB_2, batch_count));
            rocblas_error = err1 > err2 ? err1 : err2;
        }
    }

    if(arg.timing)
    {
        int number_cold_calls = arg.cold_iters;
        int number_hot_calls  = arg.iters;

        CHECK_ROCBLAS_ERROR(rocblas_set_pointer_mode(handle, rocblas_pointer_mode_host));

        for(int i = 0; i < number_cold_calls; i++)
        {
            CHECK_ROCBLAS_ERROR(rocblas_trmm_strided_batched_fn(handle,
                                                                side,
                                                                uplo,
                                                                transA,
                                                                diag,
                                                                M,
                                                                N,
                                                                &h_alpha[0],
                                                                dA,
                                                                lda,
                                                                stride_a,
                                                                dB,
                                                                ldb,
                                                                stride_b,
                                                                batch_count));
        }

        hipStream_t stream;
        CHECK_ROCBLAS_ERROR(rocblas_get_stream(handle, &stream));
        gpu_time_used = get_time_us_sync(stream); // in microseconds
        for(int i = 0; i < number_hot_calls; i++)
        {
            rocblas_trmm_strided_batched_fn(handle,
                                            side,
                                            uplo,
                                            transA,
                                            diag,
                                            M,
                                            N,
                                            &h_alpha[0],
                                            dA,
                                            lda,
                                            stride_a,
                                            dB,
                                            ldb,
                                            stride_b,
                                            batch_count);
        }
        gpu_time_used = get_time_us_sync(stream) - gpu_time_used;

        ArgumentModel<e_side,
                      e_uplo,
                      e_transA,
                      e_diag,
                      e_M,
                      e_N,
                      e_alpha,
                      e_lda,
                      e_stride_a,
                      e_ldb,
                      e_stride_b,
                      e_batch_count>{}
            .log_args<T>(rocblas_cout,
                         arg,
                         gpu_time_used,
                         trmm_gflop_count<T>(M, N, side),
                         ArgumentLogging::NA_value,
                         cpu_time_used,
                         rocblas_error);
    }
}
