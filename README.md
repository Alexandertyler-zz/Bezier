Bezier
======

Cs184 Bezier Curves Asignment

Resources:
  Wikipedia bezier curves

Outline:
  Parse control points from a file
  Set the control points in some object in 3d space:
    Locate places in space and draw mesh between points, flat and mesh shading in opengl
  Flat and smooth shading, filled and wire frame
  
  Drawing a point:
    Colors it red, makes a small circle around it, iterates by .1
    glcolor3f(1, 0, 0);
    glBegin(GL_LINELOOP);
    float u, v;
    for(float t=0; t < 2*PI; t+=.1)
    {
      //activeRadius = .05
      u = cos(t)*ActiveRadius + x;
      v = sin(t)*activeRadius + y;
      glVertex3f(u, v, 0);
    }
    glEnd();
    
    glColor3f(1, 1, 1);
    glBegin(Quad_)
    glvertex2f(x-.1, y-,1);
    etc
    draws a small sphere around the point;
    
