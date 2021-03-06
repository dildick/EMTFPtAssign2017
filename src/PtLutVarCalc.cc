#include "../interface/PtLutVarCalc.h"
#include "../src/PtAssignmentEngineAux2017.cc"

// From here down, exact copy of code used in emulator: L1Trigger/L1TMuonEndCap/src/PtLutVarCalc.cc

PtAssignmentEngineAux2017 ENG;

//From L1Trigger/L1TMuonEndCap/interface/TrackTools.h
double range_phi_deg(double deg) {
	while (deg <  -180.) deg += 360.;
	while (deg >= +180.) deg -= 360.;
	return deg;
}

double calc_phi_loc_deg_from_glob(double glob, int sector) {  // glob phi in deg, sector [1-6]
	glob = range_phi_deg(glob);  // put phi in [-180,180] range
	double loc = glob - 15. - (60. * (sector-1));
	return loc;
}

int calc_phi_loc_int(double glob, int sector) {  // glob phi in deg, sector [1-6]
	double loc = calc_phi_loc_deg_from_glob(glob, sector);
	loc = ((loc + 22.) < 0.) ? loc + 360. : loc;
	loc = (loc + 22.) * 60.;
	int phi_int = static_cast<int>(std::round(loc));
	return phi_int;
}

int calc_theta_int(double theta, int endcap) {  // theta in deg, endcap [-1,+1]
	theta = (endcap == -1) ? (180. - theta) : theta;
	theta = (theta - 8.5) * 128./(45.0-8.5);
	int theta_int = static_cast<int>(std::round(theta));
	return theta_int;
}

int CalcTrackTheta( const int th1, const int th2, const int th3, const int th4,
		    const int st1_ring2, const int mode, const bool BIT_COMP ) {

  int theta = -99;

  if      ( (mode % 8) / 4 > 0 ) // Has station 2 hit
    theta = th2;
  else if ( (mode % 4) / 2 > 0 ) // Has station 3 hit
    theta = th3;
  else if ( (mode % 2) > 0 ) // Has station 4 hit
    theta = th4;

  assert( theta > 0 );

  if (BIT_COMP) {
    int nBits = (mode == 15 ? 4 : 5);
    theta = ENG.getTheta(theta, st1_ring2, nBits);
  }

  return theta;
}


void CalcDeltaPhisGEM( int& dPh12, int& dPh13, int& dPh14, int& dPh23, int& dPh24, int& dPh34, int& dPhSign,
                       int& dPhSum4, int& dPhSum4A, int& dPhSum3, int& dPhSum3A, int& outStPh, int& dPhGE11ME11,
                       const int ph1, const int ph2, const int ph3, const int ph4, const int phGEM, const int mode, const bool BIT_COMP ) {

  CalcDeltaPhis(dPh12,dPh13,dPh14,dPh23,dPh24,dPh34,dPhSign,
                dPhSum4,dPhSum4A,dPhSum3,dPhSum3A,outStPh,
                ph1,  ph2,  ph3,  ph4,  mode,BIT_COMP );

  /*
    AWB: "One other thing: in PtLutVarCalc.cc, the variable dPhGE11ME11 should be set
    to some default value if *either* phGEM *or* ph1 is < 0, and that default
    value should be some constant like -999, rather than being set equal to phGEM (line 71)."
  */
  if (ph1 < 0 or phGEM < 0)
    dPhGE11ME11 = -999;

  /*
    AWB: "Also, the quantity dPhGE11ME11 should be multiplied by -1*dPhSign, so use
    dPhGE11ME11 = (ph1 - phGEM)*dPhSign;  With this convention, when dPhi(GEM-ME1)
    and dPhi(ME1-ME2) are in line, both will have positive values."
  */

  dPhGE11ME11 = (ph1 - phGEM)*dPhSign;

  // probably best not to change the EMTF track mode at this point
}

void CalcPhiRun3( int& ph, int ring, int strip_quart_bit, int strip_eight_bit, int station, int endcap, bool useQuartBit, bool useEighthBit) {

  // if not bit was set, do nothing
  if (!useQuartBit) return;

  /*
  The int phi is corrected by an amount depending on the quart- and eight-strip bits of the position offset.
  To get these correction values, the full strip pitch (which varies by station/ring) is divided by a factor of 4
    (for quart-strip pitch) or 8 (for eight-strip pitch), then converted from degrees to integer units by
    multiplying by a factor of 240 [4 for ES precision * 60 degree sector].
  Lastly, these corrections are either added or subtracted based on the chamber orientation (clockwise vs. counterclockwise).
    St. 1 and 2 have the opposite orientation of St. 3 and 4, and for the opposite endcap these are reversed.
  The values for full strip pitch are tabulated in p.2 of https://arxiv.org/pdf/0911.4992.pdf
  For comparison, conversion of loc phi in degrees to int in the emulator can be found in:
    https://github.com/cms-sw/cmssw/blob/master/L1Trigger/L1TMuonEndCap/interface/TrackTools.h#L201-L207
  */

  if (station == 1) {
    if (ring < 4) {
      if (strip_quart_bit == 1 ) {endcap>0 ? ph = ph + 2 : ph = ph - 2; }
      if (useEighthBit and strip_eight_bit == 1 ) {endcap>0 ? ph = ph + 1 : ph = ph - 1; }
    }

    if (ring == 4) {
      if (strip_quart_bit == 1 ) {endcap>0 ? ph = ph + 3 : ph = ph - 3; }
      if (useEighthBit and strip_eight_bit == 1 ) {endcap>0 ? ph = ph + 1 : ph = ph - 1 ; }
    }
  }

  if (station == 2) {
    if (ring==1) {
      if (strip_quart_bit == 1 ) {endcap>0 ? ph = ph + 4 : ph = ph - 4 ;}
      if (useEighthBit and strip_eight_bit == 1 ) {endcap>0 ? ph = ph + 2 : ph = ph - 2 ;}
    }
    if (ring==2) {
      if (strip_quart_bit == 1 ) {endcap>0 ? ph = ph + 2 : ph = ph - 2 ;}
      if (useEighthBit and strip_eight_bit == 1 ) {endcap>0 ? ph = ph + 1 : ph = ph - 1 ;}
    }
  }

  if (station > 2) {
    if (ring==1) {
      if (strip_quart_bit == 1 ) { endcap>0 ? ph = ph - 4 : ph = ph + 4 ;}
      if (useEighthBit and strip_eight_bit == 1 ) { endcap>0 ? ph = ph - 2 : ph = ph + 2 ; }
    }
    if (ring==2) {
      if (strip_quart_bit == 1 ) { endcap>0 ? ph = ph - 2 : ph = ph + 2 ;}
      if (useEighthBit and strip_eight_bit == 1 ) { endcap>0 ? ph = ph - 1 : ph = ph + 1 ; }
    }
  }
}


void CalcDeltaPhis( int& dPh12, int& dPh13, int& dPh14, int& dPh23, int& dPh24, int& dPh34, int& dPhSign,
                    int& dPhSum4, int& dPhSum4A, int& dPhSum3, int& dPhSum3A, int& outStPh,
                    const int ph1, const int ph2, const int ph3, const int ph4, const int mode, const bool BIT_COMP ) {

  dPh12 = ph2 - ph1;
  dPh13 = ph3 - ph1;
  dPh14 = ph4 - ph1;
  dPh23 = ph3 - ph2;
  dPh24 = ph4 - ph2;
  dPh34 = ph4 - ph3;
  dPhSign = 0;

  if (mode >= 8) {                   // First hit is station 1
    if      ( (mode % 8) / 4 > 0 )   // Has station 2 hit
      dPhSign = (dPh12 >= 0 ? +1 : -1);
    else if ( (mode % 4) / 2 > 0 )   // Has station 3 hit
      dPhSign = (dPh13 >= 0 ? +1 : -1);
    else if ( (mode % 2) > 0 )       // Has station 4 hit
      dPhSign = (dPh14 >= 0 ? +1 : -1);
  } else if ( (mode % 8) / 4 > 0 ) { // First hit is station 2
    if      ( (mode % 4) / 2 > 0 )   // Has station 3 hit
      dPhSign = (dPh23 >= 0 ? +1 : -1);
    else if ( (mode % 2) > 0 )       // Has station 4 hit
      dPhSign = (dPh24 >= 0 ? +1 : -1);
  } else if ( (mode % 4) / 2 > 0 ) { // First hit is station 3
    if      ( (mode % 2) > 0 )       // Has station 4 hit
      dPhSign = (dPh34 >= 0 ? +1 : -1);
  }

  assert(dPhSign != 0);

  dPh12 *= dPhSign;
  dPh13 *= dPhSign;
  dPh14 *= dPhSign;
  dPh23 *= dPhSign;
  dPh24 *= dPhSign;
  dPh34 *= dPhSign;

  if (BIT_COMP) {
    int nBitsA = 7;
    int nBitsB = 7;
    int nBitsC = 7;
    int maxA = 512;
    int maxB = 512;
    int maxC = 512;

    if (mode == 7 || mode == 11 || mode > 12) {
      nBitsB = 5;
      maxB = 256;
      nBitsC = 5;
      maxC = 256;
    }
    if (mode == 15) {
      nBitsC = 4;
      maxC = 256;
    }

    dPh12 = ENG.getNLBdPhi(dPh12, nBitsA, maxA);
    dPh13 = ENG.getNLBdPhi(dPh13, nBitsA, maxA);
    dPh14 = ENG.getNLBdPhi(dPh14, nBitsA, maxA);
    if (mode == 7)
      dPh23 = ENG.getNLBdPhi(dPh23, nBitsA, maxA);
    else
      dPh23 = ENG.getNLBdPhi(dPh23, nBitsB, maxB);
    dPh24 = ENG.getNLBdPhi(dPh24, nBitsB, maxB);
    dPh34 = ENG.getNLBdPhi(dPh34, nBitsC, maxC);

    // Some delta phi values must be computed from others
    switch (mode) {
    case 15:  dPh13 = dPh12 + dPh23;  dPh14 = dPh13 + dPh34;  dPh24 = dPh23 + dPh34;  break;
    case 14:  dPh13 = dPh12 + dPh23;  break;
    case 13:  dPh14 = dPh12 + dPh24;  break;
    case 11:  dPh14 = dPh13 + dPh34;  break;
    case  7:  dPh24 = dPh23 + dPh34;  break;
    default:  break;
    }

  } // End conditional: if (BIT_COMP)


  // Compute summed quantities
  if (mode == 15) CalcDeltaPhiSums( dPhSum4, dPhSum4A, dPhSum3, dPhSum3A, outStPh,
				    dPh12,  dPh13,  dPh14,  dPh23,  dPh24,  dPh34 );

} // End function: CalcDeltaPhis()



void CalcDeltaThetas( int& dTh12, int& dTh13, int& dTh14, int& dTh23, int& dTh24, int& dTh34,
		      const int th1, const int th2, const int th3, const int th4, const int mode, const bool BIT_COMP ) {

  dTh12 = th2 - th1;
  dTh13 = th3 - th1;
  dTh14 = th4 - th1;
  dTh23 = th3 - th2;
  dTh24 = th4 - th2;
  dTh34 = th4 - th3;

  if (BIT_COMP) {
    int nBits = (mode == 15 ? 2 : 3);

    dTh12 = ENG.getdTheta(dTh12, nBits);
    dTh13 = ENG.getdTheta(dTh13, nBits);
    dTh14 = ENG.getdTheta(dTh14, nBits);
    dTh23 = ENG.getdTheta(dTh23, nBits);
    dTh24 = ENG.getdTheta(dTh24, nBits);
    dTh34 = ENG.getdTheta(dTh34, nBits);
  } // End conditional: if (BIT_COMP)

} // CalcDeltaThetas()


void ConvertSlopeToRun2Pattern(const int slope, int& pattern) {
  const unsigned slopeList[32] = {10, 10, 10, 8, 8, 8, 6, 6, 6, 6, 4, 4, 2, 2, 2, 2,
                                  10, 10, 10, 9, 9, 9, 7, 7, 7, 7, 5, 5, 3, 3, 3, 3};
  pattern = slopeList[slope];
}

void ConvertSlopeToRun2Pattern(const int slope1, const int slope2, const int slope3, const int slope4,
                               int& pat1, int& pat2, int& pat3, int& pat4) {
  ConvertSlopeToRun2Pattern(slope1, pat1);
  ConvertSlopeToRun2Pattern(slope2, pat2);
  ConvertSlopeToRun2Pattern(slope3, pat3);
  ConvertSlopeToRun2Pattern(slope4, pat4);
}

void CalcBends( int& bend1, int& bend2, int& bend3, int& bend4,
                int& slope1, int& slope2, int& slope3, int& slope4,
                const int pat1, const int pat2, const int pat3, const int pat4,
                const int pat1_run3, const int pat2_run3, const int pat3_run3, const int pat4_run3,
                const int dPhSign, const int endcap, const int mode, const bool BIT_COMP, const bool isRun2) {


  if(isRun2) {
    bend1 = CalcBendFromPattern( pat1, endcap, isRun2 );
    bend2 = CalcBendFromPattern( pat2, endcap, isRun2 );
    bend3 = CalcBendFromPattern( pat3, endcap, isRun2 );
    bend4 = CalcBendFromPattern( pat4, endcap, isRun2 );
  }

  else {
    bend1 = CalcBendFromPattern( pat1_run3, endcap, isRun2 );
    bend2 = CalcBendFromPattern( pat2_run3, endcap, isRun2 );
    bend3 = CalcBendFromPattern( pat3_run3, endcap, isRun2 );
    bend4 = CalcBendFromPattern( pat4_run3, endcap, isRun2 );
  }

  if (BIT_COMP) {

    int nBits = 3;
    if (mode == 7 || mode == 11 || mode > 12)
      nBits = 2;


    if (isRun2) {
      if (  mode      / 8 > 0 ) // Has station 1 hit
        bend1 = ENG.getCLCT( pat1, endcap, dPhSign, nBits, isRun2 );
      if ( (mode % 8) / 4 > 0 ) // Has station 2 hit
        bend2 = ENG.getCLCT( pat2, endcap, dPhSign, nBits, isRun2 );
      if ( (mode % 4) / 2 > 0 ) // Has station 3 hit
        bend3 = ENG.getCLCT( pat3, endcap, dPhSign, nBits, isRun2 );
      if ( (mode % 2)     > 0 ) // Has station 4 hit
        bend4 = ENG.getCLCT( pat4, endcap, dPhSign, nBits, isRun2 );
    }

    else {
      if (  mode      / 8 > 0 ) // Has station 1 hit
        slope1 = ENG.getCLCT( slope1, endcap, dPhSign, nBits, isRun2 );
      if ( (mode % 8) / 4 > 0 ) // Has station 2 hit
        slope2 = ENG.getCLCT( slope2, endcap, dPhSign, nBits, isRun2 );
      if ( (mode % 4) / 2 > 0 ) // Has station 3 hit
        slope3 = ENG.getCLCT( slope3, endcap, dPhSign, nBits, isRun2 );
      if ( (mode % 2)     > 0 ) // Has station 4 hit
        slope4 = ENG.getCLCT( slope4, endcap, dPhSign, nBits, isRun2 );
    }

  } // End conditional: if (BIT_COMP)

} // End function: CalcBends()

void CalcSlopes( const int bend, int& slope, const int endcap, const int mode, const bool BIT_COMP, const bool isRun2) {

  if (std::abs(slope) > 15) {
    slope = -99;
    return;
  }

  /*
     multiply with bending
     make sure that bending convention is not {0,1}, but {1, -1}!!!
     bend == 0 means left bending, thus negative in CSCPatternBank
     bend == 1 means right bending, thus positive in CSCPatternBank
  */
  slope *= (1- 2*bend);

  /*
    However, in the EMTF we flip the convention, so that
    - positive bending (left)
    - negative bending (right)
  */
  slope *= -1;

  // Reverse to match dPhi convention
  if (endcap == -1)
    slope *= -1;

  if (BIT_COMP) {

    int nBits = 3;
    if (mode == 7 || mode == 11 || mode > 12)
      nBits = 2;

    if (  mode      / 8 > 0 ) // Has station 1 hit
      slope = ENG.getCLCT( slope, endcap, 0, nBits, isRun2 );

    //std::cout << "Slope after compression: " << slope << std::endl;
    //std::cout << "---Next muon---" << std::endl;
  }

  assert( slope != -99 );
}

void CalcDeltaSlopes(const int slope1, const int slope2,
                     const int slope3, const int slope4,
                     int& dSlope12, int& dSlope13,
                     int& dSlope14, int& dSlope23,
                     int& dSlope24, int& dSlope34,
                     int& dSlopeSum4, int& dSlopeSum4A,
                     int& dSlopeSum3, int& dSlopeSum3A,
                     int& outStSlope) {
  dSlope12 = slope2 - slope1;
  dSlope13 = slope3 - slope1;
  dSlope14 = slope4 - slope1;
  dSlope23 = slope3 - slope2;
  dSlope24 = slope4 - slope2;
  dSlope34 = slope4 - slope3;

  dSlopeSum4  = dSlope12 + dSlope13 + dSlope14 + dSlope23 + dSlope24 + dSlope34;
  dSlopeSum4A = abs(dSlope12) + abs(dSlope13) + abs(dSlope14) + abs(dSlope23) + abs(dSlope24) + abs(dSlope34);

  int devSt1 = abs(dSlope12) + abs(dSlope13) + abs(dSlope14);
  int devSt2 = abs(dSlope12) + abs(dSlope23) + abs(dSlope24);
  int devSt3 = abs(dSlope13) + abs(dSlope23) + abs(dSlope34);
  int devSt4 = abs(dSlope14) + abs(dSlope24) + abs(dSlope34);

  if      (devSt4 > devSt3 && devSt4 > devSt2 && devSt4 > devSt1)  outStSlope = 4;
  else if (devSt3 > devSt4 && devSt3 > devSt2 && devSt3 > devSt1)  outStSlope = 3;
  else if (devSt2 > devSt4 && devSt2 > devSt3 && devSt2 > devSt1)  outStSlope = 2;
  else if (devSt1 > devSt4 && devSt1 > devSt3 && devSt1 > devSt2)  outStSlope = 1;
  else                                                             outStSlope = 0;

  if      (outStSlope == 4) {
    dSlopeSum3  = dSlope12 + dSlope13 + dSlope23;
    dSlopeSum3A = abs(dSlope12) + abs(dSlope13) + abs(dSlope23);
  } else if (outStSlope == 3) {
    dSlopeSum3  = dSlope12 + dSlope14 + dSlope24;
    dSlopeSum3A = abs(dSlope12) + abs(dSlope14) + abs(dSlope24);
  } else if (outStSlope == 2) {
    dSlopeSum3  = dSlope13 + dSlope14 + dSlope34;
    dSlopeSum3A = abs(dSlope13) + abs(dSlope14) + abs(dSlope34);
  } else {
    dSlopeSum3  = dSlope23 + dSlope24 + dSlope34;
    dSlopeSum3A = abs(dSlope23) + abs(dSlope24) + abs(dSlope34);
  }
}

void CalcRPCs( int& RPC1, int& RPC2, int& RPC3, int& RPC4, const int mode,
	       const int st1_ring2, const int theta, const bool BIT_COMP ) {

  if (BIT_COMP) {

    // Mask some invalid locations for RPC hits
    // theta is assumed to be the compressed, mode 15 version
    if (mode == 15 && !st1_ring2) {
      //RPC1 = 0;
      //RPC2 = 0;
      if (theta < 4) {
	RPC3 = 0;
	RPC4 = 0;
      }
    }

    int nRPC = (RPC1 == 1) + (RPC2 == 1) + (RPC3 == 1) + (RPC4 == 1);

    // In 3- and 4-station modes, only specify some combinations of RPCs
    if (nRPC >= 2) {

      if        (mode == 15) {
	if        (RPC1 == 1 && RPC2 == 1) {
	  RPC3 = 0;
	  RPC4 = 0;
	} else if (RPC1 == 1 && RPC3 == 1) {
	  RPC4 = 0;
	} else if (RPC4 == 1 && RPC2 == 1) {
	  RPC3 = 0;
	} else if (RPC3 == 1 && RPC4 == 1 && !st1_ring2) {
	  RPC3 = 0;
	}
      } else if (mode == 14) {
	if        (RPC1 == 1) {
	  RPC2 = 0;
	  RPC3 = 0;
	} else if (RPC3 == 1) {
	  RPC2 = 0;
	}
      } else if (mode == 13) {
	if        (RPC1 == 1) {
	  RPC2 = 0;
	  RPC4 = 0;
	} else if (RPC4 == 1) {
	  RPC2 = 0;
	}
      } else if (mode == 11) {
	if        (RPC1 == 1) {
	  RPC3 = 0;
	  RPC4 = 0;
	} else if (RPC4 == 1) {
	  RPC3 = 0;
	}
      } else if (mode == 7) {
	if        (RPC2 == 1) {
	  RPC3 = 0;
	  RPC4 = 0;
	} else if (RPC4 == 1) {
	  RPC3 = 0;
	}
      }

    } // End conditional: if (nRPC >= 2)
  } // End conditional: if (BIT_COMP)

} // End function: void CalcRPCs()


int CalcBendFromPattern( const int pattern, const int endcap, const bool isRun2 ) {

  int bend = -99;
  if (pattern < 0)
    return bend;

  if(isRun2) {
    if (pattern == 10)
      bend = 0;
    // even numbered -> positive bending (left)
    else if ( (pattern % 2) == 0 )
      bend = (10 - pattern) / 2;
    // odd numbered -> negative bending (right)
    else if ( (pattern % 2) == 1 )
      bend = -1 * (11 - pattern) / 2;
  }

  else {
    if (pattern == 4)
      bend = 0;
    // even numbered -> positive bending (left)
    else if ( (pattern % 2) == 0 )
      bend = (4 - pattern) / 2;
    // odd numbered -> negative bending (right)
    else if ( (pattern % 2) == 1 )
      bend = -1 * (5 - pattern) / 2;
  }

  // Reverse to match dPhi convention
  if (endcap == 1)
    bend *= -1;

  assert( bend != -99 );
  return bend;
}

int CalcBendFromRun3Pattern( const int pattern, const int endcap ) {

  int bend = -99;
  if (pattern < 0)
    return bend;

  if (pattern == 4)
    bend = 0;
  else if ( (pattern % 2) == 0 )
    bend = (4 - pattern) / 2;
  else if ( (pattern % 2) == 1 )
    bend = -1 * (5 - pattern) / 2;

  // Reverse to match dPhi convention
  if (endcap == 1)
    bend *= -1;

  assert( bend != -99 );
  return bend;
}


void CalcDeltaPhiSums( int& dPhSum4, int& dPhSum4A, int& dPhSum3, int& dPhSum3A, int& outStPh,
		       const int dPh12, const int dPh13, const int dPh14, const int dPh23, const int dPh24, const int dPh34 ) {

    dPhSum4  = dPh12 + dPh13 + dPh14 + dPh23 + dPh24 + dPh34;
    dPhSum4A = abs(dPh12) + abs(dPh13) + abs(dPh14) + abs(dPh23) + abs(dPh24) + abs(dPh34);
    int devSt1 = abs(dPh12) + abs(dPh13) + abs(dPh14);
    int devSt2 = abs(dPh12) + abs(dPh23) + abs(dPh24);
    int devSt3 = abs(dPh13) + abs(dPh23) + abs(dPh34);
    int devSt4 = abs(dPh14) + abs(dPh24) + abs(dPh34);

    if      (devSt4 > devSt3 && devSt4 > devSt2 && devSt4 > devSt1)  outStPh = 4;
    else if (devSt3 > devSt4 && devSt3 > devSt2 && devSt3 > devSt1)  outStPh = 3;
    else if (devSt2 > devSt4 && devSt2 > devSt3 && devSt2 > devSt1)  outStPh = 2;
    else if (devSt1 > devSt4 && devSt1 > devSt3 && devSt1 > devSt2)  outStPh = 1;
    else                                                             outStPh = 0;

    if      (outStPh == 4) {
      dPhSum3  = dPh12 + dPh13 + dPh23;
      dPhSum3A = abs(dPh12) + abs(dPh13) + abs(dPh23);
    } else if (outStPh == 3) {
      dPhSum3  = dPh12 + dPh14 + dPh24;
      dPhSum3A = abs(dPh12) + abs(dPh14) + abs(dPh24);
    } else if (outStPh == 2) {
      dPhSum3  = dPh13 + dPh14 + dPh34;
      dPhSum3A = abs(dPh13) + abs(dPh14) + abs(dPh34);
    } else {
      dPhSum3  = dPh23 + dPh24 + dPh34;
      dPhSum3A = abs(dPh23) + abs(dPh24) + abs(dPh34);
    }

} // End function: void CalcDeltaPhiSums()
