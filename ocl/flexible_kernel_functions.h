#ifndef FLEXIBLE_KERNEL_FUNCTIONS_H
#define FLEXIBLE_KERNEL_FUNCTIONS_H
#include <CL/cl.h>
#include "cl_state.h"

// New kernel initialization and enqueue, instead of jumping back and forth in code, decide everything
// in two funcs the miner will just call.
typedef struct _flexible_algorithm_functions_t {
	// Called once at miner initialization. Must put buffers and kernels in clState and they must be ready to go!
	// Resource allocation is really critical to program execution so if it fails, just call quit and be done.
	// If it is successful then we don't care about details!
	// clState - contains buffers and kernels to be initialized
	// scan_size - aka 'global work size' how many hashes to dispatch per batch, aka 'gpu concurrency'
	void(*allocate_resources)(_clState *clState, size_t scan_size);
	cl_int(*enqueue_for_real)(_clState *clState, size_t start, size_t scan, size_t local_size);
} flexible_algorithm_functions_t;

#endif /* FLEXIBLE_KERNEL_FUNCTIONS_H */
