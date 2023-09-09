#pragma once

#include <librapid>
#include <surge/surge.hpp>

static constexpr double GRAVITY							= 0.125;			// Bird gravity
static constexpr int64_t NUM_BIRDS						= 750;				// Number of birds
static constexpr int64_t NUM_WALLS						= 10;				// Number of walls
static constexpr double BIRD_JUMP_VELOCITY				= 4.8;				// Jump power
static constexpr double WALL_GAP_SIZE					= 200;				// Opening in a wall
static constexpr double WALL_WIDTH						= 75;				// Width of a wall
static constexpr double WALL_SPACING					= WALL_WIDTH + 300; // Space between walls
static constexpr double WALL_SPEED						= 1;				// Speed of the walls
static constexpr double WALL_ACCELERATION				= 0.0005; // Acceleration of the walls
static constexpr double WALL_BUFFER						= 25;  // Gap will never be higher than this
static constexpr double WALL_SPEED_DISTANCE_COEFFICIENT = 1.1; // Walls move apart as they speed up
static constexpr double MAX_WALL_SPEED					= 50;  // Fastest the walls can go

static double generationStartTime = 0; // Time the generation started
static double worldSpeed		  = 1; // Global speed modifier
static int64_t generationNumber	  = 0; // Current generation number
// static double mutationRate		  = 0.1; // Learning/mutation rate
static float mutationRate  = 0.1; // Learning/mutation rate
static double wallDistance = 0;	  // Distance traveled by the walls (used for fitness)

using Scalar  = float;					// Scalar type for computations
using Backend = librapid::backend::CPU; // Backend for librapid

#include "utils.hpp"
#include "brain.hpp"
#include "wall.hpp"
#include "bird.hpp"
#include "generation.hpp"
