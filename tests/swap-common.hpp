#pragma once
#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

#include "leonutils/Chrono.hpp"
#include "leonutils/Converts.hpp"
#include "leonutils/ShmBuffer.hpp"

using namespace leon_utl;
using namespace std::chrono;
using namespace std::chrono_literals;
using namespace std::this_thread;

using abool_t = std::atomic_bool;
using ms = std::chrono::milliseconds;
using str_cr = str_t const&;
using str_t = std::string;

enum SwapState_t : uint64_t {
	Pluged = 0,
	Filled,
	Readed,
	Leaved,
	Swaped,
};

using AtmState_t = std::atomic<SwapState_t>;

constexpr size_t BUF_BYTES = 1024UL * 1024 * 512;
// constexpr size_t BUF_BYTES = 2048UL * 1024 * 1024;

static const str_t BUF_NAME = "test_shm_swapout";

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
