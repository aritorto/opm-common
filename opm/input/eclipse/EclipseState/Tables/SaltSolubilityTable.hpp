/*
  Copyright (C) 2020 Equinor ASA.

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
#ifndef OPM_PARSER_SALTSOLUBILITY_TABLE_HPP
#define OPM_PARSER_SALTSOLUBILITY_TABLE_HPP

#include "SimpleTable.hpp"

namespace Opm {

class DeckItem;

class SaltsolTable : public SimpleTable {
public:
    SaltsolTable( const DeckItem& item, const int tableID );

    const TableColumn& getSaltsolColumn() const;
};
}

#endif
