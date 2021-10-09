// Copyright © 2021 Sam Varner
//
// This file is part of 4color.
//
// 4color is free software: you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// Composure is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with Composure.
// If not, see <http://www.gnu.org/licenses/>.

#ifndef FOUR_COLOR_LIB4COLOR_FIGURE_HH_INCLUDED
#define FOUR_COLOR_LIB4COLOR_FIGURE_HH_INCLUDED

#include "point.hh"

#include <iostream>
#include <set>

using Tile_List = std::set<Point<int>>;

/// A polyomino with tiles at integer coordinates.
class Figure
{
public:
    Figure();
    Figure(std::initializer_list<Point<int>> ps);

    /// @return True if each tile shares an edge with another.
    bool is_contiguous() const;
    /// @return The set of tile positions.
    Tile_List const& tiles() const;
    /// @return The center of mass of the figure.
    Point<double> cm() const;

    /// If p is a point in the figure, remove it. Otherwise, add it.
    void toggle(Point<int> const& p);
    /// Remove all points.
    void clear();

private:
    Tile_List m_tiles;
    Point<double> m_cm{0.0, 0.0};
};

#endif // FOUR_COLOR_LIB4COLOR_FIGURE_HH_INCLUDED
