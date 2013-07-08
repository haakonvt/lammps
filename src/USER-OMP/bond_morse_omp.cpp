/* ----------------------------------------------------------------------
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

#include "bond_morse_omp.h"
#include "atom.h"
#include "comm.h"
#include "force.h"
#include "neighbor.h"
#include "domain.h"

#include <math.h>

#include "suffix.h"
using namespace LAMMPS_NS;

/* ---------------------------------------------------------------------- */

BondMorseOMP::BondMorseOMP(class LAMMPS *lmp)
  : BondMorse(lmp), ThrOMP(lmp,THR_BOND)
{
  suffix_flag |= Suffix::OMP;
}

/* ---------------------------------------------------------------------- */

void BondMorseOMP::compute(int eflag, int vflag)
{

  if (eflag || vflag) {
    ev_setup(eflag,vflag);
  } else evflag = 0;

  const int nall = atom->nlocal + atom->nghost;
  const int nthreads = comm->nthreads;
  const int inum = neighbor->nbondlist;

#if defined(_OPENMP)
#pragma omp parallel default(none) shared(eflag,vflag)
#endif
  {
    int ifrom, ito, tid;

    loop_setup_thr(ifrom, ito, tid, inum, nthreads);
    ThrData *thr = fix->get_thr(tid);
    thr->timer(ThrData::TIME_START);
    ev_setup_thr(eflag, vflag, nall, eatom, vatom, thr);

    if (inum > 0) {
      if (evflag) {
        if (eflag) {
          if (force->newton_bond) eval<1,1,1>(ifrom, ito, thr);
          else eval<1,1,0>(ifrom, ito, thr);
        } else {
          if (force->newton_bond) eval<1,0,1>(ifrom, ito, thr);
          else eval<1,0,0>(ifrom, ito, thr);
        }
      } else {
        if (force->newton_bond) eval<0,0,1>(ifrom, ito, thr);
        else eval<0,0,0>(ifrom, ito, thr);
      }
    }
    thr->timer(ThrData::TIME_BOND);
    reduce_thr(this, eflag, vflag, thr);
  } // end of omp parallel region
}

template <int EVFLAG, int EFLAG, int NEWTON_BOND>
void BondMorseOMP::eval(int nfrom, int nto, ThrData * const thr)
{
  int i1,i2,n,type;
  double delx,dely,delz,ebond,fbond;
  double rsq,r,dr,ralpha;

  const dbl3_t * _noalias const x = (dbl3_t *) atom->x[0];
  dbl3_t * _noalias const f = (dbl3_t *) thr->get_f()[0];
  const int3_t * _noalias const bondlist = (int3_t *) neighbor->bondlist[0];
  const int nlocal = atom->nlocal;

  for (n = nfrom; n < nto; n++) {
    i1 = bondlist[n].a;
    i2 = bondlist[n].b;
    type = bondlist[n].t;

    delx = x[i1].x - x[i2].x;
    dely = x[i1].y - x[i2].y;
    delz = x[i1].z - x[i2].z;

    rsq = delx*delx + dely*dely + delz*delz;
    r = sqrt(rsq);

    dr = r - r0[type];
    ralpha = exp(-alpha[type]*dr);

    // force & energy

    if (r > 0.0) fbond = -2.0*d0[type]*alpha[type]*(1-ralpha)*ralpha/r;
    else fbond = 0.0;

    if (EFLAG) ebond = d0[type]*(1-ralpha)*(1-ralpha);

    // apply force to each of 2 atoms

    if (NEWTON_BOND || i1 < nlocal) {
      f[i1].x += delx*fbond;
      f[i1].y += dely*fbond;
      f[i1].z += delz*fbond;
    }

    if (NEWTON_BOND || i2 < nlocal) {
      f[i2].x -= delx*fbond;
      f[i2].y -= dely*fbond;
      f[i2].z -= delz*fbond;
    }

    if (EVFLAG) ev_tally_thr(this,i1,i2,nlocal,NEWTON_BOND,
                             ebond,fbond,delx,dely,delz,thr);
  }
}
