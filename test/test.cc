#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "figure.hh"
#include "figure_view.hh"

#include "doctest.h"

Point<int> here{0, 0};

TEST_CASE("initial figure")
{
    Figure f;
    CHECK(f.tiles().empty());
    CHECK(f.is_contiguous());
    CHECK(f.cm() == Point{0.0, 0.0});
}

TEST_CASE("one tile")
{
    Figure f;
    f.toggle({13, 19});
    CHECK(f.tiles().size() == 1);
    CHECK(f.is_contiguous());
    CHECK(f.cm() == Point{13.0, 19.0});
}

TEST_CASE("discontiguous")
{
    Figure f;
    // Corners touch SW/NE
    f.toggle({13, 19});
    f.toggle({14, 20});
    CHECK(f.tiles().size() == 2);
    CHECK(!f.is_contiguous());
    CHECK(f.cm() == Point{13.5, 19.5});
    // Fill in SE
    f.toggle({14, 19});
    CHECK(f.tiles().size() == 3);
    CHECK(f.is_contiguous());
}

TEST_CASE("turn off")
{
    Figure f;
    f.toggle({1, 1});
    f.toggle({1, 2});
    f.toggle({1, 3});
    f.toggle({2, 1});

    f.toggle({1, 3});
    CHECK(f.tiles().size() == 3);
    CHECK(f.is_contiguous());
    f.toggle({1, 1});
    CHECK(f.cm() == Point{1.5, 1.5});
    CHECK(f.tiles().size() == 2);
    CHECK(!f.is_contiguous());
    f.toggle({2, 1});
    f.toggle({1, 2});
    CHECK(f.tiles().empty());
    CHECK(f.is_contiguous());
    CHECK(f.cm() == Point{0.0, 0.0});
}

std::ostream& operator<<(std::ostream& os, Tile_List const& tiles)
{
    for (auto const& tile : tiles)
        os << tile;
    return os;
}

bool same_tiles(VTiles const& prime, Tile_List const& expected)
{
    Tile_List ps{prime.begin(), prime.end()};
    if (ps == expected)
        return true;
    std::cerr << ps << " != " << expected << std::endl;
    return false;
}

TEST_CASE("view tranform")
{
    Figure dot{{-2, 0}}; // center on tile
    Figure x_bar2{{8, 7}, {9, 7}};  // center on edge
    Figure y_bar2{{0, 0}, {0, -1}}; // center on edge
    Figure x_bar3{{8, 7}, {9, 7}, {10, 7}};  // center on tile
    Figure y_bar3{{0, 0}, {0, -1}, {0, -2}}; // center on tile
    Figure square{{2, 2}, {2, 3}, {3, 3}, {3, 2}}; // center on corner;
    Figure ell{{1, 1},  //
               {1, 2},  //  #
               {1, 3},  //  #
               {2, 1}}; //  # #
    Figure tee{{1, 2},
               {2, 2},
               {3, 2},  //  # # #
               {2, 1}}; //    #

    Figure_View dot_v(dot, here, black);
    Figure_View x_bar2_v(x_bar2, here, black);
    Figure_View y_bar2_v(y_bar2, here, black);
    Figure_View x_bar3_v(x_bar3, here, black);
    Figure_View y_bar3_v(y_bar3, here, black);
    Figure_View square_v(square, here, black);
    Figure_View ell_v(ell, here, black);
    Figure_View tee_v(tee, here, black);

    SUBCASE("no transform")
    {
        CHECK(same_tiles(dot_v.tiles(), dot.tiles()));
        CHECK(same_tiles(x_bar2_v.tiles(), x_bar2.tiles()));
        CHECK(same_tiles(y_bar2_v.tiles(), y_bar2.tiles()));
        CHECK(same_tiles(x_bar3_v.tiles(), x_bar3.tiles()));
        CHECK(same_tiles(y_bar3_v.tiles(), y_bar3.tiles()));
        CHECK(same_tiles(square_v.tiles(), square.tiles()));
        CHECK(same_tiles(ell_v.tiles(), ell.tiles()));
        CHECK(same_tiles(tee_v.tiles(), tee.tiles()));
    }
    SUBCASE("translate")
    {
        CHECK(same_tiles(dot_v.translate({-2, 0}).tiles(),
                         {{-4, 0}}));
        CHECK(same_tiles(x_bar2_v.translate({0, 3}).tiles(),
                         {{8, 10}, {9, 10}}));
        CHECK(same_tiles(y_bar2_v.translate({-2, 1}).tiles(),
                         {{-2, 1}, {-2, 0}}));
        CHECK(same_tiles(x_bar3_v.translate({1, 3}).tiles(),
                         {{9, 10}, {10, 10}, {11, 10}}));
        CHECK(same_tiles(y_bar3_v.translate({0, 0}).tiles(),
                         {{0, 0}, {0, -1}, {0, -2}}));
        CHECK(same_tiles(square_v.translate({1, 3}).tiles(),
                         {{3, 5}, {3, 6}, {4, 6}, {4, 5}}));
        CHECK(same_tiles(ell_v.translate({2, 1}).tiles(),
                         {{3, 2}, {3, 3}, {3, 4}, {4, 2}}));
        CHECK(same_tiles(tee_v.translate({1, -3}).tiles(),
                         {{2, -1}, {3, -1}, {4, -1}, {3, -2}}));
    }
    SUBCASE("flip x")
    {
        CHECK(same_tiles(dot_v.flip_x().tiles(), dot.tiles()));
        CHECK(same_tiles(x_bar2_v.flip_x().tiles(), x_bar2.tiles()));
        CHECK(same_tiles(y_bar2_v.flip_x().tiles(), y_bar2.tiles()));
        CHECK(same_tiles(x_bar3_v.flip_x().tiles(), x_bar3.tiles()));
        CHECK(same_tiles(y_bar3_v.flip_x().tiles(), y_bar3.tiles()));
        CHECK(same_tiles(square_v.flip_x().tiles(), square.tiles()));
        CHECK(same_tiles(ell_v.flip_x().tiles(),
                         {{1, 3}, {1, 2}, {1, 1}, {2, 3}}));
        CHECK(same_tiles(tee_v.flip_x().tiles(),
                         {{2, 3}, {1, 2}, {2, 2}, {3, 2}}));
    }
    SUBCASE("flip y")
    {
        CHECK(same_tiles(dot_v.flip_y().tiles(), dot.tiles()));
        CHECK(same_tiles(x_bar2_v.flip_y().tiles(), x_bar2.tiles()));
        CHECK(same_tiles(y_bar2_v.flip_y().tiles(), y_bar2.tiles()));
        CHECK(same_tiles(x_bar3_v.flip_y().tiles(), x_bar3.tiles()));
        CHECK(same_tiles(y_bar3_v.flip_y().tiles(), y_bar3.tiles()));
        CHECK(same_tiles(square_v.flip_y().tiles(), square.tiles()));
        CHECK(same_tiles(ell_v.flip_y().tiles(),
                         {{1, 1}, {2, 1}, {2, 2}, {2, 3}}));
        CHECK(same_tiles(tee_v.flip_y().tiles(),
                         {{2, 1}, {3, 2}, {2, 2}, {1, 2}}));
    }
    SUBCASE("rotate ccw")
    {
        CHECK(same_tiles(dot_v.rotate_ccw().tiles(), dot.tiles()));
        CHECK(same_tiles(x_bar2_v.rotate_ccw().tiles(),
                         {{8, 6}, {8, 7}}));
        CHECK(same_tiles(y_bar2_v.rotate_ccw().tiles(),
                         {{-1, -1}, {0, -1}}));
        CHECK(same_tiles(x_bar3_v.rotate_ccw().tiles(),
                         {{9, 6}, {9, 7}, {9, 8}}));
        CHECK(same_tiles(y_bar3_v.rotate_ccw().tiles(),
                         {{-1, -1}, {0, -1}, {1, -1}}));
        CHECK(same_tiles(square_v.rotate_ccw().tiles(), square.tiles()));
        CHECK(same_tiles(ell_v.rotate_ccw().tiles(),
                         {{0, 1}, {1, 1}, {2, 1}, {2, 2}}));
        CHECK(same_tiles(tee_v.rotate_ccw().tiles(),
                         {{2, 1}, {2, 2}, {2, 3}, {3, 2}}));
    }
    SUBCASE("rotate cw")
    {
        CHECK(same_tiles(dot_v.rotate_cw().tiles(), dot.tiles()));
        CHECK(same_tiles(x_bar2_v.rotate_cw().tiles(),
                         {{8, 6}, {8, 7}}));
        CHECK(same_tiles(y_bar2_v.rotate_cw().tiles(),
                         {{-1, -1}, {0, -1}}));
        CHECK(same_tiles(x_bar3_v.rotate_cw().tiles(),
                         {{9, 6}, {9, 7}, {9, 8}}));
        CHECK(same_tiles(y_bar3_v.rotate_cw().tiles(),
                         {{-1, -1}, {0, -1}, {1, -1}}));
        CHECK(same_tiles(square_v.rotate_cw().tiles(), square.tiles()));
        CHECK(same_tiles(ell_v.rotate_cw().tiles(),
                         {{0, 2}, {1, 2}, {2, 2}, {0, 1}}));
        CHECK(same_tiles(tee_v.rotate_cw().tiles(),
                         {{2, 1}, {2, 2}, {2, 3}, {1, 2}}));
    }
    SUBCASE("all transforms")
    {
        CHECK(same_tiles(ell_v.
                         rotate_cw().
                         flip_y().
                         rotate_ccw().
                         rotate_ccw().
                         translate({3, -4}).
                         flip_x().
                         rotate_ccw().
                         translate({-3, 4}).
                         tiles(), ell.tiles()));
    }
}

TEST_CASE("view toggle")
{
    Figure fig{{0, 0}, {0, 1}, {0, 2}};
    Figure_View view(fig, here, black);
    view.toggle({0, 1});
    view.toggle({1, 1});
    CHECK(same_tiles(view.tiles(), {{0, 0}, {1, 1}, {0, 2}}));
}

TEST_CASE("view toggle transformed")
{
    Figure x_bar3{{0, 0}, {1, 0}, {2, 0}}; //  # # #
    Figure_View view(x_bar3, here, black);
    view.rotate_cw();
    CHECK(same_tiles(view.tiles(), {{1, -1}, {1, 0}, {1, 1}}));

    view.toggle({1, 2});
    view.toggle({1, 3});
    CHECK(same_tiles(view.tiles(), {{1, -1}, {1, 0}, {1, 1}, {1, 2}, {1, 3}}));
    view.rotate_cw();
    CHECK(same_tiles(view.tiles(), {{-1, 1}, {0, 1}, {1, 1}, {2, 1}, {3, 1}}));
    view.toggle({-1, 1});
    view.toggle({0, 1});
    CHECK(same_tiles(view.tiles(), {{1, 1}, {2, 1}, {3, 1}}));

    // view.translate(1, 1);
    // view.toggle({0, 0});  //      # #
    // view.toggle({1, 0});  //    # #
    // view.toggle({3, 1});
    // CHECK(same_tiles(view.tiles(),
    //                  {{0, 0}, {1, 0}, {1, 1}, {2, 1}}));
    // view.flip_x();
    // view.toggle({2, 0});
    // view.toggle({0, 0});  //    # # #
    // view.toggle({2, 1});  //    # #
    // CHECK(same_tiles(view.tiles(),
    //                  {{0, 0}, {0, 1}, {1, 0}, {1, 1}, {2, 1}}));
    // view.rotate_ccw();
    // view.rotate_ccw();    //    # # #
    // view.toggle({2, 1});  //  # # #
    // CHECK(same_tiles(view.tiles(),
    //                  {{-1, 0}, {0, 0}, {1, 0}, {0, 1}, {1, 1}, {2, 1}}));
}