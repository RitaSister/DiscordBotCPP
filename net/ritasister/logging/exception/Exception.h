#pragma once

namespace Core {
    namespace ErrorSystem {

        // Перечисление кодов ошибок для нашего приложения
        enum class ErrorCode {
            OK = 0,
            NetworkInitFailed,
            SocketOpenFailed,
            SocketConnectFailed,
            ConfigLoadFailed,
            UnknownError
        };

        class Exception {
        private:
            ErrorCode code;
            const char* message;

        public:
            constexpr Exception(ErrorCode errCode, const char* errMessage)
                : code(errCode), message(errMessage) {}

            constexpr ErrorCode getCode() const {
                return code;
            }

            constexpr const char* getMessage() const {
                return message;
            }
        };

    } // namespace ErrorSystem
} // namespace Core
