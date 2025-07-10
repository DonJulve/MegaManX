// AudioManager.h
#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <map>
#include <memory>

/**
 * @brief Gestor de audio para manejo de efectos de sonido y música.
 * 
 * AudioManager implementa el patrón singleton para proporcionar acceso centralizado
 * a las funcionalidades de carga y reproducción de audio utilizando SDL_mixer.
 */
class AudioManager {
private:
    AudioManager();
    ~AudioManager();
    
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;
    
    static AudioManager* instance;
    
    std::map<std::string, Mix_Chunk*> soundEffects;
    std::map<std::string, Mix_Music*> music;
    
    int musicVolume;
    int soundVolume;
    
    std::string currentMusicTrack;

public:
    /**
     * @brief Obtiene la instancia única de AudioManager
     * @return Puntero a la instancia de AudioManager
     */
    static AudioManager* getInstance();
    
    /**
     * @brief Carga un efecto de sonido desde un archivo
     * @param id Identificador único para el efecto de sonido
     * @param filePath Ruta del archivo de sonido
     * @return true si la carga fue exitosa, false en caso contrario
     */
    bool loadSound(const std::string& id, const std::string& filePath);
    
    /**
     * @brief Carga una pista de música desde un archivo
     * @param id Identificador único para la pista de música
     * @param filePath Ruta del archivo de música
     * @return true si la carga fue exitosa, false en caso contrario
     */
    bool loadMusic(const std::string& id, const std::string& filePath);
    
    /**
     * @brief Reproduce un efecto de sonido
     * @param id Identificador del efecto de sonido a reproducir
     * @param loops Número de repeticiones (-1 para infinito)
     * @param channel Canal en el que reproducir (-1 para automático)
     * @return Identificador del canal de reproducción o -1 si hubo un error
     */
    int playSound(const std::string& id, int loops = 0, int channel = -1);
    
    /**
     * @brief Reproduce una pista de música
     * @param id Identificador de la pista de música
     * @param loops Número de repeticiones (-1 para infinito)
     * @param fadeInMs Duración del fade in en milisegundos
     * @return true si la reproducción fue exitosa, false en caso contrario
     */
    bool playMusic(const std::string& id, int loops = -1, int fadeInMs = 0);
    
    /**
     * @brief Pausa la reproducción de música
     */
    void pauseMusic();
    
    /**
     * @brief Reanuda la reproducción de música pausada
     */
    void resumeMusic();
    
    /**
     * @brief Detiene la reproducción de música
     * @param fadeOutMs Duración del fade out en milisegundos
     */
    void stopMusic(int fadeOutMs = 0);
    
    /**
     * @brief Verifica si hay música reproduciéndose actualmente
     * @return true si hay música reproduciéndose, false en caso contrario
     */
    bool isMusicPlaying();
    
    /**
     * @brief Establece el volumen de la música
     * @param volume Volumen en escala de 0 a 100
     */
    void setMusicVolume(int volume);
    
    /**
     * @brief Establece el volumen de los efectos de sonido
     * @param volume Volumen en escala de 0 a 100
     */
    void setSoundVolume(int volume);
    
    /**
     * @brief Obtiene el volumen actual de la música
     * @return Volumen en escala de 0 a 100
     */
    int getMusicVolume() const;
    
    /**
     * @brief Obtiene el volumen actual de los efectos de sonido
     * @return Volumen en escala de 0 a 100
     */
    int getSoundVolume() const;
    
    /**
     * @brief Libera un efecto de sonido de la memoria
     * @param id Identificador del efecto de sonido a liberar
     */
    void freeSound(const std::string& id);
    
    /**
     * @brief Libera una pista de música de la memoria
     * @param id Identificador de la pista de música a liberar
     */
    void freeMusic(const std::string& id);
    
    /**
     * @brief Libera todos los recursos de audio
     */
    void freeAll();

    void save();
};

#endif // AUDIO_MANAGER_H