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
/*!
 * \file
 *
 * \brief This is the unit test for the class which manages the parameters for the ECL
 *        saturation functions.
 *
 * This test requires the presence of opm-parser.
 */
#include "config.h"

#if !HAVE_ECL_INPUT
#error "The test for EclMaterialLawManager requires eclipse input support in opm-common"
#endif

#include <opm/material/fluidmatrixinteractions/EclEpsGridProperties.hpp>
#include <opm/material/fluidmatrixinteractions/EclMaterialLawManager.hpp>
#include <opm/material/fluidstates/SimpleModularFluidState.hpp>

#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>

// values of strings taken from the SPE1 test case1 of opm-data
static const char* fam1DeckString =
    "RUNSPEC\n"
    "\n"
    "DIMENS\n"
    "   10 10 3 /\n"
    "\n"
    "TABDIMS\n"
    "/\n"
    "\n"
    "OIL\n"
    "GAS\n"
    "WATER\n"
    "\n"
    "DISGAS\n"
    "\n"
    "FIELD\n"
    "\n"
    "GRID\n"
    "\n"
    "DX\n"
    "       300*1000 /\n"
    "DY\n"
    "   300*1000 /\n"
    "DZ\n"
    "   100*20 100*30 100*50 /\n"
    "\n"
    "TOPS\n"
    "   100*8325 /\n"
    "\n"
    "\n"
    "PORO\n"
    "  300*0.15 /\n"
    "PROPS\n"
    "\n"
    "SWOF\n"
    "0.12   0               1   0\n"
    "0.18   4.64876033057851E-008   1   0\n"
    "0.24   0.000000186     0.997   0\n"
    "0.3    4.18388429752066E-007   0.98    0\n"
    "0.36   7.43801652892562E-007   0.7 0\n"
    "0.42   1.16219008264463E-006   0.35    0\n"
    "0.48   1.67355371900826E-006   0.2 0\n"
    "0.54   2.27789256198347E-006   0.09    0\n"
    "0.6    2.97520661157025E-006   0.021   0\n"
    "0.66   3.7654958677686E-006    0.01    0\n"
    "0.72   4.64876033057851E-006   0.001   0\n"
    "0.78   0.000005625     0.0001  0\n"
    "0.84   6.69421487603306E-006   0   0\n"
    "0.91   8.05914256198347E-006   0   0\n"
    "1      0.984           0   0 /\n"
    "\n"
    "\n"
    "SGOF\n"
    "0  0   1   0\n"
    "0.001  0   1   0\n"
    "0.02   0   0.997   0\n"
    "0.05   0.005   0.980   0\n"
    "0.12   0.025   0.700   0\n"
    "0.2    0.075   0.350   0\n"
    "0.25   0.125   0.200   0\n"
    "0.3    0.190   0.090   0\n"
    "0.4    0.410   0.021   0\n"
    "0.45   0.60    0.010   0\n"
    "0.5    0.72    0.001   0\n"
    "0.6    0.87    0.0001  0\n"
    "0.7    0.94    0.000   0\n"
    "0.85   0.98    0.000   0\n"
    "0.88   0.984   0.000   0 /\n";

static const char* fam2DeckString =
    "RUNSPEC\n"
    "\n"
    "DIMENS\n"
    "   10 10 3 /\n"
    "\n"
    "TABDIMS\n"
    "/\n"
    "\n"
    "OIL\n"
    "GAS\n"
    "WATER\n"
    "\n"
    "DISGAS\n"
    "\n"
    "FIELD\n"
    "\n"
    "GRID\n"
    "\n"
    "DX\n"
    "       300*1000 /\n"
    "DY\n"
    "   300*1000 /\n"
    "DZ\n"
    "   100*20 100*30 100*50 /\n"
    "\n"
    "TOPS\n"
    "   100*8325 /\n"
    "\n"
    "\n"
    "PORO\n"
    "  300*0.15 /\n"
    "PROPS\n"
    "\n"
    "PVTW\n"
    "       4017.55 1.038 3.22E-6 0.318 0.0 /\n"
    "\n"
    "\n"
    "SWFN\n"
    "0.12   0               0\n"
    "0.18   4.64876033057851E-008   0\n"
    "0.24   0.000000186     0\n"
    "0.3    4.18388429752066E-007   0\n"
    "0.36   7.43801652892562E-007   0\n"
    "0.42   1.16219008264463E-006   0\n"
    "0.48   1.67355371900826E-006   0\n"
    "0.54   2.27789256198347E-006   0\n"
    "0.6    2.97520661157025E-006   0\n"
    "0.66   3.7654958677686E-006    0\n"
    "0.72   4.64876033057851E-006   0\n"
    "0.78   0.000005625     0\n"
    "0.84   6.69421487603306E-006   0\n"
    "0.91   8.05914256198347E-006   0\n"
    "1  0.984           0 /\n"
    "\n"
    "\n"
    "SGFN\n"
    "0  0   0\n"
    "0.001  0   0\n"
    "0.02   0   0\n"
    "0.05   0.005   0\n"
    "0.12   0.025   0\n"
    "0.2    0.075   0\n"
    "0.25   0.125   0\n"
    "0.3    0.190   0\n"
    "0.4    0.410   0\n"
    "0.45   0.60    0\n"
    "0.5    0.72    0\n"
    "0.6    0.87    0\n"
    "0.7    0.94    0\n"
    "0.85   0.98    0\n"
    "0.88   0.984   0 /\n"
    "\n"
    "SOF3\n"
    "    0        0        0 \n"
    "    0.03     0        0 \n"
    "    0.09     0        0 \n"
    "    0.16     0       0 \n"
    "    0.18     1*       0 \n"
    "    0.22     0.0001   1* \n"
    "    0.28     0.001    0.0001 \n"
    "    0.34     0.01     1* \n"
    "    0.38     1*       0.001 \n"
    "    0.40     0.021    1* \n"
    "    0.43     1*       0.01 \n"
    "    0.46     0.09     1* \n"
    "    0.48     1*       0.021 \n"
    "    0.52     0.2      1* \n"
    "    0.58     0.35     0.09 \n"
    "    0.63     1*       0.2 \n"
    "    0.64     0.7      1* \n"
    "    0.68     1*       0.35 \n"
    "    0.70     0.98     1* \n"
    "    0.76     0.997    0.7 \n"
    "    0.83     1        0.98 \n"
    "    0.86     1        0.997  \n"
    "    0.879    1        1 \n"
    "    0.88     1        1    /  \n"
    "\n";

//Taken as a mix of the SPE1 cases above, and Norne to enable hysteresis
static const char* hysterDeckString =
    "RUNSPEC\n"
    "\n"
    "DIMENS\n"
    "   10 10 3 /\n"
    "\n"
    "TABDIMS\n"
    "/\n"
    "\n"
    "OIL\n"
    "GAS\n"
    "WATER\n"
    "\n"
    "DISGAS\n"
    "\n"
    "FIELD\n"
    "\n"
    "GRID\n"
    "\n"
    "DX\n"
    "       300*1000 /\n"
    "DY\n"
    "   300*1000 /\n"
    "DZ\n"
    "   100*20 100*30 100*50 /\n"
    "\n"
    "TOPS\n"
    "   100*8325 /\n"
    "PORO\n"
    "  300*0.15 /\n"
    "\n"
    "\n"
    "EHYSTR\n"
    "0.1   0  0.1 1* BOTH /\n"
    "\n"
    "SATOPTS\n"
    "HYSTER /\n"
    "\n"
    "PROPS\n"
    "\n"
    "SWOF\n"
    "0.12   0               1   0\n"
    "0.18   4.64876033057851E-008   1   0\n"
    "0.24   0.000000186     0.997   0\n"
    "0.3    4.18388429752066E-007   0.98    0\n"
    "0.36   7.43801652892562E-007   0.7 0\n"
    "0.42   1.16219008264463E-006   0.35    0\n"
    "0.48   1.67355371900826E-006   0.2 0\n"
    "0.54   2.27789256198347E-006   0.09    0\n"
    "0.6    2.97520661157025E-006   0.021   0\n"
    "0.66   3.7654958677686E-006    0.01    0\n"
    "0.72   4.64876033057851E-006   0.001   0\n"
    "0.78   0.000005625     0.0001  0\n"
    "0.84   6.69421487603306E-006   0   0\n"
    "0.91   8.05914256198347E-006   0   0\n"
    "1      0.984           0   0 /\n"
    "\n"
    "\n"
    "SGOF\n"
    "0  0   1   0\n"
    "0.001  0   1   0\n"
    "0.02   0   0.997   0\n"
    "0.05   0.005   0.980   0\n"
    "0.12   0.025   0.700   0\n"
    "0.2    0.075   0.350   0\n"
    "0.25   0.125   0.200   0\n"
    "0.3    0.190   0.090   0\n"
    "0.4    0.410   0.021   0\n"
    "0.45   0.60    0.010   0\n"
    "0.5    0.72    0.001   0\n"
    "0.6    0.87    0.0001  0\n"
    "0.7    0.94    0.000   0\n"
    "0.85   0.98    0.000   0\n"
    "0.88   0.984   0.000   0 /\n";

static const char* fam1DeckStringGasOil =
    "RUNSPEC\n"
    "\n"
    "DIMENS\n"
    "   10 10 3 /\n"
    "\n"
    "TABDIMS\n"
    "/\n"
    "\n"
    "OIL\n"
    "GAS\n"
    "\n"
    "DISGAS\n"
    "\n"
    "FIELD\n"
    "\n"
    "GRID\n"
    "\n"
    "DX\n"
    "       300*1000 /\n"
    "DY\n"
    "   300*1000 /\n"
    "DZ\n"
    "   100*20 100*30 100*50 /\n"
    "\n"
    "TOPS\n"
    "   100*8325 /\n"
    "\n"
    "\n"
    "PORO\n"
    "  300*0.15 /\n"
    "PROPS\n"
    "\n"
    "\n"
    "SGOF\n"
    "0  0   1   0\n"
    "0.001  0   1   0\n"
    "0.02   0   0.997   0\n"
    "0.05   0.005   0.980   0\n"
    "0.12   0.025   0.700   0\n"
    "0.2    0.075   0.350   0\n"
    "0.25   0.125   0.200   0\n"
    "0.3    0.190   0.090   0\n"
    "0.4    0.410   0.021   0\n"
    "0.45   0.60    0.010   0\n"
    "0.5    0.72    0.001   0\n"
    "0.6    0.87    0.0001  0\n"
    "0.7    0.94    0.000   0\n"
    "0.85   0.98    0.000   0\n"
    "0.88   0.984   0.000   0 /\n";

static const char* fam2DeckStringGasOil =
    "RUNSPEC\n"
    "\n"
    "DIMENS\n"
    "   10 10 3 /\n"
    "\n"
    "TABDIMS\n"
    "/\n"
    "\n"
    "OIL\n"
    "GAS\n"
    "\n"
    "DISGAS\n"
    "\n"
    "FIELD\n"
    "\n"
    "GRID\n"
    "\n"
    "DX\n"
    "       300*1000 /\n"
    "DY\n"
    "   300*1000 /\n"
    "DZ\n"
    "   100*20 100*30 100*50 /\n"
    "\n"
    "TOPS\n"
    "   100*8325 /\n"
    "PORO\n"
    "  300*0.15 /\n"
    "\n"
    "\n"
    "PROPS\n"
    "\n"
    "PVTW\n"
    "       4017.55 1.038 3.22E-6 0.318 0.0 /\n"
    "\n"
    "\n"
    "SGFN\n"
    "0      0   0\n"
    "0.001  0   0\n"
    "0.02   0   0\n"
    "0.05   0.005   0\n"
    "0.12   0.025   0\n"
    "0.2    0.075   0\n"
    "0.25   0.125   0\n"
    "0.3    0.190   0\n"
    "0.4    0.410   0\n"
    "0.45   0.60    0\n"
    "0.5    0.72    0\n"
    "0.6    0.87    0\n"
    "0.7    0.94    0\n"
    "0.85   0.98    0\n"
    "0.88   0.984   0 /\n"
    "\n"
    "SOF2\n"
    "0.12   0.000   \n"
    "0.15   0.000   \n"
    "0.3    0.000   \n"
    "0.4    0.0001  \n"
    "0.5    0.001   \n"
    "0.55   0.010   \n"
    "0.6    0.021   \n"
    "0.7    0.090   \n"
    "0.8    0.350   \n"
    "0.88   0.700   \n"
    "0.95   0.980   \n"
    "0.98   0.997   \n"
    "0.999  1       \n"
    "1.0    1       \n /\n";


static const char* letDeckString =
    "RUNSPEC\n"
    "\n"
    "DIMENS\n"
    "   10 10 3 /\n"
    "\n"
    "TABDIMS\n"
    "/\n"
    "\n"
    "OIL\n"
    "GAS\n"
    "WATER\n"
    "\n"
    "DISGAS\n"
    "\n"
    "FIELD\n"
    "\n"
    "GRID\n"
    "\n"
    "DX\n"
    "       300*1000 /\n"
    "DY\n"
    "   300*1000 /\n"
    "DZ\n"
    "   100*20 100*30 100*50 /\n"
    "\n"
    "TOPS\n"
    "   100*8325 /\n"
    "\n"
    "\n"
    "PORO\n"
    "  300*0.15 /\n"
    "PROPS\n"
    "\n"
    "SWOFLET\n"
    " 0.1  0.2 1.5 7.0 1.5 0.5    0.05 0.15 3.5 3.0 1.3 1.0    0.7 17.0 0.95 3.8 0.04 /\n"
    "\n"
    "SGOFLET\n"
    " 0.0  0.03 1.8 1.9 1.0 0.95   0.0  0.01 3.5 4.0 1.1 1.0   1.0 1.0 1.0 0.2 0.01 /\n"
    "\n";


template <class Scalar>
inline Scalar computeLetCurve(const Scalar S, const Scalar L, const Scalar E, const Scalar T)
{
        if (S <= 0.0) {
            return 0.0;
        } else if (S >= 1.0) {
            return 1.0;
        }

        const Scalar powS = Opm::pow(S,L);
        const Scalar pow1mS = Opm::pow(1.0-S,T);

        return powS/(powS+pow1mS*E);
}


template <class Scalar>
inline void testAll()
{
    enum { numPhases = 3 };
    enum { waterPhaseIdx = 0 };
    enum { oilPhaseIdx = 1 };
    enum { gasPhaseIdx = 2 };
    typedef Opm::ThreePhaseMaterialTraits<Scalar,
                                          /*wettingPhaseIdx=*/waterPhaseIdx,
                                          /*nonWettingPhaseIdx=*/oilPhaseIdx,
                                          /*gasPhaseIdx=*/gasPhaseIdx> MaterialTraits;

    typedef Opm::SimpleModularFluidState<Scalar,
                                         /*numPhases=*/3,
                                         /*numComponents=*/3,
                                         void,
                                         /*storePressure=*/false,
                                         /*storeTemperature=*/false,
                                         /*storeComposition=*/false,
                                         /*storeFugacity=*/false,
                                         /*storeSaturation=*/true,
                                         /*storeDensity=*/false,
                                         /*storeViscosity=*/false,
                                         /*storeEnthalpy=*/false> FluidState;

    Opm::Parser parser;

    {
        typedef Opm::EclMaterialLawManager<MaterialTraits> MaterialLawManager;
        typedef typename MaterialLawManager::MaterialLaw MaterialLaw;

        const auto deck = parser.parseString(fam1DeckString);
        const Opm::EclipseState eclState(deck);
        const auto& eclGrid = eclState.getInputGrid();

        size_t n = eclGrid.getCartesianSize();

        MaterialLawManager materialLawManager;
        materialLawManager.initFromState(eclState);
        materialLawManager.initParamsForElements(eclState, n);

        if (materialLawManager.enableEndPointScaling())
            throw std::logic_error("Discrepancy between the deck and the EclMaterialLawManager");

        if (materialLawManager.enableHysteresis())
            throw std::logic_error("Discrepancy between the deck and the EclMaterialLawManager");

        {
            const auto fam2Deck = parser.parseString(fam2DeckString);
            const Opm::EclipseState fam2EclState(fam2Deck);

            Opm::EclMaterialLawManager<MaterialTraits> fam2MaterialLawManager;
            fam2MaterialLawManager.initFromState(fam2EclState);
            fam2MaterialLawManager.initParamsForElements(fam2EclState, n);

            if (fam2MaterialLawManager.enableEndPointScaling())
                throw std::logic_error("Discrepancy between the deck and the EclMaterialLawManager");

            if (fam2MaterialLawManager.enableHysteresis())
                throw std::logic_error("Discrepancy between the deck and the EclMaterialLawManager");

            const auto hysterDeck = parser.parseString(hysterDeckString);
            const Opm::EclipseState hysterEclState(hysterDeck);

            Opm::EclMaterialLawManager<MaterialTraits> hysterMaterialLawManager;
            hysterMaterialLawManager.initFromState(hysterEclState);
            hysterMaterialLawManager.initParamsForElements(hysterEclState, n);

            if (hysterMaterialLawManager.enableEndPointScaling())
                throw std::logic_error("Discrepancy between the deck and the EclMaterialLawManager");

            if (hysterMaterialLawManager.enableHysteresis() != true)
                throw std::logic_error("Discrepancy between the deck and the EclMaterialLawManager");



            // make sure that the saturation functions for both keyword families are
            // identical, and that setting and getting the hysteresis parameters works
            for (unsigned elemIdx = 0; elemIdx < n; ++ elemIdx) {
                for (int i = -10; i < 120; ++ i) {
                    Scalar Sw = Scalar(i)/100;
                    for (int j = i; j < 120; ++ j) {
                        Scalar So = Scalar(j)/100;
                        Scalar Sg = 1 - Sw - So;
                        FluidState fs;
                        fs.setSaturation(waterPhaseIdx, Sw);
                        fs.setSaturation(oilPhaseIdx, So);
                        fs.setSaturation(gasPhaseIdx, Sg);

                        Scalar pcFam1[numPhases]  = { 0.0, 0.0 };
                        Scalar pcFam2[numPhases]  = { 0.0, 0.0 };
                        MaterialLaw::capillaryPressures(pcFam1,
                                                        materialLawManager.materialLawParams(elemIdx),
                                                        fs);
                        MaterialLaw::capillaryPressures(pcFam2,
                                                        fam2MaterialLawManager.materialLawParams(elemIdx),
                                                        fs);

                        Scalar krFam1[numPhases] = { 0.0, 0.0 };
                        Scalar krFam2[numPhases] = { 0.0, 0.0 };
                        MaterialLaw::relativePermeabilities(krFam1,
                                                            materialLawManager.materialLawParams(elemIdx),
                                                            fs);
                        MaterialLaw::relativePermeabilities(krFam2,
                                                            fam2MaterialLawManager.materialLawParams(elemIdx),
                                                            fs);

                        for (unsigned phaseIdx = 0; phaseIdx < numPhases; ++ phaseIdx) {

                            if (std::abs(pcFam1[phaseIdx] - pcFam2[phaseIdx]) > 1e-5)
                                throw std::logic_error("Discrepancy between capillary pressure of family 1 and family 2 keywords");
                            if (std::abs(krFam1[phaseIdx] - krFam2[phaseIdx]) > 1e-1)
                                throw std::logic_error("Discrepancy between relative permeabilities of family 1 and family 2 keywords");
                        }


                        // This should ideally test each of the materials (stone1, stone2, default, two-phase),
                        // but currently only tests default
                        const Scalar pcSwMdc_in[2] = { 1.0/2.0, 1.0/3.0 };
                        const Scalar krnSwMdc_in[2] = { 1.0/5.0, 1.0/7.0 };
                        hysterMaterialLawManager.setOilWaterHysteresisParams(
                                                                             pcSwMdc_in[0],
                                                                             krnSwMdc_in[0],
                                                                             elemIdx);
                        hysterMaterialLawManager.setGasOilHysteresisParams(
                                                                           pcSwMdc_in[1],
                                                                           krnSwMdc_in[1],
                                                                           elemIdx);

                        Scalar pcSwMdc_out[2] = { 0.0, 0.0 };
                        Scalar krnSwMdc_out[2] = { 0.0, 0.0 };
                        hysterMaterialLawManager.oilWaterHysteresisParams(
                                                                          pcSwMdc_out[0],
                                                                          krnSwMdc_out[0],
                                                                          elemIdx);
                        hysterMaterialLawManager.gasOilHysteresisParams(
                                                                        pcSwMdc_out[1],
                                                                        krnSwMdc_out[1],
                                                                        elemIdx);

                        for (unsigned phasePairIdx = 0; phasePairIdx < 2; ++ phasePairIdx) {
                            if ((pcSwMdc_in[phasePairIdx] - pcSwMdc_out[phasePairIdx]) != 0.0)
                                throw std::logic_error("Hysteresis parameters did not propagate correctly");
                            if ((krnSwMdc_in[phasePairIdx] - krnSwMdc_out[phasePairIdx]) != 0.0)
                                throw std::logic_error("Hysteresis parameters did not propagate correctly");

                        }
                    }
                }
            }
        }

        // Gas oil
        {
            const auto fam1Deck = parser.parseString(fam1DeckStringGasOil);
            const Opm::EclipseState fam1EclState(fam1Deck);

            MaterialLawManager fam1materialLawManager;
            fam1materialLawManager.initFromState(fam1EclState);
            fam1materialLawManager.initParamsForElements(fam1EclState, n);

            const auto fam2Deck = parser.parseString(fam2DeckStringGasOil);
            const Opm::EclipseState fam2EclState(fam2Deck);

            Opm::EclMaterialLawManager<MaterialTraits> fam2MaterialLawManager;
            fam2MaterialLawManager.initFromState(fam2EclState);
            fam2MaterialLawManager.initParamsForElements(fam2EclState, n);

            for (unsigned elemIdx = 0; elemIdx < n; ++ elemIdx) {
                for (int i = 0; i < 100; ++ i) {
                    Scalar Sw = 0;
                    Scalar So = Scalar(i)/100;
                    Scalar Sg = 1 - Sw - So;
                    FluidState fs;
                    fs.setSaturation(waterPhaseIdx, Sw);
                    fs.setSaturation(oilPhaseIdx, So);
                    fs.setSaturation(gasPhaseIdx, Sg);

                    Scalar pcFam1[numPhases]  = { 0.0, 0.0 };
                    Scalar pcFam2[numPhases]  = { 0.0, 0.0 };
                    MaterialLaw::capillaryPressures(pcFam1,
                                                    fam1materialLawManager.materialLawParams(elemIdx),
                                                    fs);
                    MaterialLaw::capillaryPressures(pcFam2,
                                                    fam2MaterialLawManager.materialLawParams(elemIdx),
                                                    fs);

                    Scalar krFam1[numPhases] = { 0.0, 0.0 };
                    Scalar krFam2[numPhases] = { 0.0, 0.0 };
                    MaterialLaw::relativePermeabilities(krFam1,
                                                        fam1materialLawManager.materialLawParams(elemIdx),
                                                        fs);
                    MaterialLaw::relativePermeabilities(krFam2,
                                                        fam2MaterialLawManager.materialLawParams(elemIdx),
                                                        fs);

                    for (unsigned phaseIdx = 0; phaseIdx < numPhases; ++ phaseIdx) {

                        if (std::abs(pcFam1[phaseIdx] - pcFam2[phaseIdx]) > 1e-5)
                            throw std::logic_error("Discrepancy between capillary pressure of family 1 and family 2 keywords");
                        if (std::abs(krFam1[phaseIdx] - krFam2[phaseIdx]) > 1e-1)
                            throw std::logic_error("Discrepancy between relative permeabilities of family 1 and family 2 keywords");
                    }
                }
            }
        }

        // LET: Curves from input parameters interpreted via MaterialLawManager versus directly computed curves.
        {
            const auto letDeck = parser.parseString(letDeckString);
            const Opm::EclipseState letEclState(letDeck);

            MaterialLawManager letmaterialLawManager;
            letmaterialLawManager.initFromState(letEclState);
            letmaterialLawManager.initParamsForElements(letEclState, n);

            Scalar Swco = 0.1;
            Scalar psi2Pa = 6894.7573;

            for (unsigned elemIdx = 0; elemIdx < 1; ++ elemIdx) {

                for (int i = -10; i < 120; ++ i) {
                    Scalar So = Scalar(i)/100;

                    // Oil in gas and conate water

                    Scalar Sw=Swco;
                    Scalar Sg = 1 - Sw - So;
                    FluidState fs;
                    fs.setSaturation(waterPhaseIdx, Sw);
                    fs.setSaturation(oilPhaseIdx, So);
                    fs.setSaturation(gasPhaseIdx, Sg);

                    Scalar pcLET[numPhases]  = { 0.0, 0.0, 0.0 };
                    MaterialLaw::capillaryPressures(pcLET,
                                                    letmaterialLawManager.materialLawParams(elemIdx),
                                                    fs);

                    Scalar krLET[numPhases] = { 0.0, 0.0, 0.0 };
                    MaterialLaw::relativePermeabilities(krLET,
                                                        letmaterialLawManager.materialLawParams(elemIdx),
                                                        fs);

                    //SGOFLET
                    //0.0  0.03 1.8 1.9 1.0 0.95   0.0  0.01 3.5 4.0 1.1 1.0   1.0 1.0 1.0 0.2 0.01
                    Scalar krg_let = 0.95*computeLetCurve((1.0-So-0.03-Swco)/(1.0-0.03-0.01-Swco), 1.8, 1.9, 1.0);
                    Scalar krog_let = 1.0*computeLetCurve((So-0.01)/(1.0-0.03-0.01-Swco), 3.5, 4.0, 1.1);
                    //S=(So-Sorg)/(1-Sorg-Sgl-Swco), Pc = Pct + (pcir_pc-Pct)*(1-S)^L/[(1-S)^L+E*S^T]
                    Scalar pcog_let = psi2Pa*(0.01 + (0.2-0.01)*computeLetCurve((1.0-So-Swco)/(1.0-Swco), 1.0, 1.0, 1.0));

                    if (std::abs(krLET[gasPhaseIdx]-krg_let) > 1e-5 || std::abs(krLET[oilPhaseIdx]-krog_let) > 1e-5)
                        throw std::logic_error("Inconsistent LET relative permeabilities family 1 gas/oil");
                    if (std::abs(pcLET[gasPhaseIdx]-pcog_let) > 1e-2)
                        throw std::logic_error("Inconsistent LET capillary pressure family 1 gas/oil");

                    // Oil in water

                    Sg = 0.0;
                    Sw = 1.0-So-Sg;
                    fs.setSaturation(waterPhaseIdx, Sw);
                    fs.setSaturation(oilPhaseIdx, So);
                    fs.setSaturation(gasPhaseIdx, Sg);

                    MaterialLaw::capillaryPressures(pcLET,
                                                    letmaterialLawManager.materialLawParams(elemIdx),
                                                    fs);

                    MaterialLaw::relativePermeabilities(krLET,
                                                        letmaterialLawManager.materialLawParams(elemIdx),
                                                        fs);

                    //SWOFLET
                    //0.1  0.2 1.5 7.0 1.5 0.5    0.05 0.15 3.5 3.0 1.3 1.0    0.7 17.0 0.95 3.8 0.04
                    Scalar krw_let = 0.5*computeLetCurve((Sw-0.2)/(1.0-0.2-0.15), 1.5, 7.0, 1.5);
                    Scalar krow_let = 1.0*computeLetCurve((1.0-Sw-0.15)/(1.0-0.2-0.15), 3.5, 3.0, 1.3);
                    // S=(Sw-Swco)/(1-Swco-Sorw), Pc = Pct + (Pcir-Pct)*(1-S)^L/[(1-S)^L+E*S^T]
                    Scalar pcow_let = -psi2Pa*(0.04 + (3.8-0.04)*computeLetCurve(1.0-(Sw-Swco)/(1.0-0.05-Swco), 0.7, 17.0, 0.95));

                    if (std::abs(krLET[waterPhaseIdx]-krw_let) > 1e-5 || std::abs(krLET[oilPhaseIdx]-krow_let) > 1e-5)
                        throw std::logic_error("Inconsistent LET relative permeabilities family 1 wat/oil");
                    if (std::abs(pcLET[waterPhaseIdx]-pcow_let) > 1e-2)
                        throw std::logic_error("Inconsistent LET capillary pressure family 1 wat/oil");

                }
            }
        }
    }
}

int main()
{
    testAll<double>();
    testAll<float>();

    return 0;
}
