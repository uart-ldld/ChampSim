/*
 * Copyright (C) 2023  Xiaoyue Chen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include <iterator>
#include <numeric>
#include <vector>

#include "cache.h"
#include "util.h"

void CACHE::initialize_replacement() {}

// find replacement victim
uint32_t CACHE::find_victim(uint32_t cpu, uint64_t instr_id, uint32_t set, const BLOCK* current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
  auto candidates = std::vector<uint32_t>(NUM_WAY);
  std::iota(std::begin(candidates), std::end(candidates), 0);
  candidates.erase(std::remove_if(std::begin(candidates), std::end(candidates), [=](uint32_t way) { return current_set[way].crit < MSHR.begin()->crit; }),
                   std::end(candidates));

  // all ways are more critical than the new line, fall back on lru
  if (candidates.empty()) {
    candidates.resize(NUM_WAY);
    std::iota(std::begin(candidates), std::end(candidates), 0);
  }

  return *std::max_element(std::begin(candidates), std::end(candidates),
                           [=](uint32_t lhs, uint32_t rhs) { return lru_comparator<BLOCK, BLOCK>()(current_set[lhs], current_set[rhs]); });
}

// called on every cache hit and cache fill
void CACHE::update_replacement_state(uint32_t cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type,
                                     uint8_t hit)
{
  if (hit && type == WRITEBACK)
    return;

  auto begin = std::next(block.begin(), set * NUM_WAY);
  auto end = std::next(begin, NUM_WAY);
  uint32_t hit_lru = std::next(begin, way)->lru;
  std::for_each(begin, end, [hit_lru](BLOCK& x) {
    if (x.lru <= hit_lru)
      x.lru++;
  });
  std::next(begin, way)->lru = 0; // promote to the MRU position
}

void CACHE::replacement_final_stats() {}
