#pragma once

#include <librapid>

// A layer in the bird's brain
template<typename Array>
class Layer {
public:
	Layer() = default;
	explicit Layer(size_t nodes) : m_nodes(nodes) {}

	size_t m_nodes; // Number of nodes in the layer
	Array m_weight; // Weight matrix
	Array m_bias;	// Bias vector
	Array m_buffer; // A temporary buffer to increase performance
};

// A simple neural network implementation
template<typename Scalar, typename Backend>
class Brain {
public:
	using Array = librapid::Array<Scalar, Backend>;

	Brain()					  = default;
	Brain(const Brain &other) = default;
	Brain(Brain &&other)	  = default;

	Brain &operator=(const Brain &other) = default;
	Brain &operator=(Brain &&other)		 = default;

	Brain &addLayer(size_t nodes) {
		m_layers.emplace_back(nodes);
		return *this;
	}

	// A nicer syntax for adding layers
	Brain &operator<<(size_t nodes) {
		addLayer(nodes);
		return *this;
	}

	// At this point, we assume no more layers will be added, so we can initialize the matrices
	// and vectors for each layer.
	void construct() {
		for (size_t i = 0; i < m_layers.size() - 1; ++i) {
			m_layers[i].m_weight =
			  Array(librapid::Shape({m_layers[i + 1].m_nodes, m_layers[i].m_nodes}));
			m_layers[i].m_bias	 = Array(librapid::Shape({m_layers[i + 1].m_nodes}));
			m_layers[i].m_buffer = Array(librapid::Shape({m_layers[i + 1].m_nodes}));

			// Each weight matrix and bias vector is initialized with random values between -1 and
			// 1.
			librapid::fillRandom(m_layers[i].m_weight, -1.0, 1.0);
			librapid::fillRandom(m_layers[i].m_bias, -1.0, 1.0);
		}
	}

	// Propagate an input vector through the neural network, returning the output
	[[nodiscard]] Array forward(const Array &inputs) {
		m_layers[0].m_buffer = inputs;
		for (size_t i = 0; i < m_layers.size() - 1; ++i) {
			const auto &layer		 = m_layers[i];

			// Dot the weight matrix with the previous layer's output
			m_layers[i + 1].m_buffer = librapid::dot(layer.m_weight, layer.m_buffer);

			// Add the bias vector
			m_layers[i + 1].m_buffer += layer.m_bias;

			// Apply the activation function
			m_activation.forward(m_layers[i + 1].m_buffer, m_layers[i + 1].m_buffer);
		}

		return m_layers.back().m_buffer;
	}

	// Create an exact copy of a brain instance
	Brain copy() const {
		Brain brain;
		for (const auto &layer : m_layers) {
			Layer<Array> newLayer;
			newLayer.m_nodes  = layer.m_nodes;
			newLayer.m_weight = layer.m_weight.copy();
			newLayer.m_bias	  = layer.m_bias.copy();

			brain.m_layers.push_back(newLayer);
		}

		brain.m_activation = m_activation;

		return brain;
	}

	// Mutate the brain's weights and biases with a given probability (the learning rate).
	// The learning rate is a value in the range [0, 1], and represents the probability that a given
	// weight or bias value is mutated. When mutated, a value is changed to a new random value.
	void mutate(double learningRate) {
		for (auto &layer : m_layers) {
			for (int64_t i = 0; i < layer.m_weight.shape().size(); ++i) {
				if (librapid::random<double>(0, 1) < learningRate) {
					// layer.m_weight.storage()[i] = librapid::random<double>(-1, 1);
					layer.m_weight.storage()[i] += librapid::randomGaussian<double>();
				}
			}

			for (int64_t i = 0; i < layer.m_bias.shape().size(); ++i) {
				if (librapid::random<double>(0, 1) < learningRate) {
					// layer.m_bias.storage()[i] = librapid::random<double>(-1, 1);
					layer.m_bias.storage()[i] += librapid::randomGaussian<double>();
				}
			}
		}
	}

private:
	std::vector<Layer<Array>> m_layers;
	librapid::ml::Sigmoid m_activation;
};
