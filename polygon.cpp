#include "polygon.h"
#include <functional>

polygon::polygon()
{
}


polygon::~polygon()
{
}

bool polygon::operator==(const polygon &another_polygon) const{
	bool eq = true;
	for (unsigned int p = 0; p < points.size(); p++){
		eq = (eq && (points[p] == another_polygon.points[p]));
	}

	return eq;
}

