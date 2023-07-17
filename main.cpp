#include "include/configuration.hpp"

int main() {
	fmt::print(fmt::fg(fmt::color::orange_red) | fmt::emphasis::bold,
			   "Welcome to the Flappy Bird AI!\n");

	librapid::setNumThreads(4);

#if defined(LIBRAPID_HAS_OPENCL)
	fmt::print(fmt::fg(fmt::color::lime_green) | fmt::emphasis::bold, "OpenCL is enabled.\n");
	librapid::configureOpenCL(true);
#else
	fmt::print(fmt::fg(fmt::color::red) | fmt::emphasis::bold, "OpenCL is disabled.\n");
#endif

#if defined(LIBRAPID_HAS_CUDA)
	fmt::print(fmt::fg(fmt::color::lime_green) | fmt::emphasis::bold, "CUDA is enabled.\n");
#else
	fmt::print(fmt::fg(fmt::color::red) | fmt::emphasis::bold, "CUDA is disabled.\n");
#endif

	// Configure the window and relevant devices
	surge::Window mainWindow(librapid::Vec2i(1000, 600), "Flappy Bird AI");
	mainWindow.setFlag(surge::WindowFlag::interlaced);
	mainWindow.setFlag(surge::WindowFlag::msaa4x);
	mainWindow.setFlag(surge::WindowFlag::vsync);
	mainWindow.init();

	surge::Mouse mouse;

	// Configure the walls
	std::vector<Wall> walls;

	for (int64_t i = 0; i < NUM_WALLS; ++i) {
		double wallPosition =
		  (i == 0) ? mainWindow.width() : walls.back().position().x() + WALL_SPACING;
		walls.emplace_back(createWall(wallPosition));
	}

	// The bird population
	generationStartTime = librapid::now();
	std::vector<Bird> birds(NUM_BIRDS);

	// Configure each bird
	for (auto &bird : birds) {
		bird = createBird();

		bird.brain() = createBirdBrain();
	}

	while (!mainWindow.shouldClose()) {
		double frameTime = mainWindow.frameTime();

		mainWindow.beginDrawing();
		mainWindow.clear(surge::Color::veryDarkGray);

		// Handle inputs
		if (mouse.isButtonPressed(surge::MouseButton::left)) { birds[0].jump(); }

		updateWalls(walls, frameTime);
		if (updateBirds(birds, walls) == 0) {
			// All birds are dead, so start a new generation
			++generationNumber;
			double generationTime = librapid::now() - generationStartTime;

			fmt::print(fmt::fg(fmt::color::orange_red) | fmt::emphasis::bold,
					   "\n\nGeneration {} lasted {}.\n",
					   generationNumber,
					   librapid::formatTime(generationTime));

			// Reset the walls before the birds, since they may collide with "ghost" walls
			// and cause some strange bugs
			resetWalls(walls);

			// Create the next generation of mutated bird brains
			std::vector<std::pair<Bird::BirdBrain, double>> birdBrains;

			for (auto &bird : birds) { birdBrains.emplace_back(bird.brain(), bird.fitness()); }

			std::vector<Bird::BirdBrain> nextGeneration = newGeneration(birdBrains);

			for (int64_t i = 0; i < NUM_BIRDS; ++i) {
				birds[i]		 = createBird();
				birds[i].brain() = nextGeneration[i];
			}

			generationStartTime = librapid::now();
		}

		mainWindow.endDrawing();
	}

	// Testing

	int64_t n = 1000;
	std::vector<std::pair<Bird::BirdBrain, double>> birdBrains;
	auto input = librapid::Array<float>(librapid::Shape({6}));
	librapid::fillRandom(input, -1, 1);
	float target = 0.75f;

	for (int64_t i = 0; i < n; ++i) { birdBrains.emplace_back(createBirdBrain(), 0); }

	double mse = 0;
	for (int64_t i = 0; i < n; ++i) {
		double loss			 = std::abs(birdBrains[i].first.forward(input)(0) - target);
		birdBrains[i].second = 1.0 / loss;
		mse += loss * loss;
	}

	mse /= n;

	fmt::print("mse: {}\n", mse);

	for (int gen = 0; gen < 10; ++gen) {
		std::vector<Bird::BirdBrain> nextGeneration = newGeneration(birdBrains);
		// keep best
		auto best = bestBird(birdBrains);
		birdBrains[0].first = best.first.copy();

		for (int64_t i = 0; i < n; ++i) { birdBrains[i].first = nextGeneration[i]; }

		mse = 0;
		for (int64_t i = 0; i < n; ++i) {
			double loss			 = std::abs(birdBrains[i].first.forward(input)(0) - target);
			birdBrains[i].second = 1.0 / loss;
			mse += loss * loss;
		}

		mse /= n;

		fmt::print("mse: {}\n", mse);
	}

	return 0;
}
