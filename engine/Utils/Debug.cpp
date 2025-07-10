#include "Debug.h"
#include <iostream>

std::ofstream Debug::logFile;

void Debug::Log(const std::string& message) {
    WriteLog("[LOG] " + message, LogLevel::Log);
}

void Debug::Warning(const std::string& message) {
    WriteLog("[WARNING] " + message, LogLevel::Warning);
}

void Debug::Error(const std::string& message) {
    WriteLog("[ERROR] " + message, LogLevel::Error);
}

void Debug::EnableFileLogging(const std::string& filename) {
    std::string logFilePath = "logs/" + filename;

    logFile.open(logFilePath, std::ios::out | std::ios::app);
    if (logFile.is_open()) {
        Log("File logging enabled: " + logFilePath);
    } else {
        std::cerr << "[DEBUG ERROR] Failed to open log file: " + logFilePath << std::endl;
    }
}

void Debug::DisableFileLogging() {
    if (logFile.is_open()) {
        Log("File logging disabled.");
        logFile.close();
    }
}

void Debug::WriteLog(const std::string& message, LogLevel level) {
    std::cout << message << std::endl;

    if (logFile.is_open()) {
        logFile << message << std::endl;
    }

    if (level == LogLevel::Error) {
        throw std::runtime_error(message);
    }
}
