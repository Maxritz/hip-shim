// Simple test program to verify HIP shim functionality
#include <hip/hip_runtime.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECK_HIP(call)                                                                               \
    do                                                                                                \
    {                                                                                                 \
        hipError_t err = call;                                                                        \
        if (err != hipSuccess)                                                                        \
        {                                                                                             \
            fprintf(stderr, "HIP Error at %s:%d - %s\n", __FILE__, __LINE__, hipGetErrorString(err)); \
            exit(1);                                                                                  \
        }                                                                                             \
    } while (0)

void print_device_info(int device)
{
    hipDeviceProp_t props;
    memset(&props, 0, sizeof(props)); // Zero out first to see what gets filled
    hipError_t err = hipGetDeviceProperties(&props, device);

    printf("\n=== Device %d Information ===\n", device);
    printf("hipGetDeviceProperties returned: %d (%s)\n", err, hipGetErrorString(err));
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
    printf("Regs/Block:        %d\n", props.regsPerBlock);

    if (err != hipSuccess)
    {
        printf("ERROR: Failed to get device properties!\n");
    }
}

void test_memory_allocation()
{
    printf("\n=== Testing Memory Allocation ===\n");

    size_t free_mem, total_mem;
    CHECK_HIP(hipMemGetInfo(&free_mem, &total_mem));
    printf("Free Memory:  %.2f GB\n", free_mem / (1024.0 * 1024.0 * 1024.0));
    printf("Total Memory: %.2f GB\n", total_mem / (1024.0 * 1024.0 * 1024.0));

    // Test small allocation (should be in VRAM)
    printf("\nAllocating 100 MB...\n");
    void *ptr1 = nullptr;
    CHECK_HIP(hipMalloc(&ptr1, 100 * 1024 * 1024));
    printf("✓ Small allocation successful (ptr=%p)\n", ptr1);

    // Test large allocation (1GB)
    printf("\nAllocating 1 GB...\n");
    void *ptr2 = nullptr;
    CHECK_HIP(hipMalloc(&ptr2, 1024 * 1024 * 1024));
    printf("✓ Large allocation successful (ptr=%p)\n", ptr2);

    // Test memory copy
    printf("\nTesting memory operations...\n");
    size_t size = 1024 * 1024; // 1MB
    char *host_data = (char *)malloc(size);
    for (size_t i = 0; i < size; i++)
    {
        host_data[i] = (char)(i % 256);
    }

    void *device_data = nullptr;
    CHECK_HIP(hipMalloc(&device_data, size));
    CHECK_HIP(hipMemcpy(device_data, host_data, size, hipMemcpyHostToDevice));
    printf("✓ Host to Device copy successful\n");

    char *verify_data = (char *)malloc(size);
    CHECK_HIP(hipMemcpy(verify_data, device_data, size, hipMemcpyDeviceToHost));
    printf("✓ Device to Host copy successful\n");

    // Verify data
    bool match = true;
    for (size_t i = 0; i < size; i++)
    {
        if (host_data[i] != verify_data[i])
        {
            match = false;
            break;
        }
    }
    printf("✓ Data verification: %s\n", match ? "PASSED" : "FAILED");

    // Cleanup
    CHECK_HIP(hipFree(ptr1));
    CHECK_HIP(hipFree(ptr2));
    CHECK_HIP(hipFree(device_data));
    free(host_data);
    free(verify_data);
    printf("✓ Memory freed successfully\n");
}

void test_streams()
{
    printf("\n=== Testing Stream Operations ===\n");

    hipStream_t stream;
    CHECK_HIP(hipStreamCreate(&stream));
    printf("✓ Stream created\n");

    CHECK_HIP(hipStreamSynchronize(stream));
    printf("✓ Stream synchronized\n");

    CHECK_HIP(hipStreamDestroy(stream));
    printf("✓ Stream destroyed\n");
}

void test_events()
{
    printf("\n=== Testing Event Operations ===\n");

    hipEvent_t event;
    CHECK_HIP(hipEventCreate(&event));
    printf("✓ Event created\n");

    CHECK_HIP(hipEventRecord(event, 0));
    printf("✓ Event recorded\n");

    CHECK_HIP(hipEventSynchronize(event));
    printf("✓ Event synchronized\n");

    CHECK_HIP(hipEventDestroy(event));
    printf("✓ Event destroyed\n");
}

int main()
{
    printf("==================================================\n");
    printf("HIP Shim Test Program\n");
    printf("==================================================\n");

    // Check device count
    int device_count = 0;
    CHECK_HIP(hipGetDeviceCount(&device_count));
    printf("\nFound %d HIP device(s)\n", device_count);

    if (device_count == 0)
    {
        printf("No HIP devices found!\n");
        return 1;
    }

    // Get and print info for first device
    int device = 0;
    CHECK_HIP(hipSetDevice(device));
    print_device_info(device);

    // Run tests
    test_memory_allocation();
    test_streams();
    test_events();

    // Final synchronization
    CHECK_HIP(hipDeviceSynchronize());

    printf("\n==================================================\n");
    printf("All tests PASSED! ✓\n");
    printf("==================================================\n");
    printf("\nCheck 'hip_shim.log' for detailed shim operation logs\n");

    return 0;
}
