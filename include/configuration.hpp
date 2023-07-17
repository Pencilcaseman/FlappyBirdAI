#pragma once

#include <librapid>
#include <surge/surge.hpp>

static constexpr double GRAVITY							= 1000;
static constexpr int64_t NUM_BIRDS						= 10000;
static constexpr int64_t NUM_WALLS						= 10;
static constexpr double WALL_GAP_SIZE					= 150;
static constexpr double WALL_WIDTH						= 75;
static constexpr double WALL_SPACING					= WALL_WIDTH + 300;
static constexpr double WALL_SPEED						= 115;
static constexpr double WALL_ACCELERATION				= 4;
static constexpr double WALL_BUFFER						= 25;
static constexpr double WALL_SPEED_DISTANCE_COEFFICIENT = 0.01;

static double generationStartTime = 0;
static double worldSpeed		  = 2.0;
static int64_t generationNumber	  = 0;
static double mutationRate		  = 0.1;

using Scalar  = float;
using Backend = librapid::backend::CPU;

#include "utils.hpp"
#include "brain.hpp"
#include "wall.hpp"
#include "bird.hpp"
#include "generation.hpp"
