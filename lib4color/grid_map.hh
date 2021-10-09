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

#ifndef FOUR_COLOR_LIB4COLOR_GRID_MAP_HH_INCLUDED
#define FOUR_COLOR_LIB4COLOR_GRID_MAP_HH_INCLUDED

#include <figure.hh>
#include <figure_view.hh>

#include <gtkmm.h>

#include <vector>

/// A 2D field for displaying polyominos.
class Grid_Map : public Gtk::DrawingArea
{
    using Context = Cairo::RefPtr<Cairo::Context>;

public:
    /// Create a grid
    /// @param num_edge_tiles The number of squares in each direction in pixels.
    /// @param tile_size The width and height of each square in pixels.
    Grid_Map(int num_edge_tiles, int tile_size);

    /// @return The width of the field in pixels.
    int width() const;
    /// @return The height of the field plus the status area in pixels.
    int height() const;

private:
    /// DrawingArea methods
    /// @{
    virtual bool on_key_press_event(GdkEventKey* event) override;
    virtual bool on_button_press_event(GdkEventButton* event) override;
    virtual bool on_draw(Context const& cr) override;
    /// @}

    /// Change the figure that events apply to.
    void focus_next_figure();
    /// Render the configuration to the file "4color.png".
    void export_png();

    Figure m_figure;
    std::vector<Figure_View> m_views;
    std::vector<Figure_View>::iterator m_focused_figure{m_views.end()};
    int m_num_edge_tiles;
    int m_tile_size;
    bool m_write_to_file{false};
};

#endif // FOUR_COLOR_LIB4COLOR_GRID_MAP_HH_INCLUDED
