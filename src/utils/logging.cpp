#include "logging.h"

#include <cstdlib>
#include <string>
#include <vector>

namespace
{

spdlog::level::level_enum defaultLevel()
{
#ifndef NDEBUG
    return spdlog::level::debug;
#else
    return spdlog::level::info;
#endif
}

bool isVerbosityFlag(const std::string& arg)
{
    return arg.size() >= 2 && arg[0] == '-' && arg.find_first_not_of('v', 1) == std::string::npos;
}

}  // namespace

namespace Log
{

void init(int& argc, char** argv)
{
    spdlog::level::level_enum level = defaultLevel();
    std::vector<char*> remaining;
    remaining.push_back(argv[0]);

    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];

        if (isVerbosityFlag(arg))
        {
            const std::size_t vCount = arg.size() - 1;
            level = (vCount >= 2) ? spdlog::level::trace : spdlog::level::debug;
            continue;
        }

        if (arg == "-D" && i + 1 < argc)
        {
            const int n = std::atoi(argv[++i]);
            if (n >= spdlog::level::trace && n <= spdlog::level::off)
            {
                level = static_cast<spdlog::level::level_enum>(n);
            }
            continue;
        }

        remaining.push_back(argv[i]);
    }

    argc = static_cast<int>(remaining.size());
    for (std::size_t i = 0; i < remaining.size(); ++i)
    {
        argv[i] = remaining[i];
    }

    spdlog::set_level(level);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
}

}  // namespace Log
