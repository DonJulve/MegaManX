#include "EntityManager.h"
#include "Debug.h"

bool EntityManager::hasComponentPointer(Component* ptr) const {
    if (!ptr) {
        return false; // Null pointers are not considered "existing"
    }

    // Iterate over all entities
    for (const auto& [entity, components] : entities) {
        // Iterate over all components of this entity
        for (const auto& [typeIdx, component] : components) {
            if (component.get() == ptr) {
                // The raw pointer matches the one managed by this shared_ptr
                return true;
            }
        }
    }
    return false; // No match found
}

// Add a component to an entity (using shared_ptr)
void EntityManager::addComponent(Entity entity, std::type_index type, std::shared_ptr<Component> component) {
    // Store the component in the entities map by its type
    entities[entity][type] = component;

    component->parent = entity;
}

// Add a component to an entity (using raw pointer)
void EntityManager::addComponent(Entity entity, std::type_index type, Component* component) {
    // Convert raw pointer to shared_ptr to ensure proper memory management
    std::shared_ptr<Component> sharedComponent(component);

    // Store the component in the entities map by its type
    entities[entity][type] = sharedComponent;

    component->parent = entity;
}

// Create a new entity and return its ID
Entity EntityManager::createEntity() {
    Entity id = nextEntity++;
    lastEntity = id;
    activeEntities.insert(id);
    Debug::Log("Entity created: " + std::to_string(id));
    return id;
}

// Mark an entity for deletion at the end of the frame
void EntityManager::deleteEntity(Entity entity) {
    // Add the entity to the deletion queue
    entitiesToDelete.insert(entity);
}

// Perform the deletion of entities marked for removal
void EntityManager::processDeletion() {
    for (Entity entity : entitiesToDelete) {
        // Check if the entity exists in activeEntities
        if (activeEntities.find(entity) != activeEntities.end()) {
            // Remove all components associated with this entity
            entities.erase(entity);

            // Remove the entity from activeEntities
            activeEntities.erase(entity);

            // Update lastEntity if we're deleting the highest ID
            if (entity == lastEntity && !activeEntities.empty()) {
                lastEntity = *activeEntities.rbegin(); // Get the highest remaining ID
            } else if (activeEntities.empty()) {
                lastEntity = 0; // Reset if no entities remain
            }

            Debug::Log("Entity deleted: " + std::to_string(entity));
        }
    }

    // Clear the deletion queue after processing
    entitiesToDelete.clear();
}

// Get the active entities
std::set<Entity>& EntityManager::getEntities() {
    return activeEntities;
}

// Get the components of an entity
std::unordered_map<std::type_index, std::shared_ptr<Component>>& EntityManager::getEntityComponents(Entity entity) {
    return entities[entity]; 
}
