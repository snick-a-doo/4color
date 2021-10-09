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

#include <grid_map.hh>

#include <iostream>
#include <list>
#include <numbers>
#include <numeric>

/// The Cairo drawing context.
using Context = Cairo::RefPtr<Cairo::Context>;
/// A map of colors to tiles.
using Figure_Map = std::map<Color, std::set<Point<int>>>;

/// Set the color in the drawing context.
void set_color(Context const& cr, Color const& color,
               double factor = 1.0, double alpha = 1.0)
{
    auto [r, g, b] = color;
    cr->set_source_rgba(factor*r, factor*g, factor*b, alpha);
}

/// Draw the gridlines in a muted shade of the passed-in color.
void draw_grid(Context const& cr, Color color, int divisions, int separation)
{
    set_color(cr, color, 0.5, 0.3);
    cr->set_line_width(1);
    auto width{separation*divisions};
    for (auto i{0}; i < divisions; ++i)
    {
        cr->move_to(i*separation, 0);
        cr->line_to(i*separation, width);
        cr->move_to(0, i*separation);
        cr->line_to(width, i*separation);
    }
    cr->move_to(0, divisions*separation);
    cr->line_to(width, divisions*separation);
    cr->stroke();
}

/// Draw status info in the gap at the bottom.
void draw_status(Context const& cr, int height, int tile_size,
                 bool is_contiguous, bool all_visible,
                 bool four_color, int num_tiles)
{
    std::string labels{"CV4" + std::to_string(num_tiles)};
    std::array states{is_contiguous, all_visible, four_color, false};
    Cairo::TextExtents te;
    auto y{height - 0.5*tile_size};
    for (auto i{0u}; i < labels.length(); ++i)
    {
        auto x{(i + 0.5)*tile_size};
        // Draw a green circle for true states.
        if (states[i])
        {
            set_color(cr, green);
            cr->move_to(x, y);
            cr->arc(x, y, 0.5*tile_size, 0, 2.0*std::numbers::pi);
            cr->fill();
        }
        // Draw the label
        std::string label{labels[i]};
        set_color(cr, black);
        cr->get_text_extents(label, te);
        cr->move_to(x - te.x_bearing - 0.5*te.width, y - te.y_bearing - 0.5*te.height);
        cr->show_text(label);
    }
}

/// @return The number of visible tiles. This is less than the total number of tiles if
/// views overlap.
std::size_t num_visible(Figure_Map const& fm)
{
    // Merge the set of the views. Duplicate tiles will be represented only once.
    return std::accumulate(
        fm.begin(), fm.end(), std::set<Point<int>>(),
        [](auto const& ps, auto const& p){
            auto p1{ps};
            auto p2{p.second};
            p1.merge(p2);
            return p1;
        }).size();
}

/// @return True if the tile at @p it shares an edge with at least one tile from the
/// figures in the container up to @p end.
bool touches_all(Figure_Map::const_iterator it, Figure_Map::const_iterator end)
{
    auto touches = [](Point<int> const& tile, std::set<Point<int>> const& others) {
        for (auto other : others)
            if ((std::abs(tile.x - other.x) == 1 && tile.y == other.y)
                || (tile.x == other.x && std::abs(tile.y - other.y) == 1))
                return true;
        return false;
    };

    auto& tiles = it->second;
    for (auto other = std::next(it); other != end; ++other)
        if (std::all_of(tiles.begin(), tiles.end(),
                        [touches, other](auto tile) { return !touches(tile, other->second); }))
            return false;
    return true;
}

/// @return True if the configuration represented by @p fn requires four colors.
bool needs_four_colors(Figure_Map const& fm)
{
    for (auto itm = fm.begin(); itm != fm.end(); ++itm)
        if (!touches_all(itm, fm.end()))
            return false;
    return !fm.empty();
}

Grid_Map::Grid_Map(int num_edge_tiles, int tile_size)
    : m_num_edge_tiles(num_edge_tiles),
      m_tile_size(tile_size)
{
    set_can_focus(true);
    add_events(Gdk::KEY_PRESS_MASK | Gdk::BUTTON_PRESS_MASK);

    // Add the views.
    for (auto i{0}; const auto& color : {red, yellow, green, blue})
        m_views.emplace_back(m_figure, Point{3*i++, 0}, color);
    m_focused_figure = m_views.begin();
}

void Grid_Map::focus_next_figure()
{
    ++m_focused_figure;
    if (m_focused_figure == m_views.end())
        m_focused_figure = m_views.begin();
}

bool Grid_Map::on_key_press_event(GdkEventKey* event)
{
    switch (event->keyval)
    {
    case GDK_KEY_Left:
        m_focused_figure->translate({-1, 0});
        break;
    case GDK_KEY_Right:
        m_focused_figure->translate({1, 0});
        break;
    case GDK_KEY_Up:
        m_focused_figure->translate({0, 1});
        break;
    case GDK_KEY_Down:
        m_focused_figure->translate({0, -1});
        break;
    case GDK_KEY_Page_Up:
        m_focused_figure->rotate_ccw();
        break;
    case GDK_KEY_Page_Down:
        m_focused_figure->rotate_cw();
        break;
    case GDK_KEY_space: // Flip
        m_focused_figure->flip_x();
        break;
    case GDK_KEY_Tab: // Focus
        focus_next_figure();
        break;
    case GDK_KEY_c: // Clear
        // Erase tiles and reset each position and orientation.
        m_figure.clear();
        for (auto& fig : m_views)
            fig.reset();
        break;
    case GDK_KEY_w: // Write
        export_png();
        break;
    case GDK_KEY_q: // Quit
        Gtk::Main::quit();
        break;
    }
    queue_draw();
    return true;
}

bool Grid_Map::on_button_press_event(GdkEventButton* event)
{
    Point<double> p{event->x, height() - event->y};
    m_focused_figure->toggle(round<int>(p, 1.0/m_tile_size));
    queue_draw();
    return true;
}

int Grid_Map::width() const
{
    return m_num_edge_tiles*m_tile_size;
}

int Grid_Map::height() const
{
    return width() + m_tile_size;
}

bool Grid_Map::on_draw(Context const& cr)
{
    if (!m_write_to_file)
        draw_grid(cr, m_focused_figure->color(), m_num_edge_tiles, m_tile_size);

    std::map<Color, std::set<Point<int>>> plotted;
    auto m1{cr->get_matrix()};
    auto focus_index{std::distance(m_views.begin(), m_focused_figure)};
    cr->scale(m_tile_size, m_tile_size);
    for (auto i{0u}; i < m_views.size(); ++i)
    {
        // Draw the other figures before the focused figure.
        const auto& fig{m_views[(i + focus_index + 1) % m_views.size()]};
        set_color(cr, fig.color());
        for (auto const& tile : fig.tiles())
        {
            auto x{static_cast<double>(tile.x)};
            auto y{static_cast<double>(tile.y)};
            // cr->user_to_device(x, y);
            plotted[fig.color()].insert(tile);
            cr->rectangle(x, m_num_edge_tiles - y, 1, 1);
        }
        cr->fill();
    }
    cr->set_matrix(m1);

    auto num_tiles{m_figure.tiles().size()};
    auto all_visible{num_visible(plotted) == m_views.size()*num_tiles};
    if (!m_write_to_file)
        draw_status(cr, height(), m_tile_size,
                    m_figure.is_contiguous(), all_visible,
                    needs_four_colors(plotted), num_tiles);

    return true;
}

void Grid_Map::export_png()
{
    m_write_to_file = true;
    auto surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, width(), width());
    auto cr = Cairo::Context::create(surface);
    on_draw(cr);
    surface->write_to_png("4color.png");
    std::cout << "wrote\n";
    m_write_to_file = false;
}
