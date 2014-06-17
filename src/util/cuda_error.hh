#ifndef __CUDA_ERROR__
#define __CUDA_ERROR__
#include <stdlib.h>
#include <stdio.h>

namespace RAT {

class CUDAException {
  // no content
};

}

#  define CUDA_SAFE_CALL( call) do {                                         \
    cudaError err = call;                                                    \
    if( cudaSuccess != err) {                                                \
        fprintf(stderr, "Cuda error in file '%s' in line %i : %s.\n",        \
                __FILE__, __LINE__, cudaGetErrorString( err) );              \
	throw RAT::CUDAException();					\
    } } while (0)

#  define CUT_CHECK_ERROR(errorMessage) do {                                 \
    cudaError_t err = cudaGetLastError();                                    \
    if( cudaSuccess != err) {                                                \
        fprintf(stderr, "Cuda error: %s in file '%s' in line %i : %s.\n",    \
                errorMessage, __FILE__, __LINE__, cudaGetErrorString( err) );\
        throw RAT::CUDAException();					\
    } } while (0)

#  define CU_SAFE_CALL( call ) do {                                          \
    CUresult err = call;                                                     \
    if( CUDA_SUCCESS != err) {                                               \
        fprintf(stderr, "Cuda driver error %x in file '%s' in line %i.\n",   \
                err, __FILE__, __LINE__ );                                   \
        throw RAT::CUDAException();					\
    } } while (0)

#endif
