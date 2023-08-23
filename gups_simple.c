/* ----------------------------------------------------------------------
   gups = algorithm for the HPCC RandomAccess (GUPS) benchmark
          implements a hypercube-style synchronous all2all

   Steve Plimpton, sjplimp@sandia.gov, Sandia National Laboratories
   www.cs.sandia.gov/~sjplimp
   Copyright (2006) Sandia Corporation
------------------------------------------------------------------------- */

/* random update GUPS code, power-of-2 number of procs
   compile with -DCHECK to check if table updates happen on correct proc */

#include <stdio.h>
#include <stdlib.h>
// #include <mpi.h>
#include <zsim_hooks.h>

#define MAX(a,b) ((a) > (b) ? (a) : (b))

/* machine defs
   compile with -DLONG64 if a "long" is 64 bits
   else compile with no setting if "long long" is 64 bit */

#ifdef LONG64
#define POLY 0x0000000000000007UL
#define PERIOD 1317624576693539401L
#define ZERO64B 0L
typedef long s64Int;
typedef unsigned long u64Int;
#else
#define POLY 0x0000000000000007ULL
#define PERIOD 1317624576693539401LL
#define ZERO64B 0LL
typedef long long s64Int;
typedef unsigned long long u64Int;
#endif

u64Int HPCC_starts(s64Int n);

int main(int narg, char **arg)
{
  int me=0,nprocs=1;
  int i,j,iterate,niterate;
  int nlocal,nlocalm1,logtable,index,logtablelocal;
  int logprocs,ipartner,ndata,nsend,nkeep,nrecv,maxndata,maxnfinal,nexcess;
  int nbad,chunk,chunkbig;
  double t0,t0_all,Gups;
  u64Int *table,*data;
  u64Int ran,datum,procmask,nglobal,offset;
  u64Int ilong,nexcess_long,nbad_long;

  zsim_roi_begin();

  /* command line args = N M chunk
     N = length of global table is 2^N
     M = # of update sets per proc
     chunk = # of updates in one set */

  if (narg != 4) {
    printf("Syntax: gups N M chunk\n");
  }

  logtable = atoi(arg[1]);
  niterate = atoi(arg[2]);
  chunk = atoi(arg[3]);


  /* nglobal = entire table
     nlocal = size of my portion
     nlocalm1 = local size - 1 (for index computation)*/

  nglobal = ((u64Int) 1) << logtable;
  nlocal = nglobal / nprocs;
  nlocalm1 = nlocal - 1;

  /* allocate local memory
     16 factor insures space for messages that (randomly) exceed chunk size */

  chunkbig = 16*chunk;

  table = (u64Int *) malloc(nlocal*sizeof(u64Int));
  data = (u64Int *) malloc(chunkbig*sizeof(u64Int));

  if (!table || !data ) {
    if (me == 0) printf("Table allocation failed\n");
  }

	zsim_work_begin();


  /* loop:
       generate chunk random values per proc
       communicate datums to correct processor via hypercube routing
       use received values to update local table */

  // loops
  ran = HPCC_starts(0);
  for (iterate = 0; iterate < niterate; iterate++) {

    // STEP1: generate chunk random values per proc
    for (i = 0; i < chunk; i++) {
      ran = (ran << 1) ^ ((s64Int) ran < ZERO64B ? POLY : ZERO64B);
      data[i] = ran;
    }
    ndata = chunk;

    // STEP2: communicate datums to correct processor via hypercube routing, skip this because procs=1

    // Step3: use received values to update local table
    for (i = 0; i < ndata; i++) {
      datum = data[i];
      index = datum & nlocalm1;
      // Important: beacause data[] contains random numbers, so the index is random address.
      table[index] ^= datum;
    }

  }

  zsim_work_end();

  if (me == 0) {
    printf("Number of procs: %d\n",nprocs);
    printf("Vector size: %lld\n",nglobal);
  }

  /* clean up */

  free(table);
  free(data);

  zsim_roi_end();

}

/* start random number generator at Nth step of stream
   routine provided by HPCC */

u64Int HPCC_starts(s64Int n)
{
  int i, j;
  u64Int m2[64];
  u64Int temp, ran;

  while (n < 0) n += PERIOD;
  while (n > PERIOD) n -= PERIOD;
  if (n == 0) return 0x1;

  temp = 0x1;
  for (i=0; i<64; i++) {
    m2[i] = temp;
    temp = (temp << 1) ^ ((s64Int) temp < 0 ? POLY : 0);
    temp = (temp << 1) ^ ((s64Int) temp < 0 ? POLY : 0);
  }

  for (i=62; i>=0; i--)
    if ((n >> i) & 1)
      break;

  ran = 0x2;
  while (i > 0) {
    temp = 0;
    for (j=0; j<64; j++)
      if ((ran >> j) & 1)
        temp ^= m2[j];
    ran = temp;
    i -= 1;
    if ((n >> i) & 1)
      ran = (ran << 1) ^ ((s64Int) ran < 0 ? POLY : 0);
  }

  return ran;
}
