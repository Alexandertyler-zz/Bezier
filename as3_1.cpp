//
//  as3.cpp
//
//

//#include "as3.h"



#include <iostream>
#include <fstream>
#include <cmath>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#ifdef OSX
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#endif

#include <string.h>
#include <time.h>
#include <math.h>
#include <vector>


using namespace std;

#define PI 3.14159265  // Should be used from mathlib
inline float sqr(float x) { return x*x; }



//****************************************************
// Some Classes
//****************************************************

class Viewport;

class Viewport {
public:
    int w, h; // width and height
};


//****************************************************
// Global Variables
//****************************************************
Viewport	viewport;
bool adaptive, uniform, wireframe;
float rotate2 = 0.0;
bool flat = true;
float step=0, error = 0;
int numPoints = -1;
int colorcount=0, patch=0, quad=0, quadcount=0, numdiv=0;
vector<float> points, p, n, center(3), diameter(3);
vector< vector <vector <float> > > ucurve(4), vcurve(4), patchpoints;
vector< vector<float> > allPoints, unifpoints, unifpoints2;
GLdouble zoomfactor = 1.0;
GLdouble rotatefactor=0.0, rotateUD=0.0, rotateLR = 0.0;
GLfloat translateLR, translateUD = 1.0;
float biggestX=0, biggestY=0, biggestZ=0, smallestX=0, smallestY=0, smallestZ=0, diam=0, endofadappatch = 0;
vector< vector < vector <float> > > adaptivepatch;
vector<vector<vector<float> > > trianglelist;
vector<vector<float> > trianglelist2;




//****************************************************
// Simple init function
//****************************************************
void initScene(){
    
    // Nothing to do here for this simple example.
}


//****************************************************
// reshape viewport if the window is resized           CHANGE TO BE SIZE OF OBJECT RENDERED
//****************************************************
void myReshape(int w, int h) {
    viewport.w = w;
    viewport.h = h;
    glViewport (0,0,viewport.w,viewport.h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    
}


//****************************************************
//
//
//
//****************************************************

//
//vector functions
//
vector<float> operator*(vector<float> vec, float scale) {
    vector<float> retVec;
    retVec.push_back(vec[0] * scale);
    retVec.push_back(vec[1] * scale);
    retVec.push_back(vec[2] * scale);
    return retVec;
}

vector<float> operator*(vector<float> vec1, vector<float> vec2){
    vector<float> retVec;
    retVec.push_back(vec1[0] * vec2[0]);
    retVec.push_back(vec1[1] * vec2[1]);
    retVec.push_back(vec1[2] * vec2[2]);
    return retVec;
}

vector<float> operator+(vector<float> vec1, vector<float> vec2){
    vector<float> retVec;
    retVec.push_back(vec1[0] + vec2[0]);
    retVec.push_back(vec1[1] + vec2[1]);
    retVec.push_back(vec1[2] + vec2[2]);
    return retVec;
}

vector<float> operator-(vector<float> vec1, vector<float> vec2){
    vector<float> retVec;
    retVec.push_back(vec1[0] - vec2[0]);
    retVec.push_back(vec1[1] - vec2[1]);
    retVec.push_back(vec1[2] - vec2[2]);
    return retVec;
}

vector<float> crossprod(vector<float> vec1, vector<float> vec2) {
    vector<float> retVec;
    retVec.push_back((vec1[1] * vec2[2]) - (vec1[2] * vec2[1]));
    retVec.push_back((vec1[2] * vec2[0]) - (vec1[0] * vec2[2]));
    retVec.push_back((vec1[0] * vec2[1]) - (vec1[1] * vec2[0]));
    return retVec;
}






void color() {
    
    //use save points to make quads
    
    numdiv = ((1+0.001)/step);

    if (10%int(10*step)!=0) {
        numdiv++;
    }
    
    GLfloat cyan[] = {0.f, .8f, .8f, 0.f};
    GLfloat lighter[] = {0.0f, .1f, .1f, 0.f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, cyan);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, lighter);

    
    if (adaptive == true) {
        int m = 0;
        //draw triangles for adaptive
        for (int i=0; i<trianglelist.size(); i++) {
            
            vector<float> trinorm(3);
            trinorm = crossprod((trianglelist[i][1]-trianglelist[i][0]), (trianglelist[i][2]-trianglelist[i][0]));
            float length = sqrt(sqr(trinorm[0])+sqr(trinorm[1])+sqr(trinorm[2]));
            trinorm = trinorm*(1/length);
            
            glBegin(GL_TRIANGLES);
            glColor3f(0.0f, 1.0f, 0.0f);
            //glNormal3f(trinorm[0], trinorm[1], trinorm[2]);
            glNormal3f(trianglelist2[m][0], trianglelist2[m][1], trianglelist2[m][2]);
            glVertex3f(trianglelist[i][0][0], trianglelist[i][0][1], trianglelist[i][0][2]);
        
            //glNormal3f(trinorm[0], trinorm[1], trinorm[2]);
            glNormal3f(trianglelist2[m+1][0], trianglelist2[m+1][1], trianglelist2[m+1][2]);
            glVertex3f(trianglelist[i][1][0], trianglelist[i][1][1], trianglelist[i][1][2]);
            
            //glNormal3f(trinorm[0], trinorm[1], trinorm[2]);
            glNormal3f(trianglelist2[m+2][0], trianglelist2[m+2][1], trianglelist2[m+2][2]);
            glVertex3f(trianglelist[i][2][0], trianglelist[i][2][1], trianglelist[i][2][2]);
            
            m+=3;
            
            glEnd();
        }
    
    } else {
        
        
        
        for (int j=0; j<unifpoints.size(); j+=sqr(numdiv+1)) { //j = start of new patch
            for (int k = j; k<j+sqr(numdiv); k+=numdiv+1) { //k = line of patch iterating through
                for (int i=k; i<k+numdiv; i++) { //i = quad in line of subdivisions
                    
                    glColor3f(0.0f, 0.5f, 0.0f);
                    glBegin(GL_POLYGON);
                    
                    /*  //Newell's Method
                     vector<vector<float> > polygonvertices(4);
                     polygonvertices[0] = unifpoints[i];
                     polygonvertices[1] = unifpoints[i+1];
                     polygonvertices[2] = unifpoints[i+numdiv+2];
                     polygonvertices[3] = unifpoints[i+numdiv+1];
                     vector<float> curr(3);
                     vector<float> next(3);
                     for (int n = 0; n<4; n++) {
                     curr = polygonvertices[n];
                     next = polygonvertices[((n+1)%4)];
                     norm2[0]+=((curr[1]-next[1]) * (curr[2]+next[2]));
                     norm2[1]+=((curr[2]-next[2]) * (curr[0]+next[0]));
                     norm2[2]+=((curr[0]-next[0]) * (curr[1]+next[1]));
                     }
                     float length = sqrt(sqr(norm2[0])+sqr(norm2[1])+sqr(norm2[2]));
                     if (length!=0) {
                     norm2[0] = norm2[0]/length;
                     norm2[1] = norm2[1]/length;
                     norm2[2] = norm2[2]/length;
                     } */
                    
                    
                    
                    glNormal3f(unifpoints2[i][0], unifpoints2[i][1], unifpoints2[i][2]);
                    glVertex3f(unifpoints[i][0], unifpoints[i][1], unifpoints[i][2]);
                    
                    glNormal3f(unifpoints2[i+numdiv+1][0], unifpoints2[i+numdiv+1][1], unifpoints2[i+numdiv+1][2]);
                    glVertex3f(unifpoints[i+numdiv+1][0], unifpoints[i+numdiv+1][1], unifpoints[i+numdiv+1][2]);
                    
                    glNormal3f(unifpoints2[i+numdiv+2][0], unifpoints2[i+numdiv+2][1], unifpoints2[i+numdiv+2][2]);
                    glVertex3f(unifpoints[i+numdiv+2][0], unifpoints[i+numdiv+2][1], unifpoints[i+numdiv+2][2]);
                    
                    glNormal3f(unifpoints2[i+1][0], unifpoints2[i+1][1], unifpoints2[i+1][2]);
                    glVertex3f(unifpoints[i+1][0], unifpoints[i+1][1], unifpoints[i+1][2]);
                    
                    glEnd();
                    
                }
            }
        }
    }
    
   /* //shade saved points white from all curves/patches
    glBegin(GL_POINTS);
    glColor3f(1.0f, 1.0f, 1.0f);
    for (vector<int>::size_type i=0; i!=unifpoints.size(); i++) {
        glVertex3f(unifpoints[i][0], unifpoints[i][1], unifpoints[i][2]);
    }
    glEnd();
    */
    
    /*cout<<"\nunifpoints count "<<unifpoints.size();
    cout<<"\nunifpoints2 count "<<unifpoints2.size();
    cout<<"\nnumdiv "<<numdiv;
    cout<<"\nbiggest XYZ "<<biggestX<<" "<<biggestY<<" "<<biggestZ<<"\n";
    cout<<"\nsmallest XYZ "<<smallestX<<" "<<smallestY<<" "<<smallestZ<<"\n";*/
    
    
}





vector<vector<float> > maketriangle (vector<float> vertex1, vector<float> vertex2, vector<float> vertex3) {
    vector<vector<float> > triangle(3);
    triangle[0] = vertex1;
    triangle[1] = vertex2;
    triangle[2] = vertex3;
    return triangle;
}



/*bool distanceCheck(vector<float> p1, vector<float> p2) {
    //need to do abs val
    vector<float> v(3);
    v[0] = abs(p1[0]-p2[0]);
    v[1] = abs(p1[1]-p2[1]);
    v[2] = abs(p1[2]-p2[2]);
    float length = sqrt(sqr(v[0])+sqr(v[1])+sqr(v[2]));
    return (length < step);
    
}*/

vector<float> distanceUV(vector<float> trianglemid, vector<vector<vector<float> > > uvpoints) {
    vector<float> dif(3);
    vector<float> retVec(2);
    float oldlength = 100, currlength=0, uvlength=0;
    int index=0;
    //find closest control point regardless of z value
    for (int i = 0; i<uvpoints.size(); i++) {
        dif[0] = abs(trianglemid[0]-uvpoints[i][0][0]);
        dif[1] = abs(trianglemid[1]-uvpoints[i][0][1]);
        //dif[2] = abs(trianglemid[2]-controlpoints[i][0][2]);
        currlength = sqrt(sqr(dif[0])+sqr(dif[1]));
        
        if (currlength<oldlength) {
            oldlength = currlength;
            index = i;
        }
    }
    
    dif[0] = abs(trianglemid[0]-uvpoints[index][0][0]);
    dif[1] = abs(trianglemid[1]-uvpoints[index][0][1]);
    dif[2] = abs(trianglemid[2]-uvpoints[index][0][2]);
    uvlength = sqrt(sqr(dif[0])+sqr(dif[1])+sqr(dif[2]));
    //if distance between midpoint and uv point < error, keep triangle
    if (uvlength<error) {
        retVec[0] = 1; //true
        retVec[1] = trianglemid[2];
        return retVec;
    //else, need to split and make new midpoint higher;
    } else {
        retVec[0] = 0;
        //store uv's z value to set new midpoint to
        retVec[1] = uvpoints[index][0][2];
        return retVec;
    }
    
}

void closestpointfornormal(vector<float> tri, vector<vector<vector<float> > > uvpoints) {
    vector<float> dif(3);
    float currlength = 100;
    float dist = 0;
    vector<float> uvnorm(1);
    for (int i = 0; i<uvpoints.size(); i++) {
        dif[0] = abs(tri[0]-uvpoints[i][0][0]);
        dif[1] = abs(tri[1]-uvpoints[i][0][1]);
        dif[2] = abs(tri[2]-uvpoints[i][0][2]);
        dist = sqrt(sqr(dif[0])+sqr(dif[1])+sqr(dif[2]));
        if (dist < currlength) {
            currlength = dist;
            uvnorm = uvpoints[i][1];
        }
    }
    trianglelist2.push_back(uvnorm);
}


//changed first arg from vec<vec<vec<float>>> to vec<vec<float>>
void recursiveSplit(vector<vector<float> > tri, int depth, vector<vector<vector<float> > > UVpatch) {
    if (depth == 4) {
        trianglelist.push_back(tri);
		closestpointfornormal(tri[0], UVpatch);
        closestpointfornormal(tri[1], UVpatch);
        closestpointfornormal(tri[2], UVpatch);
        cout<<"\ndepth= "<<depth;
        return;
	}
	int newdepth = depth + 1;
	vector<vector<float> > side1, side2, side3;
    vector<float> v1, v2, v3, m12, m23, m31, b12, b23, b31, m01, m20;
    vector<float> newm01(3), newm12(3), newm20(3);
	int e1 = 0, e2 = 0, e3 = 0;

    
    m01 = ((tri[0] + tri[1])*0.5);
    m12 = ((tri[1] + tri[2])*0.5);
    m20 = ((tri[2] + tri[0])*0.5);


    cout<<"\ndistanceUV(m01,UVpatch)[0] float value= "<<(distanceUV(m01, UVpatch)[0])<<", int value= "<<int(distanceUV(m01, UVpatch)[0]);
    if (int(distanceUV(m01, UVpatch)[0]) == 0) {
        e1 = 100;
        newm01[0] = m01[0];
        newm01[1] = m01[1];
        newm01[2] = distanceUV(m01, UVpatch)[1];
    } else {
        cout<<"\nm01 ok, e1 = "<<e1;
    }
    if (int(distanceUV(m12, UVpatch)[0]) == 0) {
        e2 = 10;
        newm12[0] = m12[0];
        newm12[1] = m12[1];
        newm12[2] = distanceUV(m12, UVpatch)[1];
    } else {
        cout<<"\nm12 ok, e2 = "<<e2;
    }
    if (int(distanceUV(m20, UVpatch)[0]) == 0) {
        e3 = 1;
        newm20[0] = m20[0];
        newm20[1] = m20[1];
        newm20[2] = distanceUV(m20, UVpatch)[1];
    } else {
        cout<<"\nm20 ok, e3 = "<<e3;
    }
    
	/*    SIDE 3
  tri[2]___________tri[1]
     S |          /
     I |        / 2
     D |      / E
     E |    / D
       |  / I
     1 |/ S
     tri[0]
     
     */
	//vector<vector<vector<float> > > tri1, tri2, tri3, tri4;
	//vector<vector<float> > t1s1, t1s2, t1s3, t2s1, t2s2, t2s3, t3s1, t3s2, t3s3, t4s1, t4s2, t4s3;
    vector<vector<float> > newtri1(3), newtri2(3), newtri3(3), newtri4(3), newtri5(3), newtri6(3), newtri7(3), pushtri(3);
    cout<<"\ne1+e2+e3= "<<e3+e2+e1;

	switch(e3 + e2 + e1) {
        case 0: //000
            cout<<"\npassed test, keep orig triangle";
            trianglelist.push_back(tri);
            closestpointfornormal(tri[0], UVpatch);
            closestpointfornormal(tri[1], UVpatch);
            closestpointfornormal(tri[2], UVpatch);
            //use triangle as is
            cout<<"\ndepth= "<<depth;
            break;
        case 1: //001
            //split on 1
            cout<<"\ncase 1";
            newtri1 = maketriangle(newm20, tri[1], tri[2]);
            newtri2 = maketriangle(tri[0], tri[1], newm20);
            newtri3 = maketriangle(newm20, tri[2], tri[0]);
            
            /*newtri1[0] = newm20;
            newtri1[1] = tri[1];
            newtri1[2] = tri[2];
            
            newtri2[0] = tri[0];
            newtri2[1] = tri[1];
            newtri2[2] = newm20;
            
            newtri3[0] = newm20;
            newtri3[1] = tri[2];
            newtri3[2] = tri[0];*/
            
            cout<<"\ndepth= "<<depth;
            
            recursiveSplit(newtri3, newdepth, UVpatch);
            recursiveSplit(newtri1, newdepth, UVpatch);
            recursiveSplit(newtri2, newdepth, UVpatch);
            break;
        case 10: //010
            //split on 2
            cout<<"\ncase 10";
            
            newtri1 = maketriangle(newm12, tri[0], tri[1]);
            newtri2 = maketriangle(newm12, tri[2], tri[0]);
            newtri3 = maketriangle(newm12, tri[2], tri[1]);
            
            /*newtri1[0] = newm12;
            newtri1[1] = tri[0];
            newtri1[2] = tri[1];
            
            newtri2[0] = newm12;
            newtri2[1] = tri[2];
            newtri2[2] = tri[0];
            
            newtri3[0] = newm12;
            newtri3[1] = tri[2];
            newtri3[2] = tri[1];*/
            
            cout<<"\ndepth= "<<depth;

            recursiveSplit(newtri3, newdepth, UVpatch);
            recursiveSplit(newtri1, newdepth, UVpatch);
            recursiveSplit(newtri2, newdepth, UVpatch);
            break;
        case 100:
            //split on 3
            cout<<"\ncase 100";
            
            newtri1 = maketriangle(tri[2], tri[0], newm01);
            newtri2 = maketriangle(tri[1], tri[2], newm01);
            newtri3 = maketriangle(newm01, tri[0], tri[1]);
            
            /*newtri1[0] = tri[2];
            newtri1[1] = tri[0];
            newtri1[2] = newm01;
            
            newtri2[0] = tri[1];
            newtri2[1] = tri[2];
            newtri2[2] = newm01;
            
            newtri3[0] = newm01;
            newtri3[1] = tri[0];
            newtri3[2] = tri[1];*/
            
            cout<<"\ndepth= "<<depth;

            recursiveSplit(newtri3, newdepth, UVpatch);
            recursiveSplit(newtri1, newdepth, UVpatch);
            recursiveSplit(newtri2, newdepth, UVpatch);
            break;
        case 11: //011
            //split on 1 and 2
            cout<<"\ncase 11";
            
            newtri1 = maketriangle(newm12, tri[2], newm20);
            newtri2 = maketriangle(newm12, newm20, tri[0]);
            newtri3 = maketriangle(newm12, tri[0], tri[1]);
            newtri4 = maketriangle(newm12, tri[2], tri[1]);
            newtri5 = maketriangle(newm20, tri[2], tri[0]);
            
            /*newtri1[0] = newm12;
            newtri1[1] = tri[2];
            newtri1[2] = newm20;
            
            newtri2[0] = newm12;
            newtri2[1] = newm20;
            newtri2[2] = tri[0];
            
            newtri3[0] = newm12;
            newtri3[1] = tri[0];
            newtri3[2] = tri[1];
            
            newtri4[0] = newm12;
            newtri4[1] = tri[2];
            newtri4[2] = tri[1];
            
            newtri5[0] = newm20;
            newtri5[1] = tri[2];
            newtri5[2] = tri[0];*/
            
            cout<<"\ndepth= "<<depth;

            recursiveSplit(newtri4, newdepth, UVpatch);
            recursiveSplit(newtri5, newdepth, UVpatch);
            recursiveSplit(newtri1, newdepth, UVpatch);
            recursiveSplit(newtri2, newdepth, UVpatch);
            recursiveSplit(newtri3, newdepth, UVpatch);
            break;
        case 110:
            //split on 2 and 3
            cout<<"\ncase 110";
            
            newtri1 = maketriangle(tri[2], tri[0], newm01);
            newtri2 = maketriangle(newm12, tri[2], newm01);
            newtri3 = maketriangle(tri[1], newm12, newm01);
            newtri4 = maketriangle(newm01, tri[0], tri[1]);
            newtri5 = maketriangle(newm12, tri[2], tri[1]);
            
            /*newtri1[0] = tri[2];
            newtri1[1] = tri[0];
            newtri1[2] = newm01;
            
            newtri2[0] = newm12;
            newtri2[1] = tri[2];
            newtri2[2] = newm01;
            
            newtri3[0] = tri[1];
            newtri3[1] = newm12;
            newtri3[2] = newm01;
            
            newtri4[0] = newm01;
            newtri4[1] = tri[0];
            newtri4[2] = tri[1];

            newtri5[0] = newm12;
            newtri5[1] = tri[2];
            newtri5[2] = tri[1];*/
            
            cout<<"\ndepth= "<<depth;

            recursiveSplit(newtri4, newdepth, UVpatch);
            recursiveSplit(newtri5, newdepth, UVpatch);
            recursiveSplit(newtri1, newdepth, UVpatch);
            recursiveSplit(newtri2, newdepth, UVpatch);
            recursiveSplit(newtri3, newdepth, UVpatch);
            break;
        case 101:
            //split on 3 and 1
            cout<<"\ncase 101";
            
            newtri1 = maketriangle(newm01, newm20, tri[0]);
            newtri2 = maketriangle(tri[2], newm20, newm01);
            newtri3 = maketriangle(tri[1], tri[2], newm01);
            newtri4 = maketriangle(newm01, tri[0], tri[1]);
            newtri5 = maketriangle(newm20, tri[2], tri[0]);
            
            
            /*newtri1[0] = newm01;
            newtri1[1] = newm20;
            newtri1[2] = tri[0];
            
            newtri2[0] = tri[2];
            newtri2[1] = newm20;
            newtri2[2] = newm01;
            
            newtri3[0] = tri[1];
            newtri3[1] = tri[2];
            newtri3[2] = newm01;
            
            newtri4[0] = newm01;
            newtri4[1] = tri[0];
            newtri4[2] = tri[1];
            
            newtri5[0] = newm20;
            newtri5[1] = tri[2];
            newtri5[2] = tri[0];*/
            
            cout<<"\ndepth= "<<depth;

            recursiveSplit(newtri4, newdepth, UVpatch);
            recursiveSplit(newtri5, newdepth, UVpatch);
            recursiveSplit(newtri1, newdepth, UVpatch);
            recursiveSplit(newtri2, newdepth, UVpatch);
            recursiveSplit(newtri3, newdepth, UVpatch);
            break;
        case 111:
            //split on 1, 2, and 3
            cout<<"\ncase 111";
            
            newtri1 = maketriangle(newm12, newm20, newm01);
            newtri2 = maketriangle(newm01, newm20, tri[0]);
            newtri3 = maketriangle(newm12, tri[2], newm20);
            newtri4 = maketriangle(tri[1], newm12, newm01);
            newtri5 = maketriangle(newm01, tri[0], tri[1]);
            newtri6 = maketriangle(newm12, tri[2], tri[1]);
            newtri7 = maketriangle(newm20, tri[2], tri[0]);
            
            
            /*newtri1[0] = newm12;
            newtri1[1] = newm20;
            newtri1[2] = newm01;
            
            newtri2[0] = newm01;
            newtri2[1] = newm20;
            newtri2[2] = tri[0];
            
            newtri3[0] = newm12;
            newtri3[1] = tri[2];
            newtri3[2] = newm20;
            
            newtri4[0] = tri[1];
            newtri4[1] = newm12;
            newtri4[2] = newm01;
            
            newtri5[0] = newm01;
            newtri5[1] = tri[0];
            newtri5[2] = tri[1];
            
            newtri6[0] = newm12;
            newtri6[1] = tri[2];
            newtri6[2] = tri[1];
            
            newtri7[0] = newm20;
            newtri7[1] = tri[2];
            newtri7[2] = tri[0];*/
            
            cout<<"\ndepth= "<<depth;
            
            recursiveSplit(newtri5, newdepth, UVpatch);
            recursiveSplit(newtri6, newdepth, UVpatch);
            recursiveSplit(newtri7, newdepth, UVpatch);
            recursiveSplit(newtri1, newdepth, UVpatch);
            recursiveSplit(newtri2, newdepth, UVpatch);
            recursiveSplit(newtri3, newdepth, UVpatch);
            recursiveSplit(newtri4, newdepth, UVpatch);
            break;
    }
	

    
}

void adaptiveTess(vector<vector<vector<float> > > patch, vector<vector<vector<float> > > UVpatch) {
    vector<vector<float> > tria1(3), tria2(3);
    
    tria1[0] = patch[0][3];
    tria1[1] = patch[0][0];
    tria1[2] = patch[3][0];
    
    tria2[0] = patch[0][3];
    tria2[1] = patch[3][0];
    tria2[2] = patch[3][3];
    
    glBegin(GL_LINES);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(tria1[1][0], tria1[1][1], tria1[1][2]);
    glVertex3f(tria1[2][0], tria1[2][1], tria1[2][2]);
    
    glVertex3f(tria1[2][0], tria1[2][1], tria1[2][2]);
    glVertex3f(tria2[2][0], tria2[2][1], tria2[2][2]);
    
    glVertex3f(tria2[2][0], tria2[2][1], tria2[2][2]);
    glVertex3f(tria2[0][0], tria2[0][1], tria2[0][2]);
    
    glVertex3f(tria2[0][0], tria2[0][1], tria2[0][2]);
    glVertex3f(tria1[1][0], tria1[1][1], tria1[1][2]);
    
    glEnd();
    
    
    /*
    vector<vector<vector<float> > > tri1(3);
    vector<vector<vector<float> > > tri2(3);
    
    //counterclockwise
	//TRIANGLE 1 SIDES
    tri1[0][0] = patch[0][0];
    tri1[0][1] = patch[1][0];
    tri1[0][2] = patch[2][0];
	tri1[0][3] = patch[3][0];
    
	tri1[1][0] = patch[3][0];
    tri1[1][1] = patch[2][1];
    tri1[1][2] = patch[1][2];
	tri1[1][3] = patch[0][3];
    
	tri1[2][0] = patch[0][3];
    tri1[2][1] = patch[0][2];
    tri1[2][2] = patch[0][1];
	tri1[2][3] = patch[0][0];
    
	//TRIANGLE 2 SIDES
    tri2[0][0] = patch[3][0];
    tri2[0][1] = patch[3][1];
    tri2[0][2] = patch[3][2];
	tri2[0][3] = patch[3][3];
    
	tri2[1][0] = patch[3][3];
    tri2[1][1] = patch[2][3];
    tri2[1][2] = patch[1][3];
	tri2[1][3] = patch[0][3];
    
	tri2[2][0] = patch[0][3];
    tri2[2][1] = patch[1][2];
    tri2[2][2] = patch[2][1];
	tri2[2][3] = patch[3][0];
     */
    
	//recursive call for each triangle
    recursiveSplit(tria1, 1, UVpatch);
    recursiveSplit(tria2, 1, UVpatch);
	cout<<"\nadaptivetess call";
    
}













//
//functions referenced from lecture 12 slides
//
vector< vector<float> > bezcurveinterp(vector<vector <float> > vec, float u) {
    //cout << "\nu: "<<u;
    vector< vector<float> > pointderiv(2);

    vector<float> a = vec[0] * (1.0f-u) + vec[1] * u;
    //cout<<"\na: "<<a[0]<<" "<<a[1]<<" "<<a[2];
    vector<float> b = vec[1] * (1.0f-u) + vec[2] * u;
    vector<float> c = vec[2] * (1.0f-u) + vec[3] * u;
    
    
    vector<float> d = a * (1.0f-u) + b * u;
    vector<float> e = b * (1.0f-u) + c * u;
    
    p = d * (1.0f-u) + e * u;
    
    vector<float> dPdu = (e-d) * 3;
    
    pointderiv[0] = p;
    pointderiv[1] = dPdu;
    
    //used for setting up appropriate window coordinates in myDisplay
    if (p[0]>biggestX) {
        biggestX = p[0];
    }
    if (p[1]>biggestY) {
        biggestY = p[1];
    }
    if (p[2]>biggestZ) {
        biggestZ = p[2];
    }
    if (p[0]<smallestX) {
        smallestX = p[0];
    }
    if (p[1]<smallestY) {
        smallestY = p[1];
    }
    if (p[2]<smallestZ) {
        smallestZ = p[2];
    }
    
    return pointderiv;
}


vector<vector<float> > bezpatchinterp(vector<vector<vector<float> > > vec, float u, float v) {
    vector<vector<float> > curve(4);
    for (int i=0; i<4; i++) {
        curve[0] = vec[i][0];
        curve[1] = vec[i][1];
        curve[2] = vec[i][2];
        curve[3] = vec[i][3];
        ucurve[i] = bezcurveinterp(curve, v);
    }
    for (int i=0; i<4; i++) {
        curve[0] = vec[0][i];
        curve[1] = vec[1][i];
        curve[2] = vec[2][i];
        curve[3] = vec[3][i];
        vcurve[i] = bezcurveinterp(curve, u);
    }
    vector<vector<float> > vcurvepoints(4);
    vcurvepoints[0] = vcurve[0][0];
    vcurvepoints[1] = vcurve[1][0];
    vcurvepoints[2] = vcurve[2][0];
    vcurvepoints[3] = vcurve[3][0];
    vector<vector<float> > vcurvePointDeriv(2);
    vcurvePointDeriv = bezcurveinterp(vcurvepoints, v);
    
    vector<vector<float> > ucurvepoints(4);
    ucurvepoints[0] = ucurve[0][0];
    ucurvepoints[1] = ucurve[1][0];
    ucurvepoints[2] = ucurve[2][0];
    ucurvepoints[3] = ucurve[3][0];
    vector<vector<float> > ucurvePointDeriv(2);
    ucurvePointDeriv = bezcurveinterp(ucurvepoints, u);
    
    vector<float> norm(3);
    norm = crossprod(ucurvePointDeriv[1], vcurvePointDeriv[1]*-1);
    
    float length = sqrt(sqr(norm[0])+sqr(norm[1])+sqr(norm[2]));
    norm[0] = norm[0]/length;
    norm[1] = norm[1]/length;
    norm[2] = norm[2]/length;
    
    vector<vector<float> > retVec;
    retVec.push_back(ucurvePointDeriv[0]);
    retVec.push_back(norm);

    return retVec;
}

void savesurfpointnorm(vector<float> vec, vector<float> vec2) {
    
    if (adaptive == true) {
        vector<vector<float> > adaptivepointnorm(2);
        adaptivepointnorm[0] = vec;
        adaptivepointnorm[1] = vec2;
        adaptivepatch.push_back(adaptivepointnorm);
        endofadappatch++;
    } else {
        unifpoints.push_back(vec);
        unifpoints2.push_back(vec2);
    }
}


void subdividepatch(vector<vector<vector<float> > > patch) {
    numdiv = ((1+0.001)/step);
    //cout<<"\nNUMDIV = "<<numdiv;
    //cout<<"\n10 mod step= "<<10%int(10*step);
    float u, v;
    
    
    //ADDED
   if (10%int(10*step)!=0) {
       numdiv++;
       cout<<"\nNUMDIV = "<<numdiv;
       cout<<"\n10 mod step= "<<10%int(10*step);
   }
    //
    
    
    for (int iu = 0; iu<=numdiv; iu++) {
        //ADDED
        if (10%int(10*step)!=0 && iu==numdiv) {
            u = 1.0;
        //
        } else {
            u = iu * step;
        }
        cout<<"\nu: "<<u;
        for (int iv = 0; iv<=numdiv; iv++) {
            //ADDED
            if (10%int(10*step)!=0 && iv==numdiv) {
                v = 1.0;
            //
            } else {
                v = iv * step;
            }
            p = bezpatchinterp(patch, u, v)[0];
            n = bezpatchinterp(patch, u, v)[1];
            savesurfpointnorm(p, n);
            
            if (adaptive==true && endofadappatch == sqr(numdiv+1)) {
                adaptiveTess(patch, adaptivepatch);
                endofadappatch=0;
                adaptivepatch.clear();
            }
            //cout<<"\nsubdivide p and n "<<p[0]<<" "<<p[1]<<" "<<p[2]<<" n "<<n[0]<<" "<<n[1]<<" "<<n[2];
            cout<<"\nv: "<<v;
        }
        cout<<"\nfinished v loop at unifpoints item"<<unifpoints.size()-1;

    }
    cout<<"\nfinished u and v loops (whole patch) at unifpoints item"<<unifpoints.size()-1;
}
























//
//get patches from inputfile
//
void patchesfromfile (string s) {
    int xyz = 0;
    string point = "";
    float b;
    if (s=="") {
        goto skip;
    }
    //cout<<"\nstring \""<<s<<"\"\nsize "<<s.size();
    for (int i=0; i<=s.size();) {
        if (s[i]!=' ') {
            point+=s[i];
            if (i==s.size()) {
                points.push_back(atof(point.c_str()));
                numPoints++;
            }
            i++;
        } else if (s[i]==' ') {
            if (point=="") {
                while (s[i]==' ') {
                    i++;
                    if (i==s.size()) {
                        goto done;
                    }
                }
            } else {
                points.push_back(atof(point.c_str()));
                numPoints++;
                point = "";
            }
            
        }
    }
done:
    cout<<"\nvector contents ";
    for (int i=0; i<points.size(); i++) {
        cout<<" \""<<points[i]<<"\" ";
    }
    if (points.size()==12) {
        allPoints.clear();
        for (int i=0; i<12;) {
            vector<float> xyz(3);
            xyz[0] = points[i];
            xyz[1] = points[i+1];
            xyz[2] = points[i+2];
            allPoints.push_back(xyz);
            i+=3;
        }
    }
    
    patchpoints.push_back(allPoints);
    patch++;
    
    if (patch == 4) {
        
        
        subdividepatch(patchpoints);
        /*cout<<"\n\nFULL PATCH patchpoints\n"<<patchpoints[0][0][0]<<" "<<patchpoints[0][0][1]<<" "<<patchpoints[0][0][2]<<"\nsecond"<<patchpoints[0][1][0]<<" "<<patchpoints[0][1][1]<<" "<<patchpoints[0][1][2]<<"\nlast item"<<patchpoints[3][3][0]<<" "<<patchpoints[3][3][1]<<" "<<patchpoints[3][3][2]<<"\n";*/
        patchpoints.clear();
        patch = 0;
    }
    
    /*cout<<"\nallPoints[0]: \""<<allPoints[0][0]<<","<<allPoints[0][1]<<","<<allPoints[0][2]<<"\"";
    cout<<"\nallPoints[1]: \""<<allPoints[1][0]<<","<<allPoints[1][1]<<","<<allPoints[1][2]<<"\"";
    cout<<"\nallPoints[2]: \""<<allPoints[2][0]<<","<<allPoints[2][1]<<","<<allPoints[2][2]<<"\"";
    cout<<"\nallPoints[3]: \""<<allPoints[3][0]<<","<<allPoints[3][1]<<","<<allPoints[3][2]<<"\"";*/
    
    points.clear();
    cout <<"\nend of patches call";
skip: ;

}






//****************************************************
// function that does the actual drawing of stuff
//***************************************************
void myDisplay() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// clear the color buffer

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    center[0]=(biggestX+smallestX)/2;
    center[1]=(biggestY+smallestY)/2;
    center[2]=(biggestZ+smallestZ)/2;
    diameter[0]=abs(biggestX-smallestX);
    diameter[1]=abs(biggestY-smallestY);
    diameter[2]=abs(biggestZ-smallestZ);
    diam = max(max(diameter[0], diameter[1]), diameter[2]);
    
    glOrtho(center[0]-diam, center[0]+diam, center[1]-diam, center[1]+diam, smallestZ-1, biggestZ+diam);
    //works glOrtho(smallestX-4, biggestX+4, smallestY-4, biggestY+4, -4, 4);
 //   glOrtho(smallestX-1, biggestX+1, smallestY-1, biggestY+1, -1, 1);
    
    
    //glOrtho(smallestX-4*zoomfactor, biggestX+4*zoomfactor, smallestY-4*zoomfactor, biggestY+4*zoomfactor, -6, 6);
    //gluPerspective(65.0, 8.0, 2.0, 15.0);
    //glFrustum(-2, 2, -2, 2, 1, 5);
    glScalef(zoomfactor, zoomfactor, 1.0);


    
    glMatrixMode(GL_MODELVIEW);	        // indicate we are specifying camera transformations
    glLoadIdentity();

    gluLookAt(center[0]+0.1, center[1]+diam, center[2], //+0.1 for objects like test to appear /* look from camera XYZ */
              center[0], center[1], center[2], /* look at the center of the object */
              0, 0, 1); /* positive Z up vector */
    glEnable (GL_DEPTH_TEST);

    glTranslatef(1*translateLR, 1*translateLR, 1*translateUD);
    glRotatef(15*rotateLR, 0, 0, 1);
    glRotatef(15*rotateUD, 1, 0, 0);


    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);
//    GLfloat lightpos[] = {0.0, 0., 4., 0.};
    glLightf(GL_LIGHT0, GL_SPECULAR, (diam, 0, diam));
    glLightf(GL_LIGHT1, GL_DIFFUSE, (diam, 0, diam));
    glLightf(GL_LIGHT2, GL_AMBIENT, (diam, 0, diam));
    

    if (flat == true) {
        glShadeModel(GL_FLAT);
    } else {
        glShadeModel(GL_SMOOTH);
    }
    if (wireframe == true) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    color();
    glFlush();
    glutSwapBuffers();
}

//Used to toggle adjustments to the viewing window
void idleInput (unsigned char key, int xmouse, int ymouse) {

    switch (key)
    
    {
        case 's': //toggle flat/smooth shading
            if (flat == true) {
                flat = false;
            } else {
                flat = true;
            }
            myDisplay();
            break;
            
        case 'w': //toggle filled/wireframe mode
            if (wireframe == true) {
                wireframe = false;
            } else {
                wireframe = true;
            }
            myDisplay();
            break;

        case '+': //zoom in
            zoomfactor+=0.25;
            myDisplay();
            break;
        case '=': //zoom in (still + key)
            zoomfactor+=0.25;
            myDisplay();
            break;

        case '-': //zoom out
            zoomfactor-=0.25;
            myDisplay();
            break;
            
        default:
            break;
    }
}

void arrowkey (int key, int x, int y) {
    int mod = glutGetModifiers();
    switch(key)
    {
        //rotate/translate adjustments for arrow keys and shift+arrow keys
        case 100:
            if (mod==GLUT_ACTIVE_SHIFT) {
                translateLR += 0.25;
            } else {
                rotateLR-=1;
            }
            myDisplay();
            break;
        case 101:
            if (mod==GLUT_ACTIVE_SHIFT) {
                translateUD += 0.25;
            } else {
                rotateUD+=1;
            }
            myDisplay();
            break;
        case 102:
            if (mod==GLUT_ACTIVE_SHIFT) {
                translateLR -= 0.25;
            } else {
                rotateLR+=1;
            }
            myDisplay();
            break;
        case 103:
            if (mod==GLUT_ACTIVE_SHIFT) {
                translateUD -= 0.25;
            } else {
                rotateUD-=1;
            }
            myDisplay();
            break;
        default:
            break;
    }
}








//****************************************************
// the usual stuff, nothing exciting here
//****************************************************
int main(int argc, char *argv[]) {
    //This initializes glut
	glutInit(&argc, argv);
    
    //glClear(GL_COLOR_BUFFER_BIT);
    //This tells glut to use a double-buffered window with red, green, and blue channels
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    
    // Initalize theviewport size
    viewport.w = 400;
    viewport.h = 400;
    
    //The size and position of the window
    glutInitWindowSize(viewport.w, viewport.h);
    glutInitWindowPosition(800,0);
    glutCreateWindow("as3");
    
    initScene();

    
    //SET INPUTFILE TO GET PATCHES FROM HERE
    ifstream inputfile(argv[1]);
    step = atof(argv[2]);
    if (argc == 3) {
        uniform = true;
    } else if (strcmp(argv[3], "-a") == 0) {
        adaptive = true;
        error = step;
        step = 0.1;
    }

    cout << "\nFile: " << inputfile;
    cout << "\nParameter: " << step;
    cout << "\nAdaptive: " << adaptive;
    cout << "\nUniform: " << uniform;
    cout << "\n";
    std::string s;

    string numPatches;
    getline(inputfile, numPatches, '\n');
    while (getline(inputfile, s, '\n')) {
        patchesfromfile(s);
    }
    
    
    glutDisplayFunc(myDisplay);				// function to run when its time to draw something
    //glFlush();
    //glutSwapBuffers();					// swap buffers (we earlier set double buffer)
    glutReshapeFunc(myReshape);				// function to run when the window gets resized
    glutSpecialFunc(arrowkey);
    glutKeyboardFunc(idleInput);

    glutMainLoop();							// infinite loop that will keep drawing and resizing

    
    return 0;
}

