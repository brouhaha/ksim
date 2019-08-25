// ksim: an 8080 simulator
// Copyright 2011, 2014 Eric Smith <spacewar@gmail.com>
// SPDX-License-Identifier: GPL-3.0

#ifndef KSIM_H
#define KSIM_H

#include <inttypes.h>
#include <stdbool.h>


extern bool halted;


extern uint16_t pc;
extern uint16_t reg_pair [4];
extern uint8_t reg_a;


#define REG_PAIR_BC (reg_pair [0])
#define REG_PAIR_DE (reg_pair [1])
#define REG_PAIR_HL (reg_pair [2])
#define REG_SP      (reg_pair [3])

// The REG_PAIR macro handles access to reg pairs based on a two
// bit index from an opcode.
#define REG_PAIR(x) (reg_pair [x])

#define REG_IDX_B 0
#define REG_IDX_C 1
#define REG_IDX_D 2
#define REG_IDX_E 3
#define REG_IDX_H 4
#define REG_IDX_L 5
#define REG_IDX_M 6
#define REG_IDX_A 7

// The REG macro handles access to A, B, C, D, E, H, L, based on a
// three bit index from an opcode.  It does NOT handle "M" (memory)!
// $$$ The ^1 needs to be adjusted for endianness
#define REG(x) (*((x == REG_IDX_A) ? (& reg_a) : (((uint8_t *)(& reg_pair)) + (x^1))))

#define REG_B REG(REG_IDX_B)
#define REG_C REG(REG_IDX_C)
#define REG_D REG(REG_IDX_D)
#define REG_E REG(REG_IDX_E)
#define REG_H REG(REG_IDX_H)
#define REG_L REG(REG_IDX_L)
#define REG_A REG(REG_IDX_A)

extern uint8_t mem [0x10000];

static inline uint8_t mem_read (uint16_t addr)
{
  return mem [addr];
}

static inline void mem_write (uint16_t addr, uint8_t data)
{
  mem [addr] = data;  // $$$ may need to change if there is any ROM,
                      // memory-mapped I/O, or nonexistent memory
}

#endif // KSIM_H
