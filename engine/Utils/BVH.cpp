#include "BVH.h"
#include <algorithm>
#include "Components.h"

// Node constructor/destructor
BVH::Node::Node() : left(nullptr), right(nullptr) {}
BVH::Node::~Node() {
    delete left;
    delete right;
}

// BVH constructor/destructor
BVH::BVH(int maxPerNode) : root(nullptr), maxCollidersPerNode(maxPerNode) {}
BVH::~BVH() {
    delete root;
}

Rect BVH::CalculateBounds(const std::vector<Collider*>& colliders) {
    if (colliders.empty()) return Rect{0, 0, 0, 0};
    
    Rect bounds = colliders[0]->boundingBox;
    for (size_t i = 1; i < colliders.size(); ++i) {
        const Rect& box = colliders[i]->boundingBox;
        int minX = std::min(bounds.x, box.x);
        int minY = std::min(bounds.y, box.y);
        int maxX = std::max(bounds.x + bounds.w, box.x + box.w);
        int maxY = std::max(bounds.y + bounds.h, box.y + box.h);
        
        bounds.x = minX;
        bounds.y = minY;
        bounds.w = maxX - minX;
        bounds.h = maxY - minY;
    }
    return bounds;
}

BVH::Node* BVH::Build(std::vector<Collider*>& colliders, int depth) {
    Node* node = new Node();
    
    if (colliders.size() <= maxCollidersPerNode || depth > 100) {
        node->colliders = colliders;
        node->bounds = CalculateBounds(colliders);
        return node;
    }

    node->bounds = CalculateBounds(colliders);
    int width = node->bounds.w;
    int height = node->bounds.h;
    bool splitHorizontal = width > height;

    std::sort(colliders.begin(), colliders.end(),
        [splitHorizontal](Collider* a, Collider* b) {
            if (splitHorizontal)
                return a->boundingBox.x < b->boundingBox.x;
            return a->boundingBox.y < b->boundingBox.y;
        });

    size_t mid = colliders.size() / 2;
    std::vector<Collider*> leftColliders(colliders.begin(), colliders.begin() + mid);
    std::vector<Collider*> rightColliders(colliders.begin() + mid, colliders.end());

    node->left = Build(leftColliders, depth + 1);
    node->right = Build(rightColliders, depth + 1);
    
    return node;
}

void BVH::Build(std::vector<Collider*>& colliders) {
    delete root;
    if (colliders.empty()) {
        root = nullptr;
        return;
    }
    root = Build(colliders, 0);
}

void BVH::GetPotentialCollisions(Node* node, const Rect& target, 
                                std::vector<Collider*>& potentials) {
    if (!node)
        return;

    Rect rect = node->bounds;

    if (!Rect::hasIntersection(rect, target))
        return;

    if (!node->left && !node->right) {
        potentials.insert(potentials.end(), 
                         node->colliders.begin(), 
                         node->colliders.end());
        return;
    }

    GetPotentialCollisions(node->left, target, potentials);
    GetPotentialCollisions(node->right, target, potentials);
}

std::vector<Collider*> BVH::GetPotentialCollisions(const Rect& target) {
    std::vector<Collider*> potentials;
    if (root) {
        GetPotentialCollisions(root, target, potentials);
    }
    return potentials;
}

void BVH::Insert(Collider* collider) {
    std::vector<Collider*> colliders;
    if (root) {
        GetAllColliders(root, colliders);
    }
    colliders.push_back(collider);
    Build(colliders);
}

void BVH::GetAllColliders(Node* node, std::vector<Collider*>& colliders) {
    if (!node) return;
    
    if (!node->left && !node->right) {
        colliders.insert(colliders.end(), 
                        node->colliders.begin(), 
                        node->colliders.end());
        return;
    }
    
    GetAllColliders(node->left, colliders);
    GetAllColliders(node->right, colliders);
}