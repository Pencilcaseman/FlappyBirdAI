#pragma once

// Helper struct containing two rectangles: one for the upper wall and one for the lower wall
struct WallRectangles {
	surge::Rectangle upper;
	surge::Rectangle lower;
};

// A wall which moves from right to left and has a gap in the middle. The location of the gap is
// random and the birds must fly through it to survive.
class Wall {
public:
	Wall()					= default;
	Wall(const Wall &other) = default;
	Wall(Wall &&other)		= default;

	Wall(double gapHeight, const librapid::Vec2d &size, const librapid::Vec2d &position,
		 const librapid::Vec2d &velocity, const librapid::Vec2d &acceleration,
		 double timeScale = 1.0) :
			m_gapHeight(gapHeight),
			m_size(size), m_position(position), m_velocity(velocity), m_acceleration(acceleration),
			m_timeScale(timeScale) {}

	Wall &operator=(const Wall &other) = default;

	Wall &operator=(Wall &&other) = default;

	[[nodiscard]] WallRectangles rectangles() const {
		// Return two Rectangle instances. The first is the upper portion of the wall; the second
		// is the lower portion of the wall.

		return WallRectangles {
		  surge::Rectangle(m_position, m_size),
		  surge::Rectangle(m_position.x(),
						   m_position.y() + m_size.y() + m_gapHeight,
						   m_size.x(),
						   surge::window.height() - m_position.y() - m_size.y() - m_gapHeight)};
	}

	[[nodiscard]] double gapHeight() const { return m_gapHeight; }
	[[nodiscard]] const librapid::Vec2d &size() const { return m_size; }
	[[nodiscard]] const librapid::Vec2d &position() const { return m_position; }
	[[nodiscard]] const librapid::Vec2d &velocity() const { return m_velocity; }
	[[nodiscard]] const librapid::Vec2d &acceleration() const { return m_acceleration; }
	[[nodiscard]] double timeScale() const { return m_timeScale; }

	double &gapHeight() { return m_gapHeight; }
	librapid::Vec2d &size() { return m_size; }
	librapid::Vec2d &position() { return m_position; }
	librapid::Vec2d &velocity() { return m_velocity; }
	librapid::Vec2d &acceleration() { return m_acceleration; }
	double &timeScale() { return m_timeScale; }

	void update() {
		// The walls accelerate slowly as the game progresses to increase the difficulty

		m_velocity += m_acceleration * m_timeScale;
		m_velocity.x(librapid::clamp(m_velocity.x(), -MAX_WALL_SPEED, 0));
		m_position += m_velocity * m_timeScale;
	}

	void draw(surge::Color color = surge::Color::brown) const {
		auto [upper, lower] = rectangles();
		upper.draw(color);
		lower.draw(color);
	}

private:
	double m_gapHeight;
	librapid::Vec2d m_size;
	librapid::Vec2d m_position;
	librapid::Vec2d m_velocity;
	librapid::Vec2d m_acceleration;
	double m_timeScale;
};

// Create a new instance of a wall at a given position
Wall createWall(double wallPosition, double wallSpeed = WALL_SPEED) {
	auto gapPosition =
	  librapid::random<double>(WALL_BUFFER, surge::window.height() - WALL_GAP_SIZE - WALL_BUFFER);
	return Wall(WALL_GAP_SIZE,
				librapid::Vec2d(WALL_WIDTH, gapPosition),
				librapid::Vec2d(wallPosition, 0),
				librapid::Vec2d(-librapid::abs(wallSpeed), 0),
				librapid::Vec2d(-WALL_ACCELERATION, 0),
				worldSpeed);
}

// Update the walls and draw them
void updateWalls(std::vector<Wall> &walls) {
	for (auto &wall : walls) {
		wall.update();
		wall.draw();

		// To save memory, walls that have gone off the screen are recycled back to the far right
		// of the screen. They're placed after the furthest wall with a gap between them to ensure
		// the birds can actually make it through both consecutive gaps.
		if (wall.position().x() < -WALL_WIDTH) {
			// Find the furthest wall
			int64_t furthestWallIndex	= 0;
			double furthestWallDistance = 0;
			for (int64_t i = 0; i < walls.size(); ++i) {
				if (walls[i].position().x() > furthestWallDistance) {
					furthestWallIndex	 = i;
					furthestWallDistance = walls[i].position().x();
				}
			}

			// As the walls move faster, the gap between them increases to accommodate for the
			// decreased time between successive walls.

			const auto &furthest = walls[furthestWallIndex];
			double vel			 = furthest.velocity().x();
			double space =
			  WALL_SPACING + WALL_WIDTH * librapid::abs(vel) * WALL_SPEED_DISTANCE_COEFFICIENT;
			wall = createWall(furthest.position().x() + space, vel);
		}
	}
}

// Reset all the walls and re-create them just off the screen
void resetWalls(std::vector<Wall> &walls) {
	int64_t numWalls = walls.size();
	walls.clear();
	walls.reserve(numWalls);
	for (int64_t i = 0; i < numWalls; ++i) {
		walls.push_back(createWall(surge::window.width() + WALL_SPACING * i));
	}
}
