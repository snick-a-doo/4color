#ifndef FOUR_COLOR_LIB4COLOR_FIGURE_HH_INCLUDED
#define FOUR_COLOR_LIB4COLOR_FIGURE_HH_INCLUDED

#include <iostream>
#include <set>

template<typename T> struct Point
{
    T x{0};
    T y{0};
    auto operator <=>(Point<T> const& p) const = default;
};

template<typename T, typename U>
Point<T> operator +(Point<T> const& p1, Point<U> const& p2)
{
    return {p1.x + p2.x, p1.y + p2.y};
}
template<typename T, typename U> Point<T>& operator +=(Point<T>& p1, Point<U> const& p2)
{
    return p1 = p1 + p2;
}

template<typename T, typename U> Point<T>
operator -(Point<T> const& p1, Point<U> const& p2)
{
    return {p1.x - p2.x, p1.y - p2.y};
}
template<typename T, typename U> Point<T>& operator -=(Point<T>& p1, Point<U> const& p2)
{
    return p1 = p1 - p2;
}

template<typename T, typename U> Point<T> operator *(Point<T> const& p, U k)
{
    return {p.x*k, p.y*k};
}
template<typename T, typename U> Point<T>& operator *=(Point<T>& p, U k)
{
    return p = p*k;
}

template<typename T, typename U> Point<T> operator /(Point<T> const& p, U k)
{
    return {p.x/k, p.y/k};
}
template<typename T, typename U> Point<T>& operator /=(Point<T>& p, U k)
{
    return p = p/k;
}
template<typename T> Point<T> operator -(Point<T>& p)
{
    return p * -1;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const Point<T>& p)
{
    return os << '(' << p.x << ", " << p.y << ')';
}

using Tile_List = std::set<Point<int>>;

class Figure
{
public:
    Figure();
    Figure(std::initializer_list<Point<int>> ps);

    bool is_contiguous() const;
    Tile_List const& tiles() const;
    Point<double> cm() const;
    Point<int> origin() const { return m_origin; }

    void toggle(Point<int> const& p);
    void clear();

private:
    Tile_List m_tiles;
    bool m_contiguous{true};
    Point<double> m_cm{0.0, 0.0};
    Point<int> m_origin{0, 0};
};

#endif // FOUR_COLOR_LIB4COLOR_FIGURE_HH_INCLUDED
