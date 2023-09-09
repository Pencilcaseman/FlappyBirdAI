#pragma once

// Return the best bird in the generation (the one with the highest fitness)
std::pair<Bird::BirdBrain, double>
bestBird(const std::vector<std::pair<Bird::BirdBrain, double>> &brains) {
	auto best = brains.front();
	for (const auto &brain : brains) {
		if (brain.second > best.second) { best = brain; }
	}
	return best;
}

// Select a parent for a new bird, weighting the selection towards those with higher fitness values
std::pair<Bird::BirdBrain, double>
selectParent(const std::vector<std::pair<Bird::BirdBrain, double>> &brains) {
	// Calculate the total fitness of the generation
	double totalFitness = 0.0;
	for (const auto &brain : brains) { totalFitness += brain.second; }
	auto targetFitness = librapid::random<double>(0.0, totalFitness);

	// Find the bird which contains the target fitness value
	double currentFitness = 0.0;
	for (const auto &brain : brains) {
		currentFitness += brain.second;
		if (currentFitness >= targetFitness) { return brain; }
	}

	return brains.back();
}

// Produce a new generation of mutated bird brains
std::vector<Bird::BirdBrain>
newGeneration(const std::vector<std::pair<Bird::BirdBrain, double>> &brains) {
	std::vector<Bird::BirdBrain> newBrains;
	newBrains.reserve(brains.size());

	for (auto &_ : brains) {
		// Select the parent brain
		auto parent = selectParent(brains);

		// Copy the brain (each pair is a brain and its fitness)
		Bird::BirdBrain newBrain = parent.first.copy();

		// Mutate the brain
		newBrain.mutate(mutationRate);

		newBrains.push_back(newBrain);
	}

	// Keep the best bird from the previous generation to prevent the birds getting worse between
	// generations
	auto best	 = bestBird(brains);
	newBrains[0] = best.first.copy();

	return newBrains;
}
