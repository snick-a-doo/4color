// Copyright © 2021 Sam Varner
//
// This file is part of 4color.
//
// 4color is free software: you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// 4color is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with 4color.
// If not, see <http://www.gnu.org/licenses/>.

#include "figure.hh"

/// A recursive flood-fill algorithm to find adjacent tiles.
void flood(Tile_List const& tiles, Tile_List& filled, Point<int> const& p)
{
    if (!tiles.contains(p) || filled.contains(p))
        return;
    filled.insert(p);
    flood(tiles, filled, {p.x+1, p.y});
    flood(tiles, filled, {p.x-1, p.y});
    flood(tiles, filled, {p.x, p.y+1});
    flood(tiles, filled, {p.x, p.y-1});
}

Figure::Figure()
{
}

Figure::Figure(std::initializer_list<Point<int>> ps)
{
    for (auto const& p : ps)
        m_tiles.insert(p);
}

bool Figure::is_contiguous() const
{
    // The figure is contiguous if all tiles are filled.
    Tile_List filled;
    flood(m_tiles, filled, *m_tiles.begin());
    return filled.size() == m_tiles.size();
}

Tile_List const& Figure::tiles() const
{
    return m_tiles;
}

Point<double> Figure::cm() const
{
    Point<double> cm{0.0, 0.0};
    for (auto const& tile : m_tiles)
        cm += tile;
    return m_tiles.empty() ? cm : cm /= m_tiles.size();
}

void Figure::toggle(Point<int> const& p)
{
    if (m_tiles.contains(p))
        m_tiles.erase(p);
    else
        m_tiles.insert(p);
}

void Figure::clear()
{
    m_tiles.clear();
}
