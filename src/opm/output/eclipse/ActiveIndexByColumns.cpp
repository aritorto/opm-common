/*
  Copyright (c) 2021 Equinor ASA

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

#include <opm/output/eclipse/ActiveIndexByColumns.hpp>

#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <functional>
#include <numeric>
#include <utility>
#include <vector>

namespace {
    std::size_t columnarGlobalIdx(const std::array<int, 3>&           dims,
                                  const std::array<int, 3>&           ijk,
                                  const std::array<int, 3>::size_type outer,
                                  const std::array<int, 3>::size_type middle)
    {
        // Linear index assuming C-like loop order
        //
        //     for (i = 0 .. Nx - 1)
        //         for (j = 0 .. Ny - 1)
        //             for (k = 0 .. Nz - 1)
        //
        // or, if Ny > Nx, the alternate loop order
        //
        //     for (j = 0 .. Ny - 1)
        //         for (i = 0 .. Nx - 1)
        //             for (k = 0 .. Nz - 1)
        //
        // swapping the 'i' and 'j' loops.  This is opposed to the usual,
        // Fortran-like, loop order ("natural ordering")
        //
        //     for (k = 0 .. Nz - 1)
        //         for (j = 0 .. Ny - 1)
        //             for (i = 0 .. Nx - 1)
        //
        // that is used elsewhere.  In other words, the inner loop is always
        // across the model layers while we ensure that the 'outer' loop
        // always iterates over an index range that is at least as large as
        // that of the 'middle' loop.

        return ijk[2] + dims[2]*(ijk[middle] + dims[middle]*ijk[outer]);
    }

    std::pair<std::array<int, 3>::size_type, std::array<int, 3>::size_type>
    inferOuterLoopOrdering(const std::array<int, 3>& cartDims)
    {
        auto outer  = std::array<int, 3>::size_type{0};
        auto middle = std::array<int, 3>::size_type{1};

        if (cartDims[middle] > cartDims[outer]) {
            std::swap(outer, middle);
        }

        return { outer, middle };
    }

    std::vector<std::size_t>
    computeColumnarGlobalIndex(const std::vector<std::size_t>&                             activeCells,
                               const std::array<int, 3>&                                   cartDims,
                               const std::function<std::array<int, 3>(const std::size_t)>& getIJK)
    {
        auto colGlobIx = activeCells;

        const auto [outer, middle] = inferOuterLoopOrdering(cartDims);

        std::transform(colGlobIx.begin(), colGlobIx.end(), colGlobIx.begin(),
                       [&cartDims, &getIJK, outer, middle]
            (const std::size_t cell)
        {
            return columnarGlobalIdx(cartDims, getIJK(cell), outer, middle);
        });

        return colGlobIx;
    }

    std::vector<int>
    buildMappingTables(const std::size_t                                           numActive,
                       const std::array<int, 3>&                                   cartDims,
                       const std::function<std::array<int, 3>(const std::size_t)>& getIJK)
    {
        auto natural2columnar = std::vector<int>(numActive, 0);

        auto activeCells = std::vector<std::size_t>(numActive, std::size_t{0});
        std::iota(activeCells.begin(), activeCells.end(), std::size_t{0});

        const auto colGlobIx = computeColumnarGlobalIndex(activeCells, cartDims, getIJK);

        std::sort(activeCells.begin(), activeCells.end(),
            [&colGlobIx](const std::size_t cell1, const std::size_t cell2)
        {
            return colGlobIx[cell1] < colGlobIx[cell2];
        });

        auto columnarActiveID = 0;
        for (const auto& naturalActiveID : activeCells) {
            natural2columnar[naturalActiveID] = columnarActiveID++;
        }

        return natural2columnar;
    }
}

bool Opm::ActiveIndexByColumns::operator==(const ActiveIndexByColumns& rhs) const
{
    return this->natural2columnar_ == rhs.natural2columnar_;
}

Opm::ActiveIndexByColumns::
ActiveIndexByColumns(const std::size_t                                           numActive,
                     const std::array<int, 3>&                                   cartDims,
                     const std::function<std::array<int, 3>(const std::size_t)>& getIJK)
    : natural2columnar_{ buildMappingTables(numActive, cartDims, getIJK) }
{}

Opm::ActiveIndexByColumns
Opm::buildColumnarActiveIndexMappingTables(const EclipseGrid& grid)
{
    return ActiveIndexByColumns { grid.getNumActive(), grid.getNXYZ(),
        [&grid](const std::size_t activeCell)
    {
        return grid.getIJK(grid.getGlobalIndex(activeCell));
    }};
}
