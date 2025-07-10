#pragma once

#include <iostream>
#include <fstream>
#include <string>

/**
 * @brief Clase Debug para gestionar mensajes de depuración y registro.
 */
class Debug {
public:
    /**
     * @brief Enumeración que define los niveles de registro.
     */
    enum class LogLevel {
        Log,     /**< Nivel de registro informativo. */
        Warning, /**< Nivel de registro de advertencia. */
        Error    /**< Nivel de registro de error. */
    };

    /**
     * @brief Registra un mensaje informativo.
     * @param message Mensaje a registrar.
     */
    static void Log(const std::string& message);

    /**
     * @brief Registra un mensaje de advertencia.
     * @param message Mensaje a registrar.
     */
    static void Warning(const std::string& message);

    /**
     * @brief Registra un mensaje de error.
     * @param message Mensaje a registrar.
     */
    static void Error(const std::string& message);

    /**
     * @brief Habilita el registro en un archivo.
     * @param filename Nombre del archivo donde se guardarán los registros.
     */
    static void EnableFileLogging(const std::string& filename);

    /**
     * @brief Deshabilita el registro en archivo.
     */
    static void DisableFileLogging();

private:
    /**
     * @brief Escribe un mensaje de registro con el nivel especificado.
     * @param message Mensaje a escribir.
     * @param level Nivel de registro (Log, Warning o Error).
     */
    static void WriteLog(const std::string& message, LogLevel level);

    static std::ofstream logFile; /**< Flujo de salida para el archivo de registro. */
};