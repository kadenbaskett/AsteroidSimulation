#ifndef ASTEROID_H
#define ASTEROID_H

#include <vector>
#include "cyMatrix.h"

class Asteroid {
public:
    cy::Matrix4f modelMatrix;
    cy::Vec3f velocity;
    float radius;

    Asteroid();

    void update(float deltaTime);

    bool checkCollision(const Asteroid& other) const;
};

#endif
