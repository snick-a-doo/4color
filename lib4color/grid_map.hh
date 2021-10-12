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

#ifndef FOUR_COLOR_LIB4COLOR_GRID_MAP_HH_INCLUDED
#define FOUR_COLOR_LIB4COLOR_GRID_MAP_HH_INCLUDED

#include <figure.hh>
#include <figure_view.hh>

#include <gtkmm.h>

#include <deque>
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

    int m_num_edge_tiles;
    int m_tile_size;
    bool m_write_to_file{false};

    Figure m_figure;
    std::vector<Figure_View> m_views;
    std::vector<Figure_View>::iterator m_focused_figure;

    /// Apply the transformation function to all figures if @p all is true. Otherwise
    /// transform just the focused figure.
    void do_transform(Figure_View& (Figure_View::*fcn)(Point<int>), bool all, Point<int> arg);
    void do_transform(Figure_View& (Figure_View::*fcn)(), bool all);

    // History management
    //11 Extract class?
    //11 Use history instead of members above for current state?

    struct State
    {
        Figure figure;
        std::vector<Figure_View> views;
        std::vector<Figure_View>::iterator focused_figure;
    };
    /// Erase states after m_now and add the current state.
    void record();
    /// Move backward through the states.
    void undo();
    /// Move forward through the states.
    void redo();
    /// Go to the initial state.
    void reset();
    /// Set the current state to the one pointed to by the iterator.
    void update(std::deque<State>::const_iterator it);
    /// The accumulated states.
    std::deque<State> m_history;
    /// An iterator to the current state.
    std::deque<State>::const_iterator m_now;
};

#endif // FOUR_COLOR_LIB4COLOR_GRID_MAP_HH_INCLUDED
