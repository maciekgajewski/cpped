#pragma once

#include <ostream>

namespace nct {

struct position
{
	int y;
	int x;

	bool operator==(const position& o) const
	{
		return x == o.x && y == o.y;
	}
	bool operator != (const position& o) const { return !operator==(o); }

	position operator+(const position& o) const
	{
		return position{y + o.y, x + o.x};
	}
};

inline std::ostream& operator<<(std::ostream& o, const position& pos)
{
	return o << "(" << pos.x << ", " << pos.y << ")";
}

struct size
{
	int h;
	int w;

	bool operator==(const size& o) const
	{
		return h == o.h && w == o.w;
	}
	bool operator != (const size& o) const { return !operator==(o); }
};


inline std::ostream& operator<<(std::ostream& o, const size& sz)
{
	return o << "(" << sz.w << "x" << sz.h << ")";
}

}
