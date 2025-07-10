# Como añadirlo al Json
"ReinforcementIA": {
    "states": [
        {"name": "Idle", "actions": ["Move", "Wait"]},
        {"name": "Moving", "actions": ["Stop", "Continue"]}
    ],
    "qTableFile": "qtable.json",
    "learning_rate": 0.1,
    "discount_factor": 0.9,
    "epsilon": 0.2
}

# Config desde el componente (Asumimos que ria es un ReinforcementIAComponent)

// Register actions
ria.registerAction("Idle", "Move", [](EntityManager* em, float dt) {
    em->moveEntity(10.0f * dt);
});
ria.registerAction("Idle", "Wait", [](EntityManager* em, float dt) {
    std::cout << "Entity waiting" << std::endl;
});
ria.registerAction("Moving", "Stop", [](EntityManager* em, float dt) {
    em->stopEntity();
});
ria.registerAction("Moving", "Continue", [](EntityManager* em, float dt) {
    em->moveEntity(10.0f * dt);
});

// Register reward function
ria.registerRewardFunction([](EntityManager* em, const std::string& state, const std::string& action, float dt) -> float {
    if (state == "Idle" && action == "Move" && em->entityPosition < 100.0f) return 1.0f;
    if (state == "Moving" && action == "Continue" && em->entityPosition < 100.0f) return 0.5f;
    if (state == "Moving" && action == "Stop" && em->entityPosition >= 100.0f) return 2.0f;
    return -0.1f;
});