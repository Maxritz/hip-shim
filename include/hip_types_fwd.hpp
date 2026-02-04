#ifndef HIP_TYPES_FWD_HPP
#define HIP_TYPES_FWD_HPP

// This file provides forward declarations for HIP types
// DO NOT include this if you're including hip_runtime_api.h or hip_runtime.h

// Guard to prevent conflicts with real HIP headers
// Only define forward declarations if the real headers haven't been included
#ifndef HIP_INCLUDE_HIP_HIP_RUNTIME_API_H
#ifndef HIP_INCLUDE_HIP_HIP_RUNTIME_H

// Forward declarations and minimal types needed from HIP
// This avoids including the full hip_runtime.h which causes redefinition errors

#ifdef __cplusplus
extern "C"
{
#endif

    // Basic types
    typedef int hipError_t;
    typedef struct ihipCtx_t *hipCtx_t;
    typedef struct ihipDevice_t *hipDevice_t;
    typedef struct ihipStream_t *hipStream_t;
    typedef struct ihipModule_t *hipModule_t;
    typedef struct ihipModuleSymbol_t *hipFunction_t;
    typedef struct ihipEvent_t *hipEvent_t;

    // Enums
    typedef enum hipMemcpyKind
    {
        hipMemcpyHostToHost = 0,
        hipMemcpyHostToDevice = 1,
        hipMemcpyDeviceToHost = 2,
        hipMemcpyDeviceToDevice = 3,
        hipMemcpyDefault = 4
    } hipMemcpyKind;

    typedef enum hipDeviceAttribute_t
    {
        hipDeviceAttributeMaxThreadsPerBlock = 1,
        hipDeviceAttributeMaxBlockDimX = 2,
        hipDeviceAttributeMaxBlockDimY = 3,
        hipDeviceAttributeMaxBlockDimZ = 4,
        hipDeviceAttributeMaxGridDimX = 5,
        hipDeviceAttributeMaxGridDimY = 6,
        hipDeviceAttributeMaxGridDimZ = 7,
        hipDeviceAttributeMaxSharedMemoryPerBlock = 8,
        hipDeviceAttributeTotalConstantMemory = 9,
        hipDeviceAttributeWarpSize = 10,
        hipDeviceAttributeMaxRegistersPerBlock = 12,
        hipDeviceAttributeClockRate = 13,
        hipDeviceAttributeMemoryClockRate = 36,
        hipDeviceAttributeMemoryBusWidth = 37,
        hipDeviceAttributeMultiprocessorCount = 16,
        hipDeviceAttributeComputeMode = 20,
        hipDeviceAttributeL2CacheSize = 38,
        hipDeviceAttributeMaxThreadsPerMultiProcessor = 39,
        hipDeviceAttributeComputeCapabilityMajor = 75,
        hipDeviceAttributeComputeCapabilityMinor = 76,
        hipDeviceAttributePciBusId = 33,
        hipDeviceAttributePciDeviceId = 34,
        hipDeviceAttributeMaxSharedMemoryPerMultiprocessor = 81,
        hipDeviceAttributeIsMultiGpuBoard = 84,
        hipDeviceAttributeIntegrated = 83,
        hipDeviceAttributeCooperativeLaunch = 95,
        hipDeviceAttributeCooperativeMultiDeviceLaunch = 96,
        hipDeviceAttributeConcurrentKernels = 31,
        hipDeviceAttributePciDomainId = 50,
        hipDeviceAttributeMaxBlocksPerMultiProcessor = 106,
        hipDeviceAttributeMemoryPoolsSupported = 115
    } hipDeviceAttribute_t;

    typedef enum hipComputeMode
    {
        hipComputeModeDefault = 0,
        hipComputeModeExclusive = 1,
        hipComputeModeProhibited = 2,
        hipComputeModeExclusiveProcess = 3
    } hipComputeMode;

    // Structures
    struct dim3
    {
        unsigned int x, y, z;
#ifdef __cplusplus
        dim3(unsigned int vx = 1, unsigned int vy = 1, unsigned int vz = 1) : x(vx), y(vy), z(vz) {}
#endif
    };

    typedef struct hipDeviceProp_t
    {
        char name[256];
        char gcnArchName[256];
        size_t totalGlobalMem;
        size_t sharedMemPerBlock;
        int regsPerBlock;
        int warpSize;
        int maxThreadsPerBlock;
        int maxThreadsDim[3];
        int maxGridSize[3];
        int clockRate;
        int memoryClockRate;
        int memoryBusWidth;
        size_t totalConstMem;
        int major;
        int minor;
        int multiProcessorCount;
        int l2CacheSize;
        int maxThreadsPerMultiProcessor;
        int computeMode;
        int clockInstructionRate;
        int arch;
        int concurrentKernels;
        int pciDomainID;
        int pciBusID;
        int pciDeviceID;
        size_t maxSharedMemoryPerMultiProcessor;
        int isMultiGpuBoard;
        int canMapHostMemory;
        int gcnArch;
        int integrated;
        int cooperativeLaunch;
        int cooperativeMultiDeviceLaunch;
        int maxTexture1D;
        int maxTexture2D[2];
        int maxTexture3D[3];
        unsigned int *hdpMemFlushCntl;
        unsigned int *hdpRegFlushCntl;
        size_t memPitch;
        size_t textureAlignment;
        size_t texturePitchAlignment;
        int kernelExecTimeoutEnabled;
        int ECCEnabled;
        int tccDriver;
        int cooperativeMultiDeviceUnmatchedFunc;
        int cooperativeMultiDeviceUnmatchedGridDim;
        int cooperativeMultiDeviceUnmatchedBlockDim;
        int cooperativeMultiDeviceUnmatchedSharedMem;
        int isLargeBar;
        int asicRevision;
        int managedMemory;
        int directManagedMemAccessFromHost;
        int concurrentManagedAccess;
        int pageableMemoryAccess;
        int pageableMemoryAccessUsesHostPageTables;
    } hipDeviceProp_t;

    typedef enum hipMemoryType
    {
        hipMemoryTypeHost = 0,
        hipMemoryTypeDevice = 1,
        hipMemoryTypeArray = 2,
        hipMemoryTypeUnified = 3
    } hipMemoryType;

    typedef struct hipPointerAttribute_t
    {
        hipMemoryType type;
        int device;
        void *devicePointer;
        void *hostPointer;
        int isManaged;
        unsigned allocationFlags;
    } hipPointerAttribute_t;

// Error codes
#define hipSuccess 0
#define hipErrorInvalidValue 1
#define hipErrorOutOfMemory 2
#define hipErrorNotInitialized 3
#define hipErrorNotSupported 101

// Host registration flags
#define hipHostRegisterDefault 0x0
#define hipHostRegisterPortable 0x1
#define hipHostRegisterMapped 0x2

#ifdef __cplusplus
}
#endif

#endif // HIP_INCLUDE_HIP_HIP_RUNTIME_H
#endif // HIP_INCLUDE_HIP_HIP_RUNTIME_API_H

#endif // HIP_TYPES_FWD_HPP
