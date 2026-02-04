#include <hip/hip_runtime_api.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("==============================================\n");
    printf("HIP Runtime Test - Device Properties\n");
    printf("==============================================\n\n");

    // Get device count
    int deviceCount = 0;
    hipError_t err = hipGetDeviceCount(&deviceCount);
    if (err != hipSuccess) {
        printf("ERROR: hipGetDeviceCount failed: %d\n", err);
        return 1;
    }
    printf("Found %d HIP device(s)\n\n", deviceCount);

    for (int i = 0; i < deviceCount; i++) {
        hipDeviceProp_t props;
        err = hipGetDeviceProperties(&props, i);
        if (err != hipSuccess) {
            printf("ERROR: hipGetDeviceProperties failed for device %d: %d\n", i, err);
            continue;
        }

        printf("=== Device %d ===\n", i);
        printf("Name:              %s\n", props.name);
        printf("Architecture:      %s\n", props.gcnArchName);
        printf("Total Memory:      %.2f GB\n", props.totalGlobalMem / (1024.0 * 1024.0 * 1024.0));
        printf("Compute Units:     %d\n", props.multiProcessorCount);
        printf("Max Threads/Block: %d\n", props.maxThreadsPerBlock);
        printf("Warp Size:         %d\n", props.warpSize);
        printf("Clock Rate:        %d MHz\n", props.clockRate / 1000);
        printf("Memory Clock:      %d MHz\n", props.memoryClockRate / 1000);
        printf("Memory Bus Width:  %d-bit\n", props.memoryBusWidth);
        printf("Shared Mem/Block:  %zu bytes\n", props.sharedMemPerBlock);
        printf("L2 Cache Size:     %d bytes\n", props.l2CacheSize);
        printf("\n");
    }

    // Test memory allocation
    printf("=== Testing Memory Allocation ===\n");
    size_t freeMem, totalMem;
    err = hipMemGetInfo(&freeMem, &totalMem);
    if (err != hipSuccess) {
        printf("ERROR: hipMemGetInfo failed: %d\n", err);
    } else {
        printf("Free Memory:  %.2f GB\n", freeMem / (1024.0 * 1024.0 * 1024.0));
        printf("Total Memory: %.2f GB\n\n", totalMem / (1024.0 * 1024.0 * 1024.0));
    }

    // Test small allocation
    void* d_ptr1 = nullptr;
    size_t size1 = 100 * 1024 * 1024; // 100 MB
    printf("Allocating 100 MB... ");
    err = hipMalloc(&d_ptr1, size1);
    if (err == hipSuccess) {
        printf("SUCCESS (ptr=%p)\n", d_ptr1);
        hipFree(d_ptr1);
    } else {
        printf("FAILED (%d)\n", err);
    }

    // Test large allocation
    void* d_ptr2 = nullptr;
    size_t size2 = 1024 * 1024 * 1024; // 1 GB
    printf("Allocating 1 GB...  ");
    err = hipMalloc(&d_ptr2, size2);
    if (err == hipSuccess) {
        printf("SUCCESS (ptr=%p)\n", d_ptr2);
        hipFree(d_ptr2);
    } else {
        printf("FAILED (%d)\n", err);
    }

    printf("\n==============================================\n");
    printf("All tests completed!\n");
    printf("==============================================\n");

    return 0;
}
