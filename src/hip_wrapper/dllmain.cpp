#include "hip_shim_common.hpp"
#include "function_table.hpp"
#include "memory_manager.hpp"
#include "device_spoof.hpp"
#include <cstring>

namespace hip_shim
{

    HipFunctionTable g_realHip = {};
    static bool g_initialized = false;

    static bool initialize_shim()
    {
        if (g_initialized)
            return true;

        Logger::instance().enable_file_logging();
        LOG_INFO("HIP-Shim v%d.%d initializing...", HIP_SHIM_VERSION_MAJOR, HIP_SHIM_VERSION_MINOR);

        // Find real HIP DLL
        char dll_path[MAX_PATH] = {0};
        HMODULE hSelf = nullptr;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           reinterpret_cast<LPCSTR>(&initialize_shim), &hSelf);

        if (GetModuleFileNameA(hSelf, dll_path, MAX_PATH) > 0)
        {
            char *last_slash = strrchr(dll_path, '\\');
            if (last_slash)
            {
                *(last_slash + 1) = '\0';
                strcat_s(dll_path, MAX_PATH, config::REAL_HIP_DLL_NAME);
            }
        }

        if (!g_realHip.load(dll_path))
        {
            // Try default location
            const char *hip_path = getenv("HIP_PATH");
            if (hip_path)
            {
                char fallback_path[MAX_PATH];
                // Try versioned DLL first (e.g., amdhip64_7.dll)
                snprintf(fallback_path, MAX_PATH, "%s\\bin\\amdhip64_7.dll", hip_path);
                if (!g_realHip.load(fallback_path))
                {
                    // Try non-versioned as fallback
                    snprintf(fallback_path, MAX_PATH, "%s\\bin\\amdhip64.dll", hip_path);
                    if (!g_realHip.load(fallback_path))
                    {
                        LOG_ERROR("Failed to load real HIP runtime");
                        return false;
                    }
                }
            }
            else
            {
                LOG_ERROR("HIP_PATH not set and real DLL not found");
                return false;
            }
        }

        // Initialize device spoof
        if (!DeviceSpoof::instance().initialize(0))
        {
            LOG_ERROR("Failed to initialize device spoofer");
            return false;
        }

        // Initialize memory manager
        size_t vram_size = DeviceSpoof::instance().get_real_vram();
        if (!MemoryManager::instance().initialize(vram_size))
        {
            LOG_ERROR("Failed to initialize memory manager");
            return false;
        }

        LOG_INFO("HIP-Shim initialization complete");
        g_initialized = true;
        return true;
    }

    static void shutdown_shim()
    {
        if (!g_initialized)
            return;
        LOG_INFO("HIP-Shim shutting down...");
        MemoryManager::instance().shutdown();
        g_realHip.unload();
        g_initialized = false;
    }

} // namespace hip_shim

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        return hip_shim::initialize_shim() ? TRUE : FALSE;
    case DLL_PROCESS_DETACH:
        hip_shim::shutdown_shim();
        break;
    }
    return TRUE;
}
