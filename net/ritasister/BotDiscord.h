#pragma once

namespace Core {

    class BotDiscord {
    private:
        void* hOut;
        void* hIn;
        bool isRunning;
        int numberOfThreads;

    public:
        BotDiscord();
        ~BotDiscord() = default;

        void start();
        void stop(int exitCode);
    };

} // namespace Core
