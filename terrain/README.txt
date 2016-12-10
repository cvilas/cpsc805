terraview - Procedural mountain terrain modeller
================================================

Author: Vilas Kumar Chitrakaran, April 16, 2004
        (cvilas@ces.clemson.edu)

To compile and run:
-------------------
	make
	./terraview
	
GUI controls:
-------------
	m: enable pointer based navigation
	x; disable pointer based navigation
	r: return to default view position
	w: toggle wireframe view of terrain 
	f: toggle flat shading of polygons
	q: quit

comments:
---------
Try the following for some good looking mountains:
./terraview -d 8 -s 900 -r 0.5
./terraview -d 8 -s 566 -r 0.5

Project webpage:
----------------
http://ece.clemson.edu/crb/students/vilas/projects/cs805/index.htm


Comments:
---------
Polygon subdivision is based on the diamond-square algorithm. An OpenGL 
based renderer that I built for my CpSc611 (Virtual Reality Systems) class
was used to render the terrain. The sky and sea textures were generated 
using Terragen
