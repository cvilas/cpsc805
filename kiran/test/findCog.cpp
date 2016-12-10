/*
 * This function finds the centre of gravity of a triangle 
 * whose vertices v1, v2 and v3 are provided in CCW order.
 * Put this in objects.hpp
 */
 
#include "data_types.hpp"

// find intercept of lines v1->v2 and v3->v4, where v1 and v2 are 
// absolute coords of end points of line 1 and v3 and v4 are
// absolute coords of end points of line 2.
vector3d_t findLineLineIntercept(const vector3d_t &v1, const vector3d_t &v2, const
vector3d_t &v3, const vector3d_t &v4)
{
 vector3d_t a, b, c, d, i;
 double t, l;
 
 a = v2 - v1;
 b = v4 - v3;
 c = v3 - v1;
 d = cross(a,b);
 l= norm(d);
 t = dot(cross(c, b), d)/(l*l);
 i = v1 + (a * t);
 
 return i;
}

// center of gravity of a triangular patch
vector3d_t findCog(const vector3d_t &v1, const vector3d_t &v2, const vector3d_t &v3)
{
 vector3d_t i;
 vector3d_t v23; // center of v2->v3
 vector3d_t v13; // center of v1->v3
 
 v23 = 0.5 * (v2 + v3);
 v13 = 0.5 * (v1 + v3);
 
 //CoG is the intersection of vectors v1->v23 and v2->v13 
 i = findLineLineIntercept(v1, v23, v2, v13);

 return i;
}


int main()
{
 vector3d_t v1(5,8.66,1), v2(0,0,10), v3(10,0,20), v;
 v = findCog(v1, v2, v3);
 cout << v.x << " " << v.y << " " << v.z << endl;
 return 0;
}
