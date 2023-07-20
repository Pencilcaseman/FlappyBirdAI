#pragma once

// A class representing a "bird" in the game, capable of moving and jumping with a brain that
// determines its actions
template<typename Scalar, typename Backend>
class BirdImpl {
public:
	using BirdBrain = Brain<Scalar, Backend>;
	using Array		= librapid::Array<Scalar, Backend>;

	BirdImpl()						= default;
	BirdImpl(const BirdImpl &other) = default;
	BirdImpl(BirdImpl &&other)		= default;

	BirdImpl(const librapid::Vec2d &size, const librapid::Vec2d &position,
			 const librapid::Vec2d &velocity, const librapid::Vec2d &acceleration,
			 double timeScale = 1.0) :
			m_size(size),
			m_position(position), m_velocity(velocity), m_acceleration(acceleration),
			m_timeScale(timeScale) {}

	BirdImpl &operator=(const BirdImpl &other) = default;
	BirdImpl &operator=(BirdImpl &&other)	   = default;

	[[nodiscard]] surge::Rectangle rectangle() const {
		return surge::Rectangle(m_position, m_size);
	}

	[[nodiscard]] const librapid::Vec2d &size() const { return m_size; }
	[[nodiscard]] const librapid::Vec2d &position() const { return m_position; }
	[[nodiscard]] const librapid::Vec2d &velocity() const { return m_velocity; }
	[[nodiscard]] const librapid::Vec2d &acceleration() const { return m_acceleration; }
	[[nodiscard]] const BirdBrain &brain() const { return m_brain; }
	[[nodiscard]] double timeScale() const { return m_timeScale; }
	[[nodiscard]] bool alive() const { return m_alive; }
	[[nodiscard]] double fitness() const { return m_fitness; }

	librapid::Vec2d &size() { return m_size; }
	librapid::Vec2d &position() { return m_position; }
	librapid::Vec2d &velocity() { return m_velocity; }
	librapid::Vec2d &acceleration() { return m_acceleration; }
	BirdBrain &brain() { return m_brain; }
	double &timeScale() { return m_timeScale; }
	bool &alive() { return m_alive; }
	double &fitness() { return m_fitness; }

	void kill(double fitness) {
		if (!m_alive) return; // Dead birds can't die again :P
		m_alive	  = false;
		m_fitness = fitness * fitness; // Square the fitness to emphasize the importance of
									   // surviving longer
	}

	void update() {
		if (!m_alive) return;

		// Simple physics implementation
		m_velocity += m_acceleration * m_timeScale;
		m_position += m_velocity * m_timeScale;
		m_acceleration *= 0.0;
	}

	void draw(surge::Color color = surge::Color::cyan) const {
		if (!m_alive) return;

		// Draw a solid rectangle with an outline
		rectangle().draw(color);
		rectangle().setThickness(5).drawLines(surge::Color::blue);
	}

	void jump() {
		// To jump, we can simply set the birds velocity. A negative value points up the screen
		m_velocity.y(-BIRD_JUMP_VELOCITY);
	}

private:
	librapid::Vec2d m_size;
	librapid::Vec2d m_position;
	librapid::Vec2d m_velocity;
	librapid::Vec2d m_acceleration;
	BirdBrain m_brain;
	double m_timeScale;

	bool m_alive	 = true;
	double m_fitness = 0;
};

using Bird = BirdImpl<Scalar, Backend>;

// Create a new bird brain, which is a neural network with 5 inputs and 1 output. The hidden layers
// can be customised
Bird::BirdBrain createBirdBrain() {
	Bird::BirdBrain brain;
	brain << 5 << 8 << 1;
	brain.construct();
	return brain;
}

// Given a bird and a set of walls, generate the set of input values it "senses" from its
// environment. This is then passed to the bird's brain to determine whether it should jump
Bird::Array generateBirdInputs(const Bird &bird, const std::vector<Wall> &walls) {
	// Birds receive the following inputs:
	// 1. The bird's height relative to the top of the screen
	// 2. The bird's vertical velocity
	// 3. The distance to the next wall
	// 4. The y position of the next wall's gap
	// 5. The closest wall's horizontal velocity

	if (!bird.alive()) return {};

	// Find the closest wall
	int64_t closestWallIndex   = 0;
	double closestWallDistance = DBL_MAX;
	for (int64_t i = 0; i < walls.size(); ++i) {
		if (walls[i].position().x() < closestWallDistance &&
			walls[i].position().x() + walls[i].size().x() > bird.position().x()) {
			closestWallIndex	= i;
			closestWallDistance = walls[i].position().x();
		}
	}

	const auto &closest = walls[closestWallIndex];

	// Map the values into a sensible range
	double birdHeight	= librapid::map(bird.position().y(), 0, surge::window.height(), -1, 1);
	double birdVelocity = librapid::map(bird.velocity().y(), -10, 10, -1, 1);
	double wallDistance =
	  librapid::map(closest.position().x() - bird.position().x(), 0, surge::window.width(), -1, 1);
	double wallGapPosition = librapid::map(closest.size().y(), 0, surge::window.height(), -1, 1);
	double wallVelocity	   = librapid::map(closest.velocity().x(), -10, 10, -1, 1);

	return librapid::fromData<Scalar, Backend>({static_cast<Scalar>(birdHeight),
												static_cast<Scalar>(birdVelocity),
												static_cast<Scalar>(wallDistance),
												static_cast<Scalar>(wallGapPosition),
												static_cast<Scalar>(wallVelocity)});
}

int64_t updateBirds(std::vector<Bird> &birds, const std::vector<Wall> &walls) {
	int64_t alive = 0;

	for (auto &bird : birds) {
		if (!bird.alive()) continue;

		// Set the bird's acceleration so that it falls under gravity
		double now			= librapid::now();
		bird.acceleration() = librapid::Vec2d(0, GRAVITY);
		bird.update();

		// Check for collisions with the ceiling and floor
		if (bird.position().y() < 0 ||
			bird.position().y() + bird.size().y() > surge::window.height()) {
			bird.kill(now - generationStartTime);
		}

		// Check for collisions with the walls
		for (const auto &wall : walls) {
			auto [upper, lower] = wall.rectangles();
			if (rectIntersection(bird.rectangle(), upper) ||
				rectIntersection(bird.rectangle(), lower)) {
				bird.kill(now - generationStartTime);
			}
		}

		// Assuming the bird is alive, generate a set of inputs and give it to the bird's brain.
		// If the resulting output is greater than 0.5, the bird jumps
		if (bird.alive()) {
			auto inputs = generateBirdInputs(bird, walls);
			auto output = bird.brain().forward(inputs);
			if (output(0) > 0.5) { bird.jump(); }
			bird.draw();
			++alive;
		}
	}

	// The best bird from the previous generation is always put in the first position of the array,
	// so draw it a different colour. It is drawn last so that it is always on top
	birds[0].draw(surge::Color::red);

	return alive;
}

// Create a default bird instance without a brain
Bird createBird() {
	return {librapid::Vec2d(30, 30),
			librapid::Vec2d(50, surge::window.height() / 2),
			librapid::Vec2d(0, 0),
			librapid::Vec2d(0, 0),
			worldSpeed};
}
