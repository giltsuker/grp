#pragma once
#include "vec4.h"
class line
{
public:
	line();
	line(vec4 a, vec4 b);
	~line();
	vec4 p_a;
	vec4 p_b;
};

bool operator==(line l, line r);

