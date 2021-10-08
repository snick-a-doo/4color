#ifndef FOUR_COLOR_LIB4COLOR_GRID_MAP_HH_INCLUDED
#define FOUR_COLOR_LIB4COLOR_GRID_MAP_HH_INCLUDED

#include <figure.hh>
#include <figure_view.hh>

#include <gtkmm.h>

#include <vector>

class Grid_Map : public Gtk::DrawingArea
{
    using Context = Cairo::RefPtr<Cairo::Context>;

public:
    Grid_Map(int num_edge_tiles, int tile_size);

    int width() const;
    int height() const;

private:
    virtual bool on_key_press_event(GdkEventKey* event) override;
    virtual bool on_button_press_event(GdkEventButton* event) override;
    virtual bool on_draw(Context const& cr) override;

    void focus_next_figure();
    void draw_grid(Context const& cr) const;
    void export_png();

    Point<int> point(int x, int y) const;

    Figure m_figure;
    std::vector<Figure_View> m_views;
    std::vector<Figure_View>::iterator m_focused_figure{m_views.end()};
    int m_num_edge_tiles;
    int m_tile_size;
    bool m_write_to_file{false};
};

#endif // FOUR_COLOR_LIB4COLOR_GRID_MAP_HH_INCLUDED
