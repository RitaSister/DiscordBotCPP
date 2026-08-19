cmake_minimum_required(VERSION 4.3)
project(DiscordBotCPP)

set(CMAKE_CXX_STANDARD 26)

add_executable(
        DiscordBotCPP
        Main.cpp
)

# Добавьте эти флаги для вашего исполняемого файла в CMakeLists.txt:
target_link_options(DiscordBotCPP PRIVATE -nostdlib -mconsole -lkernel32)
