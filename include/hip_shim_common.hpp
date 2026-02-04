#ifndef HIP_SHIM_COMMON_HPP
#define HIP_SHIM_COMMON_HPP

#include <Windows.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <mutex>
#include <unordered_map>
#include <atomic>
#include <vector>

#define HIP_SHIM_VERSION_MAJOR 1
#define HIP_SHIM_VERSION_MINOR 0

namespace hip_shim
{
    namespace config
    {
        // Default values (can be overridden by environment variables)
        constexpr size_t VRAM_HEADROOM_PERCENT = 10;
        constexpr size_t MIN_SYSTEM_RAM_RESERVE_MB = 2048;
        constexpr size_t ALLOCATION_ALIGNMENT = 256;
        constexpr bool ENABLE_MEMORY_SPILLOVER = true;
        constexpr bool ENABLE_DEBUG_LOGGING = true;
        constexpr const char *REAL_HIP_DLL_NAME = "amdhip64_7_real.dll";
        constexpr const char *REAL_ROCBLAS_DLL_NAME = "rocblas_real.dll";
        constexpr const char *LOG_FILE_PATH = "hip_shim.log";

        // Runtime config helpers (check env vars first, then use defaults)
        inline const char* get_real_hip_dll_name() {
            const char* env = getenv("HIP_SHIM_REAL_DLL");
            return env ? env : REAL_HIP_DLL_NAME;
        }

        inline const char* get_real_rocblas_dll_name() {
            const char* env = getenv("HIP_SHIM_ROCBLAS_DLL");
            return env ? env : REAL_ROCBLAS_DLL_NAME;
        }

        inline const char* get_log_file_path() {
            const char* env = getenv("HIP_SHIM_LOG_FILE");
            return env ? env : LOG_FILE_PATH;
        }

        inline bool get_debug_logging_enabled() {
            const char* env = getenv("HIP_SHIM_DEBUG");
            return env ? (strcmp(env, "1") == 0 || strcmp(env, "true") == 0) : ENABLE_DEBUG_LOGGING;
        }

        inline size_t get_vram_headroom_percent() {
            const char* env = getenv("HIP_SHIM_VRAM_HEADROOM");
            return env ? static_cast<size_t>(atoi(env)) : VRAM_HEADROOM_PERCENT;
        }
    }

    enum class LogLevel
    {
        Error = 0,
        Warning = 1,
        Info = 2,
        Debug = 3,
        Verbose = 4
    };

    class Logger
    {
    public:
        static Logger &instance()
        {
            static Logger inst;
            return inst;
        }

        void set_level(LogLevel level) { level_ = level; }

        void enable_file_logging(const char *path = nullptr)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (log_file_)
            {
                fclose(log_file_);
            }
            const char* log_path = path ? path : config::get_log_file_path();
            fopen_s(&log_file_, log_path, "w");
        }

        template <typename... Args>
        void log(LogLevel level, const char *format, Args... args)
        {
            if (level > level_)
                return;
            std::lock_guard<std::mutex> lock(mutex_);

            const char *level_str[] = {"ERROR", "WARN", "INFO", "DEBUG", "VERBOSE"};
            char buffer[1024];
            snprintf(buffer, sizeof(buffer), "[HIP-SHIM][%s] ", level_str[static_cast<int>(level)]);

            size_t prefix_len = strlen(buffer);
            snprintf(buffer + prefix_len, sizeof(buffer) - prefix_len, format, args...);

            if (log_file_)
            {
                fprintf(log_file_, "%s\n", buffer);
                fflush(log_file_);
            }

            if (level <= LogLevel::Warning)
            {
                fprintf(stderr, "%s\n", buffer);
            }
        }

        ~Logger()
        {
            if (log_file_)
                fclose(log_file_);
        }

    private:
        Logger() : level_(config::get_debug_logging_enabled() ? LogLevel::Debug : LogLevel::Info), log_file_(nullptr) {}
        LogLevel level_;
        FILE *log_file_;
        std::mutex mutex_;
    };

#define LOG_ERROR(fmt, ...) hip_shim::Logger::instance().log(hip_shim::LogLevel::Error, fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...) hip_shim::Logger::instance().log(hip_shim::LogLevel::Warning, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) hip_shim::Logger::instance().log(hip_shim::LogLevel::Info, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) hip_shim::Logger::instance().log(hip_shim::LogLevel::Debug, fmt, ##__VA_ARGS__)

    inline size_t get_available_system_ram()
    {
        MEMORYSTATUSEX status;
        status.dwLength = sizeof(status);
        if (GlobalMemoryStatusEx(&status))
        {
            return static_cast<size_t>(status.ullAvailPhys);
        }
        return 0;
    }

    inline size_t get_total_system_ram()
    {
        MEMORYSTATUSEX status;
        status.dwLength = sizeof(status);
        if (GlobalMemoryStatusEx(&status))
        {
            return static_cast<size_t>(status.ullTotalPhys);
        }
        return 0;
    }

} // namespace hip_shim
#endif
