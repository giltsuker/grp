#pragma once
#include "polygon.h"
#include <vector>
#include "vec4.h"
#include "mat4.h"

class model
{
public:
	model();
	~model();
	std::vector <polygon>  polygons;

	mat4 view_space_trans;
	vec4 min_vec;
	vec4 max_vec;
};

