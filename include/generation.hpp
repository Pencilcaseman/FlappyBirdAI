#pragma once

std::pair<Bird::BirdBrain, double>
bestBird(const std::vector<std::pair<Bird::BirdBrain, double>> &brains) {
	auto best = brains.front();
	for (const auto &brain : brains) {
		if (brain.second > best.second) { best = brain; }
	}
	return best;
}

std::pair<Bird::BirdBrain, double>
selectParent(const std::vector<std::pair<Bird::BirdBrain, double>> &brains) {
	double totalFitness = 0.0;
	for (const auto &brain : brains) { totalFitness += brain.second; }
	auto targetFitness = librapid::random<double>(0, totalFitness);

	double currentFitness = 0.0;
	for (const auto &brain : brains) {
		currentFitness += brain.second;
		if (currentFitness >= targetFitness) {
			return brain;
		}
	}

	return brains.back();
}

std::vector<Bird::BirdBrain>
newGeneration(const std::vector<std::pair<Bird::BirdBrain, double>> &brains) {
	std::vector<Bird::BirdBrain> newBrains;
	newBrains.reserve(brains.size());

	for (auto &_ : brains) {
		auto parent				 = selectParent(brains);
		Bird::BirdBrain newBrain = parent.first.copy();
		newBrain.mutate(mutationRate);
		newBrains.push_back(newBrain);
	}

	// Keep the best bird from the previous generation
	auto best	 = bestBird(brains);
	newBrains[0] = best.first.copy();

	return newBrains;
}
