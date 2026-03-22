/*
Copyright (C) 2016,2018,2019,2025,2026 Rodrigo Jose Hernandez Cordoba

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
/** @file LogLevel.hpp
 *  @brief Defines log severity levels and stream output for the AeonGames engine.
 */
#ifndef AEONGAMES_LOGLEVEL_H
#define AEONGAMES_LOGLEVEL_H
#include "aeongames/Platform.hpp"
#include <iostream>

namespace AeonGames
{
    /** @brief Severity levels for engine log messages. */
    enum class LogLevel
    {
        Debug = 0,       /**< @brief Detailed diagnostic information. */
        Info,            /**< @brief General informational messages. */
        Performance,     /**< @brief Performance-related metrics. */
        Warning,         /**< @brief Potential issues that may need attention. */
        Error            /**< @brief Error conditions. */
    };
    /** @brief Inserts a color-coded log level tag into an output stream.
     *  @param os The output stream.
     *  @param level The log level to format.
     *  @return Reference to the output stream.
     */
    inline std::ostream& operator<< ( std::ostream& os, LogLevel level )
    {
        switch ( level )
        {
        case LogLevel::Debug:
            os << "\x1B[34m" << "[DEBUG]" << "\x1B[m ";
            break;
        case LogLevel::Info:
            os << "\x1B[32m" << "[INFO]" << "\x1B[m ";
            break;
        case LogLevel::Performance:
            os << "\x1B[35m" << "[PERFORMANCE]" << "\x1B[m ";
            break;
        case LogLevel::Warning:
            os << "\x1B[33m" << "[WARNING]" << "\x1B[m ";
            break;
        case LogLevel::Error:
            os << "\x1B[31m" << "[ERROR]" << "\x1B[m ";
            break;
        }
        return os;
    }
}
#endif
