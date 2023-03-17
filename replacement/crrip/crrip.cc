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

#include "cache.h"

#define maxRRPV 3

// initialize replacement state
void CACHE::initialize_replacement()
{
  for (auto& blk : block)
    blk.lru = maxRRPV;
}

// find replacement victim
uint32_t CACHE::find_victim(uint32_t cpu, uint64_t instr_id, uint32_t set, const BLOCK* current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
  // look for the maxRRPV line
  auto begin = std::next(std::begin(block), set * NUM_WAY);
  auto end = std::next(begin, NUM_WAY);
  auto victim = std::find_if(begin, end, [](BLOCK x) { return x.lru == maxRRPV; }); // hijack the lru field
  while (victim == end) {
    for (auto it = begin; it != end; ++it)
      it->lru++;

    victim = std::find_if(begin, end, [](BLOCK x) { return x.lru == maxRRPV; });
  }

  return std::distance(begin, victim);
}

// called on every cache hit and cache fill
void CACHE::update_replacement_state(uint32_t cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type,
                                     uint8_t hit)
{
  if (hit) {
    block[set * NUM_WAY + way].lru = 0;
  } else {
    block[set * NUM_WAY + way].lru = block[set * NUM_WAY + way].crit == std::numeric_limits<uint8_t>::max() ? maxRRPV - 1 : maxRRPV - 2;
  }
}

// use this function to print out your own stats at the end of simulation
void CACHE::replacement_final_stats() {}
