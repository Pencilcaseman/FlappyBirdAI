#include "include/configuration.hpp"

int main() {
	fmt::print(fmt::fg(fmt::color::orange_red) | fmt::emphasis::bold,
			   "Welcome to the Flappy Bird AI!\n");

	librapid::setNumThreads(1);

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

	// Note that `init` will end up drawing a single frame to the screen in order to initialize the
	// OpenGL context. This is necessary for many functions to work properly.
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
		bird		 = createBird();
		bird.brain() = createBirdBrain();
	}

	// The main loop
	while (!mainWindow.shouldClose()) {
		// Begin a drawing and clear the screen
		mainWindow.beginDrawing();
		mainWindow.clear(surge::Color::veryDarkGray);

		// Handle inputs
		if (mouse.isButtonPressed(surge::MouseButton::left)) { birds[0].jump(); }

		// Update the birds and walls
		updateWalls(walls);
		int64_t alive = updateBirds(birds, walls);

		// Occasionally print the number of birds that are still alive
		if (mainWindow.frameCount() % 10 == 0) {
			fmt::print(fmt::fg(fmt::color::purple) | fmt::emphasis::bold,
					   "Alive: {:>7} / {:>7}\r",
					   alive,
					   NUM_BIRDS);
		}

		if (alive == 0) {
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

		// Draw some built-in stats to the screen (Surge will support text soon, but it is not
		// currently implemented)
		mainWindow.drawFPS(librapid::Vec2i(20, 20));
		mainWindow.drawFrameTime(librapid::Vec2i(20, 40));
		mainWindow.drawTime(librapid::Vec2i(20, 60));

		// End the drawing
		mainWindow.endDrawing();
	}

	return 0;
}
