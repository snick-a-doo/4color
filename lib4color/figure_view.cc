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

#include "figure.hh"
#include "figure_view.hh"

#include <cassert>
#include <cmath>

// Transformation matrices for reflections and 90-degree rotations.
Matrix constexpr Rl{ 0, -1,  1,  0}; // Left (CCW) rotation
Matrix constexpr Rr{ 0,  1, -1,  0}; // Right (CW) rotation
Matrix constexpr Fx{ 1,  0,  0, -1}; // Flip about x-axis
Matrix constexpr Fy{-1,  0,  0,  1}; // Flip about y-axis

using VTiles = std::vector<Point<int>>;

/// Multiply transformation matrices.
Matrix operator*(Matrix const& m1, Matrix const& m2)
{
    return {m1.xx*m2.xx + m1.xy*m2.yx, m1.xx*m2.xy + m1.xy*m2.yy,
            m1.yx*m2.xx + m1.yy*m2.yx, m1.yx*m2.xy + m1.yy*m2.yy};
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

/// @return An integer point found by rounding a real point after scaling.
Point<int> round(Point<double> const& p, double factor = 1.0)
{
    return {static_cast<int>(std::round(p.x * factor)),
            static_cast<int>(std::round(p.y * factor))};
}

/// Change the tiles' positions.
void do_translate(Point<int> dr, VTiles& tiles)
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

/// Multiply the tile positions by an integer.
void expand(int factor, VTiles& tiles)
{
    for (auto& tile : tiles)
        tile *= factor;
}

/// Divide the tile positions by an integer.
void shrink(int factor, VTiles& tiles)
{
    for (auto& tile : tiles)
        tile /= factor;
}

Figure_View::Figure_View(Figure& fig, Point<int> position, Color const& color)
    : m_figure{fig},
      m_dr{static_cast<double>(position.x), static_cast<double>(position.y)},
      m_color{color}
{
}

Figure_View& Figure_View::operator=(Figure_View const& rhs)
{
    m_dr = rhs.m_dr;
    m_transform = rhs.m_transform;
    return *this;
}

Color Figure_View::color() const
{
    return m_color;
}

VTiles Figure_View::tiles() const
{
    if (m_figure.tiles().empty())
        return {};

    // The rotation axis may be at the corners of tiles, i.e. 1/2-integer coordinates.
    // Magnify the figure by a factor of 2 so the corners are at integer coordinates.
    VTiles tiles(m_figure.tiles().begin(), m_figure.tiles().end());

    auto r2{round(m_figure.cm(), 2.0)};

    expand(2, tiles);
    // Move the expanded tiles to the origin.
    do_translate(-r2, tiles);
    // Rotate and flip.
    transform(m_transform, tiles);

    // Move the figure back to where it started and contract it to normal size. Rotating
    // can put tiles on odd coordinates. Adjust the translation to ensure even
    // coordinates.
    if ((tiles.front().x + r2.x) % 2 != 0)
        --r2.x;
    if ((tiles.front().y + r2.y) % 2 != 0)
        --r2.y;
    do_translate(r2, tiles);
    // Shrink the figure to its original size.
    shrink(2, tiles);

    // Finally, translate the figure.
    do_translate(round(m_dr), tiles);
    return tiles;
}

Figure_View& Figure_View::toggle(Point<int> p)
{
    auto cm1{m_figure.cm()};

    auto r2{round(m_figure.cm(), 2.0)};
    // Un-transform the point.
    VTiles tiles{p};
    Point<int> dr{round(m_dr, 0.9999)}; //11 Fix
    do_translate(-dr, tiles);
    expand(2, tiles);
    do_translate(-r2, tiles);
    transform(transpose(m_transform), tiles);
    do_translate(r2, tiles);
    shrink(2, tiles);
    m_figure.toggle(tiles.front());

    // Account for the change in the center of mass.
    auto cm2{m_figure.cm()};
    auto dcm{cm2 - cm1};
    m_dr += m_transform*dcm - dcm;

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
    VTiles ps{f.tiles()};
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
