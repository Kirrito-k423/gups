## GUPS

1. delete MPI part in `gups_vanillla.c` to get `gups_single.c` file which can get the same result in page walk overhead using perf
2. `gups_simple.c` is the simplest one-core version code.

```bash
# compile
make
# test tlb
# shaojiemike @ snode6 in ~/github/PIA_huawei/investigation/pagewalk on git:main x [14:48:14]
$ ./tlbstat -c '/staff/shaojiemike/github/gups/gups 30 100000 4096'                      
command is /staff/shaojiemike/github/gups/gups 30 100000 4096
K_CYCLES   K_INSTR      IPC DTLB_WALKS ITLB_WALKS K_DTLBCYC  K_ITLBCYC  DTLB% ITLB%
25908875   11484493    0.44 48799603   9548956    2603091    205007     10.05  0.79
25940172   7028750     0.27 261267553  2457476    26588222   61496      102.50  0.24
25888783   8446009     0.33 175005801  6043660    17887257   138348     69.09  0.53
```

## Old README

GUPS distribution - 13 Oct 2006

This directory contains several implementations of an algorithm that
can be used to run the HPCC RandomAccess (GUPS) benchmark.

The algorithm is described on this WWW page:
www.cs.sandia.gov/~sjplimp/algorithms/html#gups

The tar file of codes can be downloaded from this WWW page:
www.cs.sandia.gov/~sjplimp/download.html

These codes are distributed by Steve Plimpton
of Sandia National Laboratories:
sjplimp@sandia.gov, www.cs.sandia.gov/~sjplimp

--------------------------------------------------------------------------

This directory should contain the following files:

gups_vanilla.c	      vanilla power-of-2 version of algorithm
gups_nonpow2.c	      non-power-of-2 version
gups_opt.c	      optimized power-of-2 version

MPIRandomAccess_vanilla.c	implementation of gups_vanilla in HPCC harness
MPIRandomAccess_opt.c		implementation of gups_opt in HPCC harness

Makefile.*	      Makefiles for various machines

--------------------------------------------------------------------------

The gups_* files are stand-alone single-file codes that can be built
using a Makefile like those provided.  E.g.

make -f Makefile.linux gups_vanilla

You will need to create a Makefile.* appropriate to your platform,
that points at the correct MPI library, etc.  Note that these 3 codes
support a -DLONG64 C compiler flag.  If a "long" on your processor is
32-bit (presumably long long is 64 bits), then don't use -DLONG64; if
a "long" is 64 bits, then use -DLONG64.

--------------------------------------------------------------------------

You can run any of the 3 gups* codes as follows:

1 proc:
gups_vanilla N M chunk

P procs:
mpirun -np P gups_vanilla N M chunk

where

N = length of global table is 2^N
M = # of update sets per proc
chunk = # of updates in one set on each proc

Note that 2^N is the length of the global table across all processors.
Thus N = 30 would run with a billion-element table.

Chunk is the number of updates each proc will do before communicating.
In the official HPCC benchmark this is specified to be no larger than
1024, but you can run the code with any value you like.  Your GUPS
performance will typically decrease for smaller chunk size.

When each proc performs "chunk" updates, that is one "set" of updates.
M determines how many sets are performed.  The GUPS performance is a
"rate", so it's independent of M, once M is large enough to get good
statistics.  So you can start your testing with a small M to see how
fast your machine runs with this algorithm, then get better stats with
longer runs with a larger M.  An official HPCC benchmark run requires
M be a large number (like the total number of updates = 4x the table
size, if I recall), but your GUPS rate won't change.

After the code runs, it will print out some stats, like this:
> mpirun -np 2 gups_vanilla 20 1000 1024
Number of procs: 2
Vector size: 1048576
Max datums during comm: 1493
Max datums after comm: 1493
Excess datums (frac): 39395 (0.0192358)
Bad locality count: 0
Update time (secs):     0.383
Gups:  0.005351

"Vector size" is the length of the global table.

The "max datums" values tell how message size varied as datums were
routed thru the hypercube dimensions.  They should only exceed "chunk"
by a modest amount.  However the random number generation in the HPCC
algorithm is not very random, so in the first few iterations a few
procs tend to receive larger messages.

The "excess datums" value is the number of updates (and fraction) that
would have been missed if datums greater than the chunk size were
discarded.  It should typically be < 1% for long runs.  The codes do
not discard these excess updates.

The "bad locality" should be 0.  If the code was compiled with -DCHECK
and a non-zero value results, it means some procs are trying to
perform table updates on table indices they don't own, so something is
wrong.

The "update time" is how long the code ran.

"Gups" is the GUPS performance rate, as HPCC defines it.  Namely the
total # of updates per second across all processors.  The total # of
updates is M*chunk*P, where P = # of processors.  Once you run on
enough processors (e.g. 32), you should see the GUPS rate nearly
double each time you double the number of procs, unless communication
on your machine is slowing things down.

--------------------------------------------------------------------------

The MPIRandomAccess*.c codes are versions of the same algorithms
implemented within the framework that HPCC provides to enable users to
implement new optimized algorithms.

In principle you can take these files and drop them into the HPCC
harness, re-compile the HPCC suite, and run an official HPCC benchmark
test with the new algorithms.  In practice, I don't know the specifics
of how to do that!  Courtenay Vaughan at Sandia was the one who worked
on that part of the project.  You can email him if you have questions
at ctvaugh@sandia.gov.

You should get essentially the same GUPS number when running these
algorithms in the HPCC harness as you get with the stand-alone codes.

Note that we have only ported the vanilla and opt algorithms (not the
non-power-of-2 version) to the HPCC framework.
