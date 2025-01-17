/*
 Copyright (C) 2023 Equinor
  This file is part of the Open Porous Media project (OPM).
  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
 */

#define BOOST_TEST_MODULE LgrTests

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Grid/LgrCollection.hpp>
#include <opm/input/eclipse/EclipseState/Grid/Carfin.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>

#include <filesystem>

using namespace Opm;

BOOST_AUTO_TEST_CASE(CreateLgrCollection) {
    Opm::LgrCollection lgrs;
    BOOST_CHECK_EQUAL( lgrs.size() , 0U );
    BOOST_CHECK(! lgrs.hasLgr("NO-NotThisOne"));
    BOOST_CHECK_THROW( lgrs.getLgr("NO") , std::invalid_argument );
}

BOOST_AUTO_TEST_CASE(ReadLgrCollection) { 
    const std::string deck_string = R"(
RUNSPEC

DIMENS
 10 10 10 /

GRID

CARFIN
-- NAME I1-I2 J1-J2 K1-K2 NX NY NZ
'LGR1'  5  6  5  6  1  3  6  6  9 /
ENDFIN

CARFIN
-- NAME I1-I2 J1-J2 K1-K2 NX NY NZ
'LGR2'  7  8  7  8  1  3  8  8  9 /
ENDFIN


DX
1000*1 /
DY
1000*1 /
DZ
1000*1 /
TOPS
100*1 /

PORO
  1000*0.15 /

PERMX
  1000*1 /

COPY
  PERMX PERMZ /
  PERMX PERMY /
/

EDIT

OIL
GAS

TITLE
The title

START
16 JUN 1988 /

PROPS

REGIONS

SOLUTION

SCHEDULE
)";

    Opm::Parser parser;
    Opm::Deck deck = parser.parseString(deck_string);
    Opm::EclipseState state(deck);
    Opm::LgrCollection lgrs = state.getLgrs();

    BOOST_CHECK_EQUAL( lgrs.size() , 2U );
    BOOST_CHECK(lgrs.hasLgr("LGR1"));
    BOOST_CHECK(lgrs.hasLgr("LGR2"));

    const auto& lgr1 = state.getLgrs().getLgr("LGR1");
    BOOST_CHECK_EQUAL(lgr1.NAME(), "LGR1");
    const auto& lgr2 = lgrs.getLgr("LGR2");
    BOOST_CHECK_EQUAL( lgr2.NAME() , "LGR2");

    const auto& lgr3 = state.getLgrs().getLgr(0);
    BOOST_CHECK_EQUAL( lgr1.NAME() , lgr3.NAME());
}
