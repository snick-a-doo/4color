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

#ifndef FOUR_COLOR_LIB4COLOR_POINT_HH_INCLUDED
#define FOUR_COLOR_LIB4COLOR_POINT_HH_INCLUDED

#include <iostream>

/// A two-dimensional point
template<typename T> struct Point
{
    T x{0};
    T y{0};
    auto operator <=>(Point<T> const& p) const = default;
};

/// Math operators
/// @{

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

template<typename T, typename U> Point<T> operator *(U k, Point<T> const& p)
{
    return p*k;
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
/// @}

template<typename T>
std::ostream& operator<<(std::ostream& os, const Point<T>& p)
{
    return os << '(' << p.x << ", " << p.y << ')';
}

#endif // FOUR_COLOR_LIB4COLOR_POINT_HH_INCLUDED
