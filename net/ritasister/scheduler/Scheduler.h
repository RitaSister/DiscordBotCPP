#pragma once

namespace SchedulerSystem {

    // Получить текущее системное время в миллисекундах (кросс-платформенно)
    unsigned long long getSystemTimeMs();

    // Функция паузы (сна) потока на заданное количество миллисекунд
    void systemSleep(unsigned long ms);

    /**
     * Класс или структура для управления отдельной периодической задачей.
     */
    struct Task {
        unsigned long long lastExecutionTime; // Время последнего запуска
        unsigned long long intervalMs;        // Интервал срабатывания в мс
        bool enabled;                         // Включена ли задача

        // Конструктор по умолчанию
        Task(unsigned long long interval = 1000, bool startEnabled = true)
            : lastExecutionTime(getSystemTimeMs()), intervalMs(interval), enabled(startEnabled) {}

        // Установка нового интервала
        void setInterval(unsigned long long interval) {
            intervalMs = interval;
        }

        // Проверяет, пришло ли время выполнить задачу
        bool check(unsigned long long currentTime) {
            if (!enabled) return false;

            if (currentTime - lastExecutionTime >= intervalMs) {
                lastExecutionTime = currentTime;
                return true;
            }
            return false;
        }
    };

} // namespace SchedulerSystem
