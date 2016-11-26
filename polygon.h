#pragma once
#include <vector>
#include "vec4.h"

class polygon
{
public:
	polygon();
	~polygon();
	std::vector<vec4> points;
	
	bool operator==(const polygon &another_polygon) const;
};

