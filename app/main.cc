#include <grid_map.hh>

#include <gtkmm.h>

int main(int argc, char** argv)
{
  // Initialize gtkmm and create the main window
    auto app = Gtk::Application::create(argc, argv, "4-color");

    Gtk::Window window;
    Grid_Map grid(16, 20);
    window.add(grid);
    window.resize(grid.width(), grid.height());
    grid.show();

    return app->run(window);
}
