#include "model.h"


model::model()
{
	obj_coord_trans[0].x = 1;

	obj_coord_trans[1].y = 1;

	obj_coord_trans[2].z = 1;
	
	obj_coord_trans[3].p = 1;

	inv_obj_coord_trans[0].x = 1;

	inv_obj_coord_trans[1].y = 1;

	inv_obj_coord_trans[2].z = 1;

	inv_obj_coord_trans[3].p = 1;

	color = RGB(255, 255, 255);
}


model::~model()
{
}
