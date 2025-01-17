// -*- mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
// vi: set et ts=4 sw=4 sts=4:
/*
  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.

  Consult the COPYING file in the top-level source directory of this
  module for the precise wording of the license and the list of
  copyright holders.
*/

#include <config.h>
#include <opm/material/fluidsystems/blackoilpvt/BrineH2Pvt.hpp>

#include <opm/common/OpmLog/OpmLog.hpp>

#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Tables/TableManager.hpp>

namespace Opm {

template<class Scalar>
void BrineH2Pvt<Scalar>::
initFromState(const EclipseState& eclState, const Schedule&)
{
    if( !eclState.getTableManager().getDensityTable().empty()) {
        OpmLog::warning("H2STORE is enabled but DENSITY is in the deck. \n"
                        "The surface density is computed based on H2-BRINE PVT "
                        "at standard conditions (STCOND) and DENSITY is ignored ");
    }

    if(eclState.getTableManager().hasTables("PVDO") || 
       !eclState.getTableManager().getPvtgTables().empty()) {
        OpmLog::warning("H2STORE is enabled but PVDO or PVTO is in the deck. \n"
                        "H2 PVT properties are calculated internally, "
                        "and PVDO/PVTO input is ignored.");
    }
    // Check if DISGAS has been activated (enables H2 dissolved in brine)
    setEnableDissolvedGas(eclState.getSimulationConfig().hasDISGASW() || eclState.getSimulationConfig().hasDISGAS());

    // Check if BRINE has been activated (varying salt concentration in brine)
    setEnableSaltConcentration(eclState.runspec().phases().active(Phase::BRINE));

    // We only supported single pvt region for the H2-brine module
    size_t numRegions = 1;
    setNumRegions(numRegions);
    size_t regionIdx = 0;

    // Currently we only support constant salinity
    const Scalar molality = eclState.getTableManager().salinity(); // mol/kg
    const Scalar MmNaCl = 58.44e-3; // molar mass of NaCl [kg/mol]
    salinity_[regionIdx] = 1 / ( 1 + 1 / (molality*MmNaCl)); // convert to mass fraction

    // set the surface conditions using the STCOND keyword
    Scalar T_ref = eclState.getTableManager().stCond().temperature;
    Scalar P_ref = eclState.getTableManager().stCond().pressure;

    brineReferenceDensity_[regionIdx] = Brine::liquidDensity(T_ref, P_ref, salinity_[regionIdx], extrapolate);
    h2ReferenceDensity_[regionIdx] = H2::gasDensity(T_ref, P_ref, extrapolate);
}

template class BrineH2Pvt<double>;
template class BrineH2Pvt<float>;

} // namespace Opm