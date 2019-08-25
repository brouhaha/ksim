// ksim: an 8080 simulator
// Copyright 2019 Eric Smith <spacewar@gmail.com>
// SPDX-License-Identifier: GPL-3.0

#ifndef BDOS_H
#define BDOS_H

typedef enum
{
  BDOS_P_TERMCPM    =  0,
  BDOS_C_READ       =  1,
  BDOS_C_WRITE      =  2,
  BDOS_A_READ       =  3,
  BDOS_A_WRITE      =  4,
  BDOS_L_WRITE      =  5,
  BDOS_C_RAWIO      =  6,  // CP/M 1.3: return base address of CCP
  BDOS_C_GET_IOBYTE =  7,  // CP/M 3:   A_STATIN
  BDOS_C_SET_IOBYTE =  8,  // CP/M 3:   A_STATOUT
  BDOS_C_WRITESTR   =  9,
  BDOS_C_READSTR    = 10,
  BDOS_C_STAT       = 11,
  BDOS_S_BDOSVER    = 12,  // CP/M 1.3: lift head
  BDOS_DRV_ALLRESET = 13,
  BDOS_DRV_SET      = 14,
  BDOS_F_OPEN       = 15,
  BDOS_F_CLOSE      = 16,
  BDOS_F_SFIRST     = 17,
  BDOS_F_SNEXT      = 18,
  BDOS_F_DELETE     = 19,
  BDOS_F_READ       = 20,
  BDOS_F_WRITE      = 21,
  BDOS_F_MAKE       = 22,
  BDOS_F_RENAME     = 23,
  BDOS_DRV_LOGINVEC = 24,
  BDOS_DRV_GET      = 25,
  BDOS_F_DMAOFF     = 26,
  BDOS_DRV_ALLOCVEC = 27,
  BDOS_DRV_SETRO    = 28,
  BDOS_DRV_ROVEC    = 29,
  BDOS_F_ATTRIB     = 30,  // CP/M 1.3: set echo mode for C_READ
  BDOS_DRV_DPB      = 31,
  BDOS_F_USERNUM    = 32,
  BDOS_F_READRAND   = 33,
  BDOS_F_WRITERAND  = 34,
  BDOS_F_SIZE       = 35,
  BDOS_F_RANDREC    = 36,
  BDOS_DRV_RESET    = 37,
  BDOS_F_WRITEZF    = 40
} bdos_fn_t;

void bdos_call (void);

#endif // BDOS_H
