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

#include <grid_map.hh>

#include <cassert>
#include <iostream>
#include <list>
#include <numbers>
#include <numeric>

/// The Cairo drawing context.
using Context = Cairo::RefPtr<Cairo::Context>;
/// A map of colors to tiles.
using Figure_Map = std::map<Color, std::set<Point<int>>>;

constexpr Point<int> left{-1, 0};
constexpr Point<int> right{1, 0};
constexpr Point<int> up{0, 1};
constexpr Point<int> down{0, -1};

/// Set the color in the drawing context.
void set_color(Context const& cr, Color const& color,
               double factor = 1.0, double alpha = 1.0)
{
    auto [r, g, b] = color;
    cr->set_source_rgba(factor*r/255.0, factor*g/255.0, factor*b/255.0, alpha);
}

/// Draw the gridlines in a muted shade of the passed-in color.
void draw_grid(Context const& cr, Color color, int divisions, int separation)
{
    // Use muted, semi-transparent lines for the grid.
    set_color(cr, color, 0.6, 0.4);
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
                 bool four_color, int num_tiles,
                 std::size_t undo_pos, std::size_t num_undos)
{
    std::string undos{std::to_string(undo_pos) + "/" + std::to_string(num_undos)};
    std::vector<std::pair<std::string, bool>> states{{"C", is_contiguous},
                                                     {"V", all_visible},
                                                     {"4", four_color},
                                                     {std::to_string(num_tiles), false},
                                                     {"", false},
                                                     {undos, false}};
    Cairo::TextExtents te;
    auto y{height - 0.5*tile_size};
    for (auto i{0u}; auto const& state : states)
    {
        auto x{(i++ + 0.5)*tile_size};
        // Draw a green circle for true states.
        if (state.second)
        {
            set_color(cr, green);
            cr->move_to(x, y);
            cr->arc(x, y, 0.5*tile_size, 0, 2.0*std::numbers::pi);
            cr->fill();
        }
        // Draw the label
        set_color(cr, black);
        cr->get_text_extents(state.first, te);
        cr->move_to(x - te.x_bearing - 0.5*te.width, y - te.y_bearing - 0.5*te.height);
        cr->show_text(state.first);
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

// Grid_Map implementation

Grid_Map::Grid_Map(int num_edge_tiles, int tile_size)
    : m_num_edge_tiles(num_edge_tiles),
      m_tile_size(tile_size),
      m_image_export_chooser(
          std::make_unique<Gtk::FileChooserDialog>(
              "Save figure image", Gtk::FILE_CHOOSER_ACTION_SAVE, Gtk::DIALOG_MODAL))
{
    set_can_focus(true);
    add_events(Gdk::KEY_PRESS_MASK | Gdk::BUTTON_PRESS_MASK);

    m_image_export_chooser->set_modal(true);
    m_image_export_chooser->signal_response().connect(
        sigc::mem_fun(*this, &Grid_Map::export_png));
    m_image_export_chooser->add_button("Save", Gtk::RESPONSE_OK);
    m_image_export_chooser->add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    auto PNG_Filter{Gtk::FileFilter::create()};
    PNG_Filter->set_name("PNG files");
    PNG_Filter->add_mime_type("image/png");
    m_image_export_chooser->add_filter(PNG_Filter);

    // Add the views.
    for (auto i{0}; const auto& color : {red, yellow, green, blue})
        m_views.emplace_back(m_figure, Point{3*i++, 0}, color);
    m_focused_figure = m_views.begin();

    m_history.emplace_back(Figure(), m_views, m_focused_figure);
    m_now = m_history.begin();
}

void Grid_Map::focus_next_figure()
{
    ++m_focused_figure;
    if (m_focused_figure == m_views.end())
        m_focused_figure = m_views.begin();
}

void Grid_Map::record()
{
    assert (m_now != m_history.end());
    m_history.erase(std::next(m_now), m_history.end());
    m_history.emplace_back(m_figure, m_views, m_focused_figure);
    m_now = std::prev(m_history.end());
}

void Grid_Map::undo()
{
    assert (!m_history.empty());
    assert (m_now != m_history.end());
    if (m_now != m_history.begin())
        update(std::prev(m_now));
}

void Grid_Map::redo()
{
    assert (!m_history.empty());
    assert (m_now != m_history.end());
    if (std::next(m_now) != m_history.end())
        update(std::next(m_now));
}

void Grid_Map::reset()
{
    assert (!m_history.empty());
    update(m_history.begin());
}

void Grid_Map::update(std::deque<State>::const_iterator it)
{
    m_now = it;
    m_figure = m_now->figure;
    m_views = m_now->views;
    m_focused_figure = m_now->focused_figure;
}

void Grid_Map::do_transform(Figure_View& (Figure_View::*fcn)(Point<int>),
                            bool all,
                            Point<int> arg)
{
    for_each(all ? m_views.begin() : m_focused_figure,
             all ? m_views.end() : std::next(m_focused_figure),
             std::bind(fcn, std::placeholders::_1, arg));
}

void Grid_Map::do_transform(Figure_View& (Figure_View::*fcn)(), bool all)
{
    //11 For rotating and flipping, I'd really like to flip/rotate about the center of the
    //11 grid. Right now it flips/rotates each figure about its cm.
    for_each(all ? m_views.begin() : m_focused_figure,
             all ? m_views.end() : std::next(m_focused_figure),
             std::bind(fcn, std::placeholders::_1));
}

bool Grid_Map::on_key_press_event(GdkEventKey* event)
{
    if (event->keyval == GDK_KEY_z)
        undo();
    else if (event->keyval == GDK_KEY_y)
        redo();
    else if (event->keyval == GDK_KEY_c)
        reset();
    else
    {
        auto shift{event->state & Gdk::ModifierType::SHIFT_MASK};
        switch (event->keyval)
        {
        case GDK_KEY_Left:
            do_transform(&Figure_View::translate, shift, left);
            break;
        case GDK_KEY_Right:
            do_transform(&Figure_View::translate, shift, right);
            break;
        case GDK_KEY_Up:
            do_transform(&Figure_View::translate, shift, up);
            break;
        case GDK_KEY_Down:
            do_transform(&Figure_View::translate, shift, down);
            break;
        case GDK_KEY_Page_Up:
            do_transform(&Figure_View::rotate_ccw, shift);
            break;
        case GDK_KEY_Page_Down:
            do_transform(&Figure_View::rotate_cw, shift);
            break;
        case GDK_KEY_space: // Flip
            do_transform(&Figure_View::flip_y, shift);
            break;
        case GDK_KEY_Tab: // Focus
            focus_next_figure();
            break;
        case GDK_KEY_w: // Write
            m_image_export_chooser->show();
            break;
        case GDK_KEY_q: // Quit
            Gtk::Main::quit();
            break;
        default:
            return true;
        }
        record();
    }
    queue_draw();
    return true;
}

bool Grid_Map::on_button_press_event(GdkEventButton* event)
{
    m_focused_figure->toggle(Point{static_cast<int>(event->x)/m_tile_size,
                                   static_cast<int>(height() - event->y)/m_tile_size - 1});
    record();
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
            plotted[fig.color()].insert(tile);
            cr->rectangle(tile.x, m_num_edge_tiles - tile.y - 1, 1, 1);
        }
        cr->fill();
    }
    cr->set_matrix(m1);

    auto num_tiles{m_figure.tiles().size()};
    auto all_visible{num_visible(plotted) == m_views.size()*num_tiles};
    if (!m_write_to_file)
        draw_status(cr, height(), m_tile_size,
                    m_figure.is_contiguous(), all_visible,
                    needs_four_colors(plotted), num_tiles,
                    std::distance(m_history.cbegin(), m_now) + 1, m_history.size());

    return true;
}

void Grid_Map::export_png(int response)
{
    m_image_export_chooser->hide();
    if (response != Gtk::RESPONSE_OK)
        return;

    m_write_to_file = true;
    auto surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, width(), width());
    auto cr = Cairo::Context::create(surface);
    on_draw(cr);
    surface->write_to_png(m_image_export_chooser->get_file()->get_path());
    m_write_to_file = false;
}
