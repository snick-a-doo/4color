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
#include "figure_view.hh"

#include <cassert>

template <typename Container>
Point<double> cm(Container const& tiles)
{
    Point<double> cm{0.0, 0.0};
    for (auto const& tile : tiles)
        cm += tile;
    return tiles.empty() ? cm : cm /=tiles.size();
}

// Transformation matrices for reflections and 90-degree rotations.
Matrix constexpr Rl{ 0, -1,  1,  0}; // Left (CCW) rotation
Matrix constexpr Rr{ 0,  1, -1,  0}; // Right (CW) rotation
Matrix constexpr Fx{ 1,  0,  0, -1}; // Flip about x-axis
Matrix constexpr Fy{-1,  0,  0,  1}; // Flip about y-axis

/// Multiply transformation matrices.
Matrix operator*(Matrix const& m1, Matrix const& m2)
{
    return {m1.xx*m2.xx + m1.xy*m2.yx, m1.xx*m2.xy + m1.xy*m2.yy,
            m1.yx*m2.xx + m1.yy*m2.yx, m1.yx*m2.xy + m1.yy*m2.yy};
}

std::ostream& operator<<(std::ostream& os, Matrix const& m)
{
    return os << '[' << m.xx << ' ' << m.xy << " / " << m.yx << ' ' << m.yy << ']';
}

std::ostream& operator<<(std::ostream& os, Tile_List const& tiles)
{
    for (auto t : tiles)
        os << t;
    return os;
}

/// Apply a transformation to a point.
template <typename T>
Point<T> operator*(Matrix const& m1, Point<T> const& p)
{
    return {m1.xx*p.x + m1.xy*p.y, m1.yx*p.x + m1.yy*p.y};
}

/// @return The transpose of a matrix - the inverse of a transformation matrix.
Matrix transpose(Matrix const& m)
{
    return {m.xx, m.yx,
            m.xy, m.yy};
}

using VTiles = std::vector<Point<double>>;

/// Change the tiles' positions.
void do_translate(Point<double> dr, VTiles& tiles)
{
    for (auto& tile : tiles)
        tile += dr;
}

/// Rotate or reflect the tiles.
void transform(Matrix const& m, VTiles& tiles)
{
    for (auto& tile : tiles)
        tile = m*tile;
}

Figure_View::Figure_View(Figure& fig, Point<int> position, Color const& color)
    : m_figure{fig},
      m_dr{to_double(position)},
      m_color{color}
{
}

Figure_View& Figure_View::operator=(Figure_View const& rhs)
{
    m_dr = rhs.m_dr;
    m_dcm = rhs.m_dcm;
    m_transform = rhs.m_transform;
    return *this;
}

Color Figure_View::color() const
{
    return m_color;
}

Tile_List Figure_View::tiles() const
{
    if (m_figure.tiles().empty())
        return {};

    VTiles tiles;
    for (auto t : (m_figure.tiles()))
        tiles.emplace_back(t.x, t.y);

    auto r{cm(tiles)};
    do_translate(-r, tiles);
    transform(m_transform, tiles);
    do_translate(r, tiles);
    do_translate(m_dcm, tiles);
    do_translate(m_dr, tiles);
    auto p{to_double(flooround(tiles.front()))};
    m_dr += p - tiles.front();

    Tile_List rounded;
    for (auto t : tiles)
        rounded.insert(flooround(t));
    return rounded;
}

Figure_View& Figure_View::toggle(Point<int> p)
{
    VTiles tiles;
    for (auto t : (m_figure.tiles()))
        tiles.emplace_back(t.x, t.y);
    auto r{cm(tiles)};

    // Un-transform the point.
    VTiles tile{to_double(p)};
    do_translate(-m_dr, tile);
    do_translate(-m_dcm, tile);
    do_translate(-r, tile);
    transform(transpose(m_transform), tile);
    do_translate(r, tile);

    m_figure.toggle(flooround(tile.front()));
    auto dcm{cm(m_figure.tiles()) - r};
    m_dcm += m_transform*dcm - dcm;

    return *this;
}

Figure_View& Figure_View::translate(Point<int> dr)
{
    m_dr += dr;
    return *this;
}

Figure_View& Figure_View::flip_x()
{
    m_transform = Fx*m_transform;
    return *this;
}

Figure_View& Figure_View::flip_y()
{
    m_transform = Fy*m_transform;
    return *this;
}

Figure_View& Figure_View::rotate_ccw()
{
    m_transform = Rl*m_transform;
    return *this;
}

Figure_View& Figure_View::rotate_cw()
{
    m_transform = Rr*m_transform;
    return *this;
}

std::ostream& operator<<(std::ostream& os, Figure_View const& f)
{
    // True if p1 is to the left of p2.
    auto x_less = [](Point<int> const& p1, Point<int> const& p2){
        return p1.x < p2.x;
    };
    // True if p1 comes before p2 scanning left to right, top to bottom.
    auto raster = [](Point<int> const& p1, Point<int> const& p2){
        return p1.y > p2.y || (p1.y == p2.y && p1.x < p2.x);
    };

    // Get the transformed tiles in the order they will be sent to the stream.
    std::vector<Point<int>> ps;
    for (auto t : f.tiles())
        ps.push_back(t);
    std::sort(ps.begin(), ps.end(), raster);

    auto it_ps = ps.begin();
    auto [it_x_min, it_x_max] = std::minmax_element(ps.begin(), ps.end(), x_less);
    for (auto y{ps.front().y}; y >= ps.back().y; --y)
    {
        // Scan across the row from smallest to largest x.
        for (auto x{it_x_min->x}; x <= it_x_max->x; ++x)
        {
            // If there's a tile at (x,y) send a symbol.
            if (it_ps != ps.end() && Point{x, y} == *it_ps)
            {
                os << "# ";
                ++it_ps;
                continue;
            }
            os << ". ";
        }
        os << '\n';
    }
    // Print the coordinates of the lower left slot.
    return os << it_x_min->x << ',' << ps.back().y << '\n';
}
