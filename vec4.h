#pragma once
class vec4
{
public:
	vec4();
	vec4(double _x, double _y, double _z, double _p);
	~vec4();
	double x;
	double y;
	double z;
	double p;
	double& operator[](int indx);
};

bool operator==(vec4 l, vec4 r);
vec4 operator+(vec4 lhs, vec4 rhs);