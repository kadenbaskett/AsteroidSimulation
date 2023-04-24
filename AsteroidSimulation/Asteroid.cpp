#include <vector>
#include "cyMatrix.h"

class Asteroid {
public:
	cy::Matrix4f modelMatrix;
	cy::Vec3f velocity;
	float radius;

	Asteroid() {
		modelMatrix.SetIdentity();
		velocity.Zero();
		radius = 0.0f;
	}

	// Update the position of the asteroid based on its velocity
	void update(float deltaTime) {
		cy::Vec3f position = modelMatrix.GetTranslation();
		position += velocity * deltaTime;
		modelMatrix.SetTranslation(position);
	}

	// Check if the asteroid is colliding with another asteroid
	bool checkCollision(const Asteroid& other) const {
		cy::Vec3f currentAsteroidCenter = modelMatrix.GetTranslation();
		cy::Vec3f otherAsteroidCenter = other.modelMatrix.GetTranslation();

		// compute distance between model'c centers
		float dx = otherAsteroidCenter.x - currentAsteroidCenter.x;
		float dy = otherAsteroidCenter.y - currentAsteroidCenter.y;
		float dz = otherAsteroidCenter.z - currentAsteroidCenter.z;
		float distance = std::sqrt(dx * dx + dy * dy + dz * dz);

		float radiusSum = radius + other.radius;

		if (distance <= radiusSum) {
			// asteroids are colliding
			return true;
		}
		else {
			// asteroids are NOT colliding
			return false;
		}
	}
};

