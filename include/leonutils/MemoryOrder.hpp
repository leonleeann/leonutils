#pragma once
#include <atomic>

using abool_t = std::atomic_bool;

using ai16_t = std::atomic_int16_t;
using ai32_t = std::atomic_int32_t;
using ai64_t = std::atomic_int64_t;

using au16_t = std::atomic_uint16_t;
using au32_t = std::atomic_uint32_t;
using au64_t = std::atomic_uint64_t;

namespace leon_utl {

inline constexpr std::memory_order mo_relaxed = std::memory_order::relaxed;
// inline constexpr std::memory_order mo_consume = std::memory_order::consume;
inline constexpr std::memory_order mo_acquire = std::memory_order::acquire;
inline constexpr std::memory_order mo_release = std::memory_order::release;
inline constexpr std::memory_order mo_acq_rel = std::memory_order::acq_rel;
inline constexpr std::memory_order mo_seq_cst = std::memory_order::seq_cst;

};  // namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
