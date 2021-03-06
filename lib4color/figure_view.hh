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

#ifndef FOUR_COLOR_LIB4COLOR_FIGURE_VIEW_HH_INCLUDED
#define FOUR_COLOR_LIB4COLOR_FIGURE_VIEW_HH_INCLUDED

#include <iostream>
#include <tuple>
#include <vector>

class Figure;

/// RGB 0.0 to 1.0
using Color = std::tuple<int, int, int>;

/// Some pre-defined colors from the Brewer palette.
/// @{
constexpr Color black{0, 0, 0};
constexpr Color red{277, 74, 51};
constexpr Color yellow{254, 217, 142};
constexpr Color green{44, 162, 95};
constexpr Color blue{5, 112, 176};
/// @}

/// An integer transformation matrix for reflections and 90-degree rotations.
struct Matrix
{
    int xx{1}, xy{0};
    int yx{0}, yy{1};
};

/// A transformed polyomino figure
class Figure_View
{
public:
    Figure_View(Figure& fig, Point<int> position, Color const& color);

    Figure_View& operator=(Figure_View const& rhs);

    /// @return A vector of transformed tiles.
    Tile_List tiles() const;
    /// @return The view's color.
    Color color() const;

    /// Add or remove a tile from source figure.
    Figure_View& toggle(Point<int> p);

    /// Transformations applied to the points returned by tiles(). The source figure is
    /// not changed.
    /// @{
    /// Move the view by an integer amount.
    Figure_View& translate(Point<int> dr);
    /// Reflect about the x-axis.
    Figure_View& flip_x();
    /// Reflect about the y-axis.
    Figure_View& flip_y();
    /// Rotate 90° counter-clockwise.
    Figure_View& rotate_ccw();
    /// Rotate 90° clockwise.
    Figure_View& rotate_cw();
    /// @}
private:
    /// The source polyomino.
    Figure& m_figure;
    /// The current position.
    mutable Point<double> m_dr;
    Point<double> m_dcm;
    /// The current transformation matrix.
    Matrix m_transform;
    /// The displayed color.
    const Color m_color;
};

/// Send an ASCII representation of the transformed figure.
std::ostream& operator<<(std::ostream& os, Figure_View const& f);

#endif // FOUR_COLOR_LIB4COLOR_FIGURE_VIEW_HH_INCLUDED
