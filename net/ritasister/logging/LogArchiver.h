#pragma once

namespace LogSystem {

    class LogArchiver {
    public:
        // Архивация предыдущего файла latest.log (переименование/перенос в архив)
        static void archiveOldLog();
    };

} // namespace LogSystem
