// AudioManager.cpp
#include "AudioManager.h"
#include <iostream>
#include <fstream>

// Initialize static member
AudioManager* AudioManager::instance = nullptr;

AudioManager::AudioManager() : 
    musicVolume(50),
    soundVolume(50),
    currentMusicTrack("") {
    
    // Allocate channels for sound effects
    Mix_AllocateChannels(16);

    std::ifstream inFile("volume.save");
    if (inFile.is_open()) {
        // Load existing values
        inFile >> musicVolume >> soundVolume;
        inFile.close();
    } else {
        // Create file and write default values
        std::ofstream outFile("volume.save");
        if (outFile.is_open()) {
            outFile << musicVolume << " " << soundVolume;
            outFile.close();
        } else {
            std::cerr << "Failed to create volume.save file." << std::endl;
        }
    }
}

void AudioManager::save() {
    std::ofstream outFile("volume.save");
    if (outFile.is_open()) {
        outFile << musicVolume << " " << soundVolume;
        outFile.close();
    } else {
        std::cerr << "Failed to save volume settings to volume.save." << std::endl;
    }
}


AudioManager::~AudioManager() {
    // Free all resources
    freeAll();
}

AudioManager* AudioManager::getInstance() {
    if (instance == nullptr) {
        instance = new AudioManager();
    }
    return instance;
}

bool AudioManager::loadSound(const std::string& id, const std::string& filePathP) {
    // Free the sound if it already exists
    freeSound(id);

    std::string filePath = "resources/"+filePathP;
    
    // Load the sound effect
    Mix_Chunk* sound = Mix_LoadWAV(filePath.c_str());
    if (sound == nullptr) {
        std::cerr << "Failed to load sound effect " << filePath
                  << "! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }
    
    // Add the sound to the map
    soundEffects[id] = sound;
    return true;
}

bool AudioManager::loadMusic(const std::string& id, const std::string& filePath) {
    // Free the music if it already exists
    freeMusic(id);

    std::string filePath2 = "resources/"+filePath;
    
    // Load the music
    Mix_Music* track = Mix_LoadMUS(filePath2.c_str());
    if (track == nullptr) {
        std::cerr << "Failed to load music " << filePath2
                  << "! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }
    
    // Add the music to the map
    music[id] = track;
    return true;
}

int AudioManager::playSound(const std::string& id, int loops, int channel) {
    // Find the sound effect
    auto it = soundEffects.find(id);
    if (it == soundEffects.end()) {
        std::cerr << "Sound effect with ID " << id << " not found!" << std::endl;
        return -1;
    }
    
    // Play the sound effect
    channel = Mix_PlayChannel(channel, it->second, loops);
    if (channel == -1) {
        std::cerr << "Failed to play sound effect! SDL_mixer Error: " 
                  << Mix_GetError() << std::endl;
        return -1;
    }
    
    // Set the volume for this channel
    Mix_Volume(channel, soundVolume);
    
    return channel;
}

bool AudioManager::playMusic(const std::string& id, int loops, int fadeInMs) {
    // Find the music
    auto it = music.find(id);
    if (it == music.end()) {
        std::cerr << "Music with ID " << id << " not found!" << std::endl;
        return false;
    }
    
    // If music is already playing, stop it
    if (Mix_PlayingMusic() == 1) {
        Mix_HaltMusic();
    }
    
    // Play the music
    if (fadeInMs > 0) {
        if (Mix_FadeInMusic(it->second, loops, fadeInMs) == -1) {
            std::cerr << "Failed to play music! SDL_mixer Error: " 
                      << Mix_GetError() << std::endl;
            return false;
        }
    } else {
        if (Mix_PlayMusic(it->second, loops) == -1) {
            std::cerr << "Failed to play music! SDL_mixer Error: " 
                      << Mix_GetError() << std::endl;
            return false;
        }
    }
    
    // Set the current music track
    currentMusicTrack = id;
    
    // Set the volume
    Mix_VolumeMusic(musicVolume);
    
    return true;
}

void AudioManager::pauseMusic() {
    if (Mix_PlayingMusic() == 1) {
        Mix_PauseMusic();
    }
}

void AudioManager::resumeMusic() {
    if (Mix_PausedMusic() == 1) {
        Mix_ResumeMusic();
    }
}

void AudioManager::stopMusic(int fadeOutMs) {
    if (fadeOutMs > 0) {
        Mix_FadeOutMusic(fadeOutMs);
    } else {
        Mix_HaltMusic();
    }
    
    currentMusicTrack = "";
}

bool AudioManager::isMusicPlaying() {
    return Mix_PlayingMusic() == 1 && Mix_PausedMusic() == 0;
}

void AudioManager::setMusicVolume(int volume) {
    // Clamp volume between 0 and 100
    musicVolume = (volume < 0) ? 0 : (volume > MIX_MAX_VOLUME) ? MIX_MAX_VOLUME : volume;
    
    Mix_VolumeMusic(musicVolume);
}

void AudioManager::setSoundVolume(int volume) {
    // Clamp volume between 0 and 100
    soundVolume = (volume < 0) ? 0 : (volume > MIX_MAX_VOLUME) ? MIX_MAX_VOLUME : volume;
    
    
    // Set volume for all channels
    Mix_Volume(-1, soundVolume);
}

int AudioManager::getMusicVolume() const {
    return musicVolume;
}

int AudioManager::getSoundVolume() const {
    return soundVolume;
}

void AudioManager::freeSound(const std::string& id) {
    auto it = soundEffects.find(id);
    if (it != soundEffects.end()) {
        Mix_FreeChunk(it->second);
        soundEffects.erase(it);
    }
}

void AudioManager::freeMusic(const std::string& id) {
    auto it = music.find(id);
    if (it != music.end()) {
        if (currentMusicTrack == id) {
            stopMusic();
        }
        
        Mix_FreeMusic(it->second);
        music.erase(it);
    }
}

void AudioManager::freeAll() {
    // Stop any playing music
    stopMusic();
    
    // Free all sound effects
    for (auto& pair : soundEffects) {
        Mix_FreeChunk(pair.second);
    }
    soundEffects.clear();
    
    // Free all music
    for (auto& pair : music) {
        Mix_FreeMusic(pair.second);
    }
    music.clear();
}