#pragma once

namespace Core {

    class BotDiscord {
    private:
        void* hOut;
        void* hIn;
        bool isRunning;

    public:
        BotDiscord();
        ~BotDiscord() = default;

        void start();
        void stop(int exitCode);
    };

} // namespace Core
