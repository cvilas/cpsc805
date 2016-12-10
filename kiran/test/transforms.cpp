#include "data_types.hpp"
#include <math.h>

int main()
{
 vector3d_t v;
 transform_t t1,t2;
 
 v = vector3d_t(1,0,0);
 
 t1 = rotate(0,0,-M_PI/2);
 t2 = translate(0,1,0);
 v = t1 * (t2 * v);
 
 cout << v.x << " " << v.y << " " << v.z << endl;
 return 0;
}
