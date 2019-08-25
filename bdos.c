// ksim: an 8080 simulator
// Copyright 2019 Eric Smith <spacewar@gmail.com>
// SPDX-License-Identifier: GPL-3.0

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ksim.h"
#include "bdos.h"

typedef union
{
  uint8_t bytes[36];
  struct
  {
    uint8_t drive;         // 0 = default, 1..16 = drive A..P
    uint8_t filename[8];   // ASCII upper case, MSB = 0, space-filled
    uint8_t extension[3];  // ASCII upper case, MSB = 0, space-filled
    uint8_t extent_num;    // set to zero on call to F_OPEN
    uint8_t s1;
    uint8_t s2;            // set to zero on call to F_OPEN, F_MAKE, search
    uint8_t rc;            // record count for extent
    uint8_t dn[16];
    uint8_t cr;            // current record number, set to zero
    uint8_t r[3];          // random access record number for F_READRAND, F_WRITERAND
  };
} fcb_t;

static int dmaoff = 0x0080;  // "DMA" address

static void get_fcb(uint16_t fcb_address, fcb_t *fcb)
{
  for (unsigned int i = 0; i < sizeof(fcb_t); i++)
    fcb->bytes[i] = mem_read(fcb_address + i);
}

static void put_fcb(uint16_t fcb_address, fcb_t *fcb, bool random)
{
  unsigned int size;
  if (random)
    size = 36;
  else
    size = 33;
  for (unsigned int i = 0; i < size; i++)
    mem_write(fcb_address + i, fcb->bytes[i]);
}

static void get_filename_from_fcb(fcb_t *fcb, char *fn)
{
  bool has_extension = false;

  // XXX currently ignores drive letter
  for (unsigned int i = 0; i < sizeof(fcb->filename); i++)
  {
    char c = fcb->filename[i];
    if (c == ' ')
      break;
    *(fn++) = c;
  }
  for (unsigned int i = 0; i < sizeof(fcb->extension); i++)
  {
    char c = fcb->extension[i];
    if (c == ' ')
      break;
    if (! has_extension)
    {
      has_extension = true;
      *(fn++) = '.';
    }
    *(fn++) = c;
  }
  *(fn++) = '\0';
}


#define FILE_MAGIC 0x454c4946

typedef struct
{
  uint32_t magic;
  uint32_t check;
  FILE *f;		// typically 4 or 8 bytes
} file_handle_t;

static void init_file_handle(fcb_t *fcb, FILE *f)
{
  file_handle_t *handle = (file_handle_t *) & fcb->dn;
  uint64_t fp;

  handle->magic = FILE_MAGIC;
  handle->f = f;
  fp = (uint64_t) handle->f;
  handle->check = (fp >> 32) ^ fp;
}

static void kill_file_handle(fcb_t *fcb)
{
  file_handle_t *handle = (file_handle_t *) & fcb->dn;
  memset(handle, 0, sizeof(file_handle_t));
}

static FILE *get_file_from_handle(fcb_t *fcb)
{
  file_handle_t *handle = (file_handle_t *) & fcb->dn;
  uint64_t fp;

  if (handle->magic != FILE_MAGIC)
    return NULL;
  fp = (uint64_t) handle->f;
  if (handle->check != ((fp >> 32) ^ fp))
    return NULL;
  return handle->f;
}


// get a character from console into A
static void bdos_c_read(void)
{
  fprintf(stderr, "BDOS C_READ not yet implemented\n");
  // XXX
}

// output character in E to console
static void bdos_c_write(void)
{
  uint8_t c = REG_E;
  write (STDOUT_FILENO, & c, 1);
}

// output character in E to list device
static void bdos_l_write(void)
{
  fprintf(stderr, "BDOS L_WRITE not yet implemented\n");
  // uint8_t c = REG_E;
  // XXX
}

// output message pointed to by DE to console
static void bdos_c_writestr(void)
{
  uint8_t c;
  for (uint16_t addr = REG_PAIR_DE; (c = mem_read (addr)) != '$'; addr++)
    write (STDOUT_FILENO, & c, 1);
}

// read a string from console into buffer pointed to by DE
// first byte of buffer is maximum number of characters buffer can hold
// second byte of buffer returns actual number of characters read
static void bdos_c_readstr(void)
{
  // XXX
  fprintf(stderr, "BDOS C_READSTR not yet implemented\n");
}

// open a file, FCB pointed to by DE
// '?' in filename or extension is wildcard (XXX not implemented)
// returns 0..3 in A if success, 0ffh if not found
static void bdos_f_open(void)
{
  fcb_t fcb;
  char fn[30];
  FILE *f;
  
  get_fcb(REG_PAIR_DE, & fcb);
  get_filename_from_fcb(& fcb, fn);
  f = fopen(fn, "r+");
  if (! f)
    {
      REG_A = 0xff;
      return;
    }
  fcb.s1 = 0;
  fcb.s2 = 0;
  fcb.rc = 0;
  fcb.cr = 0;
  init_file_handle(& fcb, f);
  f = NULL;
  put_fcb(REG_PAIR_DE, & fcb, false);
}

// open a file, FCB pointed to by DE
// returns 0..3 in A if success, 0ffh if error
static void bdos_f_close(void)
{
  fcb_t fcb;
  FILE *f;

  get_fcb(REG_PAIR_DE, & fcb);
  f = get_file_from_handle(& fcb);
  if (f)
  {
    fprintf(stderr, "f_close on non-open file\n");
    return;
  }
  fclose(f);
  kill_file_handle(& fcb);
}

// delete a file, FCB pointed to by DE
// '?' in filename or extension is wildcard (XXX not implemented)
// returns 0..3 in A if success, 0ffh if error
static void bdos_f_delete(void)
{
  fprintf(stderr, "BDOS F_DELETE not yet implemented\n");
  // XXX
}

// read sequention from file, FCB pointed to by DE
// returns 0 in A if success, non-zero if error
static void bdos_f_read(void)
{
  fcb_t fcb;
  FILE *f;
  size_t s;
  uint8_t buf[128];

  get_fcb(REG_PAIR_DE, & fcb);
  f = get_file_from_handle(& fcb);
  if (f)
  {
    fprintf(stderr, "f_read on non-open file\n");
    return;
  }
  memset(buf, 0x1a, sizeof(buf));
  s = fread(buf, 1, sizeof(buf), f);
  if (s == 0)
  {
    REG_A = 0xff;
    return;
  }

  for (unsigned int i = 0; i < sizeof(buf); i++)
    mem_write(dmaoff + i, buf[i]);
  REG_A = 0x00;
}

// write sequention to file, FCB pointed to by DE
// returns 0 in A if success, non-zero if error
static void bdos_f_write(void)
{
  fcb_t fcb;
  FILE *f;
  size_t s;
  uint8_t buf[128];

  get_fcb(REG_PAIR_DE, & fcb);
  f = get_file_from_handle(& fcb);
  if (f)
  {
    fprintf(stderr, "f_read on non-open file\n");
    return;
  }
  for (unsigned int i = 0; i < sizeof(buf); i++)
    buf[i] = mem_read(dmaoff + i);
  s = fwrite(buf, sizeof(buf), 1, f);
  if (s != 1)
  {
    REG_A = 0xff;
    return;
  }

  REG_A = 0x00;
}

// create a file, FCB pointed to by DE
// returns 0..3 in A if success, 0ffh if error
static void bdos_f_make(void)
{
  fcb_t fcb;
  char fn[30];
  FILE *f;
  
  get_fcb(REG_PAIR_DE, & fcb);
  get_filename_from_fcb(& fcb, fn);
  f = fopen(fn, "w+");
  if (! f)
    {
      REG_A = 0xff;
      return;
    }
  fcb.s1 = 0;
  fcb.s2 = 0;
  fcb.rc = 0;
  fcb.cr = 0;
  init_file_handle(& fcb, f);
  f = NULL;
  put_fcb(REG_PAIR_DE, & fcb, false);
}

// set "DMA" address to value in DE
static void bdos_f_dmaoff(void)
{
  dmaoff = REG_PAIR_DE;
}

// random-access read, FCB pointed to by DE
// retruns 0 in A if success
//   01 reading unwritten data
//   03 cannot close current extent
//   04 seek to unwritten extent
//   06 seek past physical end of disk
static void bdos_f_readrand(void)
{
  fprintf(stderr, "BDOS F_READRAND not yet implemented\n");
  // XXX
}

void bdos_call (void)
{
  switch (REG_C)
    {
    case BDOS_C_READ:      bdos_c_read();      break;
    case BDOS_C_WRITE:     bdos_c_write();     break;
    case BDOS_L_WRITE:     bdos_l_write();     break;
    case BDOS_C_WRITESTR:  bdos_c_writestr();  break;
    case BDOS_C_READSTR:   bdos_c_readstr();   break;
    case BDOS_F_OPEN:      bdos_f_open();      break;
    case BDOS_F_CLOSE:     bdos_f_close();     break;
    case BDOS_F_DELETE:    bdos_f_delete();    break;
    case BDOS_F_READ:      bdos_f_read();      break;
    case BDOS_F_WRITE:     bdos_f_write();     break;
    case BDOS_F_MAKE:      bdos_f_make();      break;
    case BDOS_F_DMAOFF:    bdos_f_dmaoff();    break;
    case BDOS_F_READRAND:  bdos_f_readrand();  break;
    default:
      printf ("unrecognized BDOS function %02x pc=%04x\n", REG_C, pc);
      printf ("BC=%04x\n", REG_PAIR_BC);
      printf ("B=%02x\n", REG_B);
      printf ("C=%02x\n", REG_C);
      halted = true;
    }
}

