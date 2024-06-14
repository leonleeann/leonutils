#pragma once
#include <atomic>

namespace leon_utl {

inline constexpr std::memory_order mo_relaxed = std::memory_order::relaxed;
// inline constexpr std::memory_order mo_consume = std::memory_order::consume;
inline constexpr std::memory_order mo_acquire = std::memory_order::acquire;
inline constexpr std::memory_order mo_release = std::memory_order::release;
inline constexpr std::memory_order mo_acq_rel = std::memory_order::acq_rel;
inline constexpr std::memory_order mo_seq_cst = std::memory_order::seq_cst;

};  // namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
