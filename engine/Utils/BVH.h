#ifndef BVH_H
#define BVH_H

#include <SDL2/SDL.h>
#include "Components.h"
#include <vector>

class Collider; // Forward declaration

class BVH {
private:
    struct Node {
        Rect bounds;
        std::vector<Collider*> colliders;
        Node* left;
        Node* right;
        
        Node();
        ~Node();
    };

    Node* root;
    int maxCollidersPerNode;

    // Private helper functions
    Rect CalculateBounds(const std::vector<Collider*>& colliders);
    Node* Build(std::vector<Collider*>& colliders, int depth);
    void GetPotentialCollisions(Node* node, const Rect& target, 
                              std::vector<Collider*>& potentials);
    void GetAllColliders(Node* node, std::vector<Collider*>& colliders);

public:
    BVH(int maxPerNode = 4);
    ~BVH();

    void Build(std::vector<Collider*>& colliders);
    std::vector<Collider*> GetPotentialCollisions(const Rect& target);
    void Insert(Collider* collider);
};

#endif // BVH_H