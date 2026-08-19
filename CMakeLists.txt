cmake_minimum_required(VERSION 3.28)
project(DiscordBotCPP)

set(CMAKE_CXX_STANDARD 20)

# Отключаем исключения и RTTI, чтобы GCC не генерировал __gxx_personality_seh0 и _Unwind_Resume
add_compile_options(-fno-exceptions -fno-rtti)

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
        net/ritasister/network/socket/Socket.cpp
        net/ritasister/network/socket/Socket.h
        net/ritasister/logging/exception/Exception.cpp
        net/ritasister/logging/exception/Exception.h
        net/ritasister/discord/DiscordClient.cpp
        net/ritasister/discord/DiscordClient.h
        net/ritasister/network/http/server/HttpServer.cpp
        net/ritasister/network/http/server/HttpServer.h
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
        net/ritasister/logging/exception/Exception.cpp
        net/ritasister/network/socket/Socket.cpp
        net/ritasister/network/http/server/HttpServer.cpp
        net/ritasister/discord/DiscordClient.cpp
        PROPERTIES CXX_SCAN_FOR_MODULES OFF
)

target_link_options(DiscordBotCPP PRIVATE -nostdlib)

# 1. Убираем стандартные библиотеки, которые линковщик пытается воткнуть автоматически,
# и добавляем только те, что нужны для нашего "голого" билда.
set_target_properties(DiscordBotCPP PROPERTIES LINK_FLAGS "-nostdlib -Wl,-e,_discord_bot_entry")

if(WIN32)
    # Добавляем ws2_32 (для сети) и kernel32 (обязательно для WinAPI)
    target_link_libraries(DiscordBotCPP PRIVATE kernel32 ws2_32)
    target_link_options(DiscordBotCPP PRIVATE -mconsole)
elseif(UNIX)
    # В Linux ws2_32 не нужен, там сокеты встроены в системные вызовы
    target_link_options(DiscordBotCPP PRIVATE -Wl,-e,_kernel_entry)
endif()
