#include "figure.hh"

void check(Tile_List const& tiles, Tile_List& found, Point<int> const& p)
{
    if (!tiles.contains(p) || found.contains(p))
        return;
    found.insert(p);
    // Use a flood-fill algorithm to find adjacent tiles.
    check(tiles, found, {p.x+1, p.y});
    check(tiles, found, {p.x-1, p.y});
    check(tiles, found, {p.x, p.y+1});
    check(tiles, found, {p.x, p.y-1});
}

bool connected(Tile_List const& tiles)
{
    Tile_List found;
    check(tiles, found, *tiles.begin());
    return found.size() == tiles.size();
}

Figure::Figure()
{
}

Figure::Figure(std::initializer_list<Point<int>> ps)
{
    for (auto const& p : ps)
        m_tiles.insert(p);
    if (!m_tiles.empty())
        m_origin = *ps.begin();
}

bool Figure::is_contiguous() const
{
    return m_contiguous;
}

Tile_List const& Figure::tiles() const
{
    return m_tiles;
}

Point<double> Figure::cm() const
{
    Point<double> cm{0.0, 0.0};
    if (m_tiles.empty())
        return cm;
    for (auto const& tile : m_tiles)
        cm += tile;
    return cm /= m_tiles.size();
}

void Figure::toggle(Point<int> const& p)
{
    if (m_tiles.empty())
        m_origin = p;
    if (m_tiles.contains(p))
        m_tiles.erase(p);
    else
        m_tiles.insert(p);

    m_contiguous = connected(m_tiles);
}

void Figure::clear()
{
    m_tiles.clear();
}
