#pragma once

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
		if (!m_alive) return;
		m_alive	  = false;
		m_fitness = fitness * fitness;
	}

	void update(double deltaTime) {
		if (!m_alive) return;

		m_velocity += m_acceleration * deltaTime * m_timeScale;
		m_position += m_velocity * deltaTime * m_timeScale;
		m_acceleration *= 0.0;
	}

	void draw(surge::Color color = surge::Color::cyan) const {
		if (!m_alive) return;
		rectangle().draw(color);
		rectangle().setThickness(5).drawLines(surge::Color::blue);
	}

	void jump() { m_velocity.y(-400.0); }

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

Bird::BirdBrain createBirdBrain() {
	Bird::BirdBrain brain;
	brain << 5 << 5 << 5 << 1;
	brain.construct();
	return brain;
}

Bird::Array generateBirdInputs(const Bird &bird, const std::vector<Wall> &walls) {
	// Birds receive the following inputs:
	// 1. The bird's height relative to the top of the screen
	// 2. The bird's vertical velocity
	// 3. The distance to the next wall
	// 4. The y position of the next wall's gap
	// 5. The closest wall's horizontal velocity

	if (!bird.alive()) return {};

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

	double birdHeight	= librapid::map(bird.position().y(), 0, surge::window.height(), -1, 1);
	double birdVelocity = librapid::map(bird.velocity().y(), -1000, 1000, -1, 1);
	double wallDistance =
	  librapid::map(closest.position().x() - bird.position().x(), 0, surge::window.width(), -1, 1);
	double wallGapPosition = librapid::map(closest.size().y(), 0, surge::window.height(), -1, 1);
	double wallVelocity	   = librapid::map(closest.velocity().x(), -1000, 1000, -1, 1);

#if 0

	// Bird height
	surge::Line(bird.position().x(),
				0,
				bird.position().x(),
				librapid::map(birdHeight, -1, 1, 0, surge::window.height()))
	  .setThickness(5)
	  .draw(surge::Color::red);

	// Wall distance
	surge::Line(bird.position().x(),
				bird.position().y(),
				librapid::map(wallDistance, -1, 1, 0, surge::window.width()) + bird.position().x(),
				bird.position().y())
	  .setThickness(5)
	  .draw(surge::Color::green);

	// Wall gap position
	surge::Line(closest.position().x(),
				0,
				closest.position().x(),
				librapid::map(wallGapPosition, -1, 1, 0, surge::window.height()))
	  .setThickness(5)
	  .draw(surge::Color::blue);

#endif

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

		double now			= librapid::now();
		bird.acceleration() = librapid::Vec2d(0, GRAVITY);
		bird.update(surge::window.frameTime());

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

		if (bird.alive()) {
			auto inputs = generateBirdInputs(bird, walls);
			auto output = bird.brain().forward(inputs);
			if (output(0) > 0.5) { bird.jump(); }
			bird.draw();
			++alive;
		}
	}

	if (birds[0].alive()) birds[0].draw(surge::Color::red);

	return alive;
}

Bird createBird() {
	return {librapid::Vec2d(30, 30),
			librapid::Vec2d(50, surge::window.height() / 2),
			librapid::Vec2d(0, 0),
			librapid::Vec2d(0, 0),
			worldSpeed};
}
