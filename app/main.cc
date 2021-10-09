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

#include <gtkmm.h>

int main(int argc, char** argv)
{
  // Initialize gtkmm and create the main window
    auto app = Gtk::Application::create(argc, argv, "4color");

    Gtk::Window window;
    Grid_Map grid(16, 20);
    window.add(grid);
    window.resize(grid.width(), grid.height());
    grid.show();

    return app->run(window);
}
