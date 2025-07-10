#ifndef HEALTHCOMPONENT_H
#define HEALTHCOMPONENT_H
#include <algorithm>
#include "Components.h"

struct HealthComponentParameters {
    float maxHealth;

    HealthComponentParameters(float maxHP) : maxHealth(maxHP)  {}
};

class HealthComponent : public Component {
    private:
        float maxHealth;
        float currentHealth;
        bool isAlive;
    
    public:
        HealthComponent(HealthComponentParameters params) 
            : maxHealth(params.maxHealth), currentHealth(params.maxHealth), isAlive(true) {
        }
        
        void takeDamage(float damage, float percentageReduce = 0.0f) {
            if (!isAlive) {
                return;
            }
            
            float reducedDamage = damage * (1.0f - percentageReduce);
            
            currentHealth -= reducedDamage;
            if (currentHealth <= 0.0f) {
                currentHealth = 0.0f;
                isAlive = false;
            }
        }
        
        void heal(float amount) {
            if (!isAlive) {
                return;
            }
            
            currentHealth = std::min(maxHealth, currentHealth + amount);
        }
        
        float getCurrentHealth() const { return currentHealth; }
        float getMaxHealth() const { return maxHealth; }
        bool isDead() const { return !isAlive; }
        
        void reset() {
            currentHealth = maxHealth;
            isAlive = true;
        }
    };

    class HealthComponentLoader {
        public:
    
            static HealthComponentParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
                HealthComponentParameters params(j.value("maxHealth", 100.0f));
        
                return params;
            }
        
            // Helper function to directly create a Camera component from JSON
            static HealthComponent createFromJSON( nlohmann::json& j, EntityManager& entityManager) {
                return HealthComponent(fromJSON(j, entityManager));
            }
        };

#endif