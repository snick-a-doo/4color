#ifndef FOUR_COLOR_LIB4COLOR_FIGURE_VIEW_HH_INCLUDED
#define FOUR_COLOR_LIB4COLOR_FIGURE_VIEW_HH_INCLUDED

#include <iostream>
#include <tuple>
#include <vector>

class Figure;

using Color = std::tuple<double, double, double>;
using VTiles = std::vector<Point<int>>;

constexpr Color black{0.0, 0.0, 0.0};
constexpr Color red{1.0, 0.0, 0.25};
constexpr Color yellow{1.0, 1.0, 0.0};
constexpr Color green{0.15, 0.75, 0.15};
constexpr Color blue{0.0, 0.0, 1.0};

struct Matrix
{
    int xx{1}, xy{0};
    int yx{0}, yy{1};
};

class Figure_View
{
public:
    Figure_View(Figure& fig, Point<int> dr, Color const& color);

    VTiles tiles() const;
    Color color() const;

    Figure_View& toggle(Point<int> p);
    Figure_View& translate(Point<int> dr);
    Figure_View& flip_x();
    Figure_View& flip_y();
    Figure_View& rotate_ccw();
    Figure_View& rotate_cw();
    Figure_View& reset();

private:
    Figure& m_figure;
    const Point<double> m_init_dr;
    Point<double> m_dr;
    Matrix m_transform;
    Color m_color;
};

std::ostream& operator<<(std::ostream& os, Figure_View const& f);

#endif // FOUR_COLOR_LIB4COLOR_FIGURE_VIEW_HH_INCLUDED
