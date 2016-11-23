#include "line.h"


line::line()
{
}

line::line(vec4 a, vec4 b)
{
	p_a = a;
	p_b = b;
}

line::~line()
{
}

bool operator==(line l, line r){
	if (l.p_a == r.p_a && l.p_b == r.p_b)
		return true;
	else
		return false;
}
