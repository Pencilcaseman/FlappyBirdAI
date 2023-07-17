#pragma once

#include <librapid>

template<typename Array>
class Layer {
public:
	Layer() = default;
	explicit Layer(size_t nodes) : m_nodes(nodes) {}

	size_t m_nodes;
	Array m_weight;
	Array m_bias;
	Array m_buffer;
};

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

	Brain &operator<<(size_t nodes) {
		addLayer(nodes);
		return *this;
	}

	void construct() {
		for (size_t i = 0; i < m_layers.size() - 1; ++i) {
			m_layers[i].m_weight =
			  Array(librapid::Shape({m_layers[i + 1].m_nodes, m_layers[i].m_nodes}));
			m_layers[i].m_bias	 = Array(librapid::Shape({m_layers[i + 1].m_nodes}));
			m_layers[i].m_buffer = Array(librapid::Shape({m_layers[i + 1].m_nodes}));

			librapid::fillRandom(m_layers[i].m_weight, -1.0, 1.0);
			librapid::fillRandom(m_layers[i].m_bias, -1.0, 1.0);
		}
	}

	[[nodiscard]] Array forward(const Array &inputs) {
		m_layers[0].m_buffer = inputs;
		for (size_t i = 0; i < m_layers.size() - 1; ++i) {
			const auto &layer		 = m_layers[i];
			m_layers[i + 1].m_buffer = librapid::dot(layer.m_weight, layer.m_buffer);
			m_layers[i + 1].m_buffer += layer.m_bias;
			m_activation.forward(m_layers[i + 1].m_buffer, m_layers[i + 1].m_buffer);
		}

		return m_layers.back().m_buffer;
	}

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

	void mutate(double learningRate) {
		for (auto &layer : m_layers) {
			for (int64_t i = 0; i < layer.m_weight.shape().size(); ++i) {
				if (librapid::random<double>(0, 1) < learningRate) {
					layer.m_weight.storage()[i] = librapid::random<double>(-1, 1);
				}
			}

			for (int64_t i = 0; i < layer.m_bias.shape().size(); ++i) {
				if (librapid::random<double>(0, 1) < learningRate) {
					layer.m_bias.storage()[i] = librapid::random<double>(-1, 1);
				}
			}
		}
	}

private:
	std::vector<Layer<Array>> m_layers;
	librapid::ml::Sigmoid m_activation;
};
