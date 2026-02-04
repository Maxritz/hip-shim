#include "hip_shim_common.hpp"
#include <Windows.h>

namespace hip_shim
{

    static HMODULE g_realRocblas = nullptr;

    static bool initialize_rocblas()
    {
        if (g_realRocblas)
            return true;

        // Try to load real rocblas DLL
        const char *hip_path = getenv("HIP_PATH");
        if (hip_path)
        {
            char dll_path[MAX_PATH];
            snprintf(dll_path, MAX_PATH, "%s\\bin\\rocblas_real.dll", hip_path);
            g_realRocblas = LoadLibraryA(dll_path);
        }

        if (!g_realRocblas)
        {
            g_realRocblas = LoadLibraryA("rocblas_real.dll");
        }

        if (!g_realRocblas)
        {
            LOG_WARNING("ROCBlas wrapper: real DLL not found, will passthrough");
            return false;
        }

        LOG_INFO("ROCBlas wrapper initialized");
        return true;
    }

    static void shutdown_rocblas()
    {
        if (g_realRocblas)
        {
            FreeLibrary(g_realRocblas);
            g_realRocblas = nullptr;
        }
    }

} // namespace hip_shim

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        hip_shim::initialize_rocblas();
        break;
    case DLL_PROCESS_DETACH:
        hip_shim::shutdown_rocblas();
        break;
    }
    return TRUE;
}
