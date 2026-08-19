cmake_minimum_required(VERSION 3.28)
project(DiscordBotCPP)

set(CMAKE_CXX_STANDARD 20)

add_executable(DiscordBotCPP
        net/ritasister/Main.cpp
        net/ritasister/console/Console.cpp
        net/ritasister/console/Console.h
        net/ritasister/network/Network.cpp
        net/ritasister/network/Network.h
        net/ritasister/config/Config.cpp
        net/ritasister/config/Config.h
        net/ritasister/scheduler/Scheduler.cpp
        net/ritasister/scheduler/Scheduler.h
        net/ritasister/logging/debug/Debug.cpp  # Учитываем реальную папку logging
        net/ritasister/logging/debug/Debug.h    # Учитываем реальную папку logging
)

set_source_files_properties(
        net/ritasister/Main.cpp
        net/ritasister/console/Console.cpp
        net/ritasister/network/Network.cpp
        net/ritasister/config/Config.cpp
        net/ritasister/scheduler/Scheduler.cpp
        net/ritasister/logging/debug/Debug.cpp  # Не забываем про отключение сканирования для нового файла
        PROPERTIES CXX_SCAN_FOR_MODULES OFF
)

target_link_options(DiscordBotCPP PRIVATE -nostdlib -mconsole)

target_link_libraries(DiscordBotCPP PRIVATE kernel32)
if(WIN32)
    target_link_libraries(DiscordBotCPP PRIVATE ws2_32)
endif()
