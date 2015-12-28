#pragma once

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

}
