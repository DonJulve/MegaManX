#ifndef BASECOMPONENT_H
#define BASECOMPONENT_H

#include "Components.h"
#include "Debug.h"
#include "BlockComponent.h"
#include "vector"


struct BaseComponentParameters {

    Transform* transform;
    EntityManager* entityManager;

    BaseComponentParameters(Transform* t, EntityManager* em):
    transform(t), entityManager(em) {}
};

struct BaseComponent : public Component {
    Transform* transform;
    EntityManager* entityManager;
    int numBlocks = 2;
    Collider* collider;

    int alturaMaxima1;
    int alturaMaxima2;

    std::vector<BlockComponent*> blocks;

    BaseComponent(BaseComponentParameters params):
    transform(params.transform),
    entityManager(params.entityManager) {}


    std::function<void(int)> blockDeathCallback = [this](int pos) {
        if(pos == 0){
            blocks[0]->enBase = false;
            blocks.erase(blocks.begin());
            if(blocks.size() > 0){
                blocks[0]->rigidbody->velocityY = 100.0f;
                blocks[0]->asignarAltura(alturaMaxima1);
                blocks[0]->setPosition(0);
            }
        }
        else if(pos == 1){
            blocks[1]->enBase = false;
            blocks.erase(blocks.begin() + 1);
        }
    };



    void attack(){
        if(!hayBloques()) return;
        blocks[0]->changeToSpike();
        blocks[0]->rigidbody->velocityX = -200.0f;
        blocks[0]->enBase = false;
        blocks.erase(blocks.begin());
        //REASIGNAR TAGS
        //PONEMOS QUE CAIGAN LOS BLOQUES
        //El bloque 2 cae
        if(blocks.size() > 0){
            blocks[0]->rigidbody->velocityY = 100.0f;
            blocks[0]->asignarAltura(alturaMaxima1);
            blocks[0]->setPosition(0);
        }
    }

    bool bloquesLenos(){
        return blocks.size() == numBlocks;
    }

    bool hayBloques(){
        return blocks.size() > 0;
    }

    bool bloquesColocados(){
        if(blocks.size() < 2){
            return false;
        }
        return blocks[1]->transform->posY == alturaMaxima2;
    }

    void crearBloque(){
        // Debug::Log("Creando bloque");
        if(blocks.size() < numBlocks){
            // Debug::Log("El bloque se puede crear");
            if(!PrefabManager::getInstance().hasPrefab("blockAxeMax")) {
                PrefabManager::getInstance().loadPrefab("blockAxeMax");
            }
            Entity blockEntity = PrefabManager::getInstance().instantiatePrefab("blockAxeMax", *entityManager);
            BlockComponent* blockComponent = entityManager->getComponent<BlockComponent>(blockEntity);
            blockComponent->start();
            blockComponent->enBase = true;
            blockComponent->setOnDeath(blockDeathCallback);
            if(blockComponent->transform){
                blockComponent->transform->posX = transform->posX;
                blockComponent->transform->posY = transform->posY;
            }
            if(blockComponent->rigidbody){
                blockComponent->rigidbody->velocityY = -100.0f;
            }
            if(blocks.size() == 0){
                //Creamos el primer bloque
                blockComponent->setPosition(0);
                blockComponent->asignarAltura(alturaMaxima1);
            }
            else{
                //Creamos el segundo bloque
                blockComponent->setPosition(1);
                blockComponent->asignarAltura(alturaMaxima2);
                
            }
            blocks.push_back(blockComponent);
            // Debug::Log("Bloque creado");
            // Debug::Log("Bloques en el vector: " + std::to_string(blocks.size()));
            //ASIGNAR TAGS

        }
    }

    void start() override {
        collider = entityManager->getComponent<Collider>(parent);
        this->collider->setTag("BaseAxeMax");

        alturaMaxima1 = transform->posY - 41;
        alturaMaxima2 = transform->posY - 82;
    }

    void update(float deltaTime) override {

    }
};

class BaseComponentLoader {
    public:
    
        static BaseComponentParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("BaseComponent requires a Transform component");
            }

            BaseComponentParameters params(
                transform,
                &entityManager
            );

            return params;
        }

        static BaseComponent createFromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            return BaseComponent(fromJSON(j, entityManager));
        }
};


#endif