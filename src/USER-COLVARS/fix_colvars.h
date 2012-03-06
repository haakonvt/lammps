/* -*- c++ -*- ----------------------------------------------------------
   LAMMPS - Large-scale Atomic/Molecular Massively Parallel Simulator
   http://lammps.sandia.gov, Sandia National Laboratories
   Steve Plimpton, sjplimp@sandia.gov

   Copyright (2003) Sandia Corporation.  Under the terms of Contract
   DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
   certain rights in this software.  This software is distributed under 
   the GNU General Public License.

   See the README file in the top-level LAMMPS directory.
------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
      Contributing author: Axel Kohlmeyer (Temple U)
------------------------------------------------------------------------- */

#ifdef FIX_CLASS

FixStyle(colvars,FixColvars)

#else

#ifndef LMP_FIX_COLVARS_H
#define LMP_FIX_COLVARS_H

#include "fix.h"

// forward declaration
class colvarproxy_lammps;

namespace LAMMPS_NS {

class FixColvars : public Fix {

 public:
  FixColvars(class LAMMPS *, int, char **);
  ~FixColvars();

  int setmask();
  void init();
  void setup(int);
  void post_force(int); 
  void post_force_respa(int, int, int);
  void post_run();
  double memory_usage();

 protected:
  void deallocate();     // free internal buffers

 protected:
  class colvarproxy_lammps *proxy; // pointer to the colvars proxy class
  char *conf_file;     // name of colvars config file
  char *inp_name;      // name/prefix of colvars restart file
  char *out_name;      // prefix string for all output files
  char *tmp_name;      // name of thermostat fix.
  int   rng_seed;      // seed to initialize random number generator 
  int   thermo_id;     // id of the thermostat fix

  int   me;            // my MPI rank in this "world".
  int   num_coords;    // total number of atoms controlled by this fix

  int   size_one;      // bytes per atom in communication buffer.
  int   maxbuf;        // size of atom communication buffer.

  void *coord_buf;     // coordinate communication buffer
  void *force_buf;     // force data buffer

  void *idmap;         // hash for mapping atom indices to consistent order.
  int  *rev_idmap;     // list of the hash keys for reverse mapping.

  int   nlevels_respa; // flag to determine respa levels.

  int   msglen;
  char *msgdata;
};

}

#endif
#endif

// Local Variables:
// mode: c++
// compile-command: "make -j4 openmpi"
// c-basic-offset: 2
// fill-column: 76
// indent-tabs-mode: nil
// End:
