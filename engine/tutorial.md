@mainpage Tutorial

# Tutorial

Esta página te guiará paso a paso para empezar a usar Capybara Engine.


## 1: Configurar Main.cpp en el directorio raiz del proyecto

```cpp
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include "Game.h"
#include "Engine.h"
#include <iostream>

int SDL_main(int argc, char* argv[]) {
    Game game("Nombre del juego", 800, 600);
    game.run();

    return 0;
}
```

## 2: Configurar Game.h de la carpeta game
```cpp
#ifndef GAME_H
#define GAME_H

#include "Engine.h"
#include "Systems.h"
#include "Components.h"
#include "SceneLoader.h"
#include "Debug.h"

//Esto es para el tutorial
#include "ComponenteDePrueba.h"

class Game {
public:
    Game(const char* title, int width, int height) {}

    void registro() {
        SceneLoader* sl = &(SceneLoader::getInstance());

        sl->registerComponentLoader<ComponenteDePrueba, ComponenteDePruebaParameters>(
            "CDP",
            [](const nlohmann::json& j, EntityManager& entityManager) -> ComponenteDePrueba* {
                return new ComponenteDePrueba(ComponenteDePruebaLoader::fromJSON(j, entityManager));
            }
        );

        //Por cada componente personalizado creado para el juego, repetir el proceso de arriba
    }
    void run() {

        try {
            Engine* engine = Engine::getInstance("Nombre del juego", 800, 600);
            registro();
            Debug::EnableFileLogging("Nombre del fichero de logs");  
            
            //Por cada escena a cargar de inicio, hacer lo siguiente
            engine->getSceneManager()->addScene("Nombre de la escena");
            
            engine->run();  
            Debug::DisableFileLogging();  
    
        } catch (const std::exception& e) {
            Debug::Error(std::string("Unhandled Exception: ") + e.what());
        } catch (...) {
            Debug::Error("Unknown exception caught!");
        }
    }
};

#endif
```

## 3: Crear componentes personalizados (Plantilla)

Si el componente a crear se llama T, se deben crear complementariamente
TParameters y TLoader

```cpp
#include "Components.h"
//Incluir ficheros necesarios del engine para el componente. Algunos ejemplos:
#include "InputManager.h"
#include "Debug.h"

struct TParameters {
    //Parametros necesarios para pasar a T
    ComponenteDePruebaParameters(Transform* t) : transform(t) {}
};

struct T : public Component {
    //Parametros necesarios para el componente
    T(TParameters t) {}
    
    void start() override { }

    void update(float deltaTime) override { };
};

class TLoader {
    public:

        static TParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            //Si el componente requiere el Transform de la entidad en la que esta
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("T requires a Transform component");
            }
    
            TParameters params();
    
            return params;
        }
    
        static T createFromJSON( nlohmann::json& j, EntityManager& entityManager) {
            return T(fromJSON(j, entityManager));
        }
    };
```

## 3: Escena de ejemplo, donde CDP y CDP2 son componentes personalizados
```json
{
    "name": "test",
    "entities": 
    [
        {
            "components": {
                "Transform": {
                    "posX": 0,
                    "posY": 0,
                    "rotZ": 0,
                    "sizeX": 1,
                    "sizeY": 1,
                    "anchor": "CENTER"
                },
                "Camera": {
                    "zoom": 1,
                    "windowWidth": 800,
                    "windowHeight": 600
                }
            }
        },
        {
            "components": {
                "Transform": {
                    "posX": 300,
                    "posY": 100,
                    "rotZ": 0,
                    "sizeX": 0.1,
                    "sizeY": 0.1,
                    "anchor": "CENTER"
                },
                "Collider": {
                    "scaleX": 100,
                    "scaleY": 100,
                    "isTrigger": false
                },
                "Rigidbody": {
                    "velocityX": 0,
                    "velocityY": 0,
                    "mass": 100,
                    "gravity": 9.8,
                    "drag": 0.00,
                    "bounceFactor": 0.99,
                    "kinematic": false
                },
                "Sprite": {
                    "texturePath": "megaman.png",
                    "anchor": "CENTER"
                },
                "CDP2": {
                    
                }
            }
        },
        {
            "components": {
                "Transform": {
                    "posX": 300,
                    "posY": 500,
                    "rotZ": 0,
                    "sizeX": 1,
                    "sizeY": 1,
                    "anchor": "CENTER"
                },
                "Collider": {
                    "scaleX": 100,
                    "scaleY": 10,
                    "isTrigger": false
                },
                "Rigidbody": {
                    "velocityX": 0,
                    "velocityY": 0,
                    "mass": 10000,
                    "gravity": 9.8,
                    "drag": 0.01,
                    "bounceFactor": 0.01,
                    "kinematic": true
                },
                "Sprite": {
                    "texturePath": "suelo.png",
                    "anchor": "CENTER"
                },
                "CDP": {
                    
                }
            }
        }
    ]
}
```

## 4: Uso de prefabs

Puedes tener entidades predefinidas, para no tener que copiarla entera en cada escena

### Megaman prefab
```json
{
    "components": {
        "Transform": {
            "posX": 300,
            "posY": 100,
            "rotZ": 0,
            "sizeX": 0.1,
            "sizeY": 0.1,
            "anchor": "CENTER"
        },
        "Collider": {
            "scaleX": 100,
            "scaleY": 100,
            "isTrigger": false
        },
        "Rigidbody": {
            "velocityX": 0,
            "velocityY": 0,
            "mass": 100,
            "gravity": 9.8,
            "drag": 0.00,
            "bounceFactor": 0.99,
            "kinematic": false
        },
        "Sprite": {
            "texturePath": "megaman.png",
            "anchor": "CENTER"
        },
        "CDP2": {
            
        }
    }
}
```

### La escena del punto 3 con el prefab
```json
{
    "name": "test",
    "entities": 
    [
        {
            "components": {
                "Transform": {
                    "posX": 0,
                    "posY": 0,
                    "rotZ": 0,
                    "sizeX": 1,
                    "sizeY": 1,
                    "anchor": "CENTER"
                },
                "Camera": {
                    "zoom": 1,
                    "windowWidth": 800,
                    "windowHeight": 600
                }
            }
        },
        {
            "prefab": "megaman"
        },
        {
            "components": {
                "Transform": {
                    "posX": 300,
                    "posY": 500,
                    "rotZ": 0,
                    "sizeX": 1,
                    "sizeY": 1,
                    "anchor": "CENTER"
                },
                "Collider": {
                    "scaleX": 100,
                    "scaleY": 10,
                    "isTrigger": false
                },
                "Rigidbody": {
                    "velocityX": 0,
                    "velocityY": 0,
                    "mass": 10000,
                    "gravity": 9.8,
                    "drag": 0.01,
                    "bounceFactor": 0.01,
                    "kinematic": true
                },
                "Sprite": {
                    "texturePath": "suelo.png",
                    "anchor": "CENTER"
                },
                "CDP": {
                    
                }
            }
        }
    ]
}
```

Si quieres instanciar un prefab desde el codigo de un componente (El componente necesitara la referencia del EntityManager de la escena en la que este):

```cpp
if (!PrefabManager::getInstance().hasPrefab("NOMBRE_DEL_PREFAB")) {
    PrefabManager::getInstance().loadPrefab("NOMBRE_DEL_PREFAB");
}
Entity entity = PrefabManager::getInstance().instantiatePrefab("NOMBRE_DEL_PREFAB", entityManager);
```

## 5: Ejemplo de Animator

```json
"Animator": {
    "parameters": {
        "isWalking": {"type": "bool", "value": false}
    },
    "states": {
        "idle": {
            "looping": true,
            "folder": "sprites/megaman/idle", 
            "duration": 0.2
        },
        "run": {
            "looping": true,
            "folder": "sprites/megaman/run", 
            "duration": 0.12
        }
    },
    "transitions": [
        {
            "from": "idle",
            "to": "run",
            "conditions": [
                {"parameter": "isWalking", "mode": "EQ", "value": 1}
            ]
        },
        {
            "from": "run",
            "to": "idle",
            "conditions": [
                {"parameter": "isWalking", "mode": "NEQ", "value": 1}
            ]
        }
    ],
    "initialState": "idle"
}
```