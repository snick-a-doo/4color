#include <grid_map.hh>

#include <iostream>
#include <list>
#include <numbers>
#include <numeric>

using Context = Cairo::RefPtr<Cairo::Context>;

constexpr auto grid_grey{0.8};

void set_color(Context const& cr, Color const& color,
               double factor = 1.0, double alpha = 1.0)
{
    auto [r, g, b] = color;
    cr->set_source_rgba(factor*r, factor*g, factor*b, alpha);
}

Grid_Map::Grid_Map(int num_edge_tiles, int tile_size)
    : m_num_edge_tiles(num_edge_tiles),
      m_tile_size(tile_size)
{
    set_can_focus(true);
    add_events(Gdk::KEY_PRESS_MASK | Gdk::BUTTON_PRESS_MASK);

    auto x{-3};
    for (const auto& color : {red, yellow, green, blue})
        m_views.emplace_back(m_figure, Point{x += 3, 0}, color);
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

Point<int> Grid_Map::point(int x, int y) const
{
    return Point<int>(x, height() - y)/m_tile_size;
}

bool Grid_Map::on_button_press_event(GdkEventButton* event)
{
    m_focused_figure->toggle(point(event->x, event->y));
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

void Grid_Map::draw_grid(Context const& cr) const
{
    set_color(cr, m_focused_figure->color(), 0.5, 0.3);
    cr->set_line_width(1);
    for (auto i{0}; i < m_num_edge_tiles; ++i)
    {
        cr->move_to(i*m_tile_size, 0);
        cr->line_to(i*m_tile_size, width());
        cr->move_to(0, i*m_tile_size);
        cr->line_to(width(), i*m_tile_size);
    }
    cr->move_to(0, m_num_edge_tiles*m_tile_size);
    cr->line_to(width(), m_num_edge_tiles*m_tile_size);
    cr->stroke();
}

void draw_status(Context const& cr, int height, int tile_size,
                 bool is_contiguous, bool all_visible, bool four_color)
{
    using std::numbers::pi;

    std::string labels{"CV4"};
    std::array states{is_contiguous, all_visible, four_color};
    Cairo::TextExtents te;
    for (auto i{0u}; i < labels.length(); ++i)
    {
        auto x{(i + 0.5)*tile_size};
        auto y{height - 0.5*tile_size};
        if (states[i])
        {
            set_color(cr, green);
            cr->move_to(x, y);
            cr->arc(x, y, 0.5*tile_size, 0, 2.0*pi);
            cr->fill();
        }
        auto label{labels.substr(i, 1)};
        set_color(cr, black);
        cr->get_text_extents(label, te);
        cr->move_to(x - te.x_bearing - 0.5*te.width, y - te.y_bearing - 0.5*te.height);
        cr->show_text(label);
    }
}

using Figure_Map = std::map<Color, std::set<Point<int>>>;

std::size_t num_visible(Figure_Map const& fm)
{
    return std::accumulate(
        fm.begin(), fm.end(), std::set<Point<int>>(),
        [](auto const& ps, auto const& p){
            auto p1{ps};
            auto p2{p.second};
            p1.merge(p2);
            return p1;
        }).size();
}

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

bool needs_four_colors(Figure_Map const& fm)
{
    for (auto itm = fm.begin(); itm != fm.end(); ++itm)
        if (!touches_all(itm, fm.end()))
            return false;
    return !fm.empty();
}

bool Grid_Map::on_draw(Context const& cr)
{
    using std::numbers::pi;

    if (!m_write_to_file)
        draw_grid(cr);

    std::map<Color, std::set<Point<int>>> plotted;
    auto m1{cr->get_matrix()};
    cr->scale(m_tile_size, m_tile_size);
    for (auto const& fig : m_views)
    {
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

    auto all_visible{num_visible(plotted) == m_views.size()*m_figure.tiles().size()};
    if (!m_write_to_file)
        draw_status(cr, height(), m_tile_size,
                    m_figure.is_contiguous(), all_visible, needs_four_colors(plotted));

    return true;
}

void Grid_Map::export_png()
{
    m_write_to_file = true;
    auto surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, width(), width());
    auto cr = Cairo::Context::create(surface);
    on_draw(cr);
    surface->write_to_png("4-color.png");
    std::cout << "wrote\n";
    m_write_to_file = false;
}
