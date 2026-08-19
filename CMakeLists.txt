cmake_minimum_required(VERSION 3.28)
project(DiscordBotCPP)

set(CMAKE_CXX_STANDARD 20)

add_executable(DiscordBotCPP
        net/ritasister/Main.cpp
        net/ritasister/BotDiscord.cpp
        net/ritasister/BotDiscord.h
        net/ritasister/console/Console.cpp
        net/ritasister/console/Console.h
        net/ritasister/network/Network.cpp
        net/ritasister/network/Network.h
        net/ritasister/config/Config.cpp
        net/ritasister/config/Config.h
        net/ritasister/scheduler/Scheduler.cpp
        net/ritasister/scheduler/Scheduler.h
        net/ritasister/logging/debug/Debug.cpp
        net/ritasister/logging/debug/Debug.h
        net/ritasister/util/TimeUtils.cpp
        net/ritasister/util/TimeUtils.h
        net/ritasister/logging/LogArchiver.h
        net/ritasister/logging/LogArchiver.cpp
)

set_source_files_properties(
        net/ritasister/Main.cpp
        net/ritasister/BotDiscord.cpp
        net/ritasister/console/Console.cpp
        net/ritasister/network/Network.cpp
        net/ritasister/config/Config.cpp
        net/ritasister/scheduler/Scheduler.cpp
        net/ritasister/logging/debug/Debug.cpp
        net/ritasister/util/TimeUtils.cpp
        net/ritasister/logging/LogArchiver.cpp
        PROPERTIES CXX_SCAN_FOR_MODULES OFF
)

target_link_options(DiscordBotCPP PRIVATE -nostdlib -mconsole)

target_link_libraries(DiscordBotCPP PRIVATE kernel32)
if(WIN32)
    target_link_libraries(DiscordBotCPP PRIVATE ws2_32)
endif()
