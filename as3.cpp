//
//  as3.cpp
//
//

#include "as3.h"



#include <iostream>
#include <fstream>
#include <cmath>
#include <sstream>
#include <stdio.h>
//#include <ncurses.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include <GL/glut.h>
#include <GL/glu.h>

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
bool flat = true;
float step = 0;
int numPoints = -1;
int colorcount, patch = 0;
vector<float> points, p, n;
vector< vector <vector <float> > > ucurve(4), vcurve(4), patchpoints;
vector< vector<float> > allPoints, unifpoints, unifpoints2;




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


void color() {
    //shade saved points white from all curves/patches
    glBegin(GL_POINTS);
    glColor3f(1.0f, 1.0f, 1.0f);
	for (vector<int>::size_type i = 0; i != unifpoints.size(); i++) {
			glVertex3f(unifpoints[i][0], unifpoints[i][1], unifpoints[i][2]);
    }
    glEnd();
    
    /*
    glBegin(GL_POINTS);
    glColor3f(1.0f, 0.0f, 0.0f);
    for (int i=0; i<unifpoints2.size(); i++) {
        glVertex3f(unifpoints2[i][0], unifpoints2[i][1], unifpoints2[i][2]);
    }
    glEnd();*/
    
    //use save points to make quads - need to fix, not right ordering when iterate through saved points as they are stored now
    glBegin(GL_QUADS);
    glColor3f(0.5f, 0.0f, 0.0f);
	for (vector<int>::size_type i = 0; i != unifpoints.size(); i++) {
        glVertex3f(unifpoints[i][0], unifpoints[i][1], unifpoints[i][2]);
    }
    glEnd();
}


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



//
//functions referenced from lecture 12 slides
//
vector< vector<float> > bezcurveinterp(vector<vector <float> > vec, float u) {
    //cout << "\nu: "<<u;
    vector< vector<float> > pointderiv;

    vector<float> a = vec[0] * (1.0f-u) + vec[1] * u;
    //cout<<"\na: "<<a[0]<<" "<<a[1]<<" "<<a[2];
    vector<float> b = vec[1] * (1.0f-u) + vec[2] * u;
    vector<float> c = vec[2] * (1.0f-u) + vec[3] * u;
    
    
    vector<float> d = a * (1.0f-u) + b * u;
    vector<float> e = b * (1.0f-u) + c * u;
    
    p = d * (1.0f-u) + e * u;
    
    vector<float> dPdu = (e-d) * 3.0f;
    
    pointderiv.push_back(p);
    pointderiv.push_back(dPdu);
    
    return pointderiv;
}


vector<vector<float> > bezpatchinterp(vector<vector<vector<float> > > vec, float u, float v) {
    vector<vector<float> > curve(4);
    for (int i=0; i<4; i++) {
        curve[0] = vec[i][0];
        curve[1] = vec[i][1];
        curve[2] = vec[i][2];
        curve[3] = vec[i][3];
        vcurve[i] = bezcurveinterp(curve, u);
    }
    for (int i=0; i<4; i++) {
        curve[0] = vec[0][i];
        curve[1] = vec[1][i];
        curve[2] = vec[2][i];
        curve[3] = vec[3][i];
        ucurve[i] = bezcurveinterp(curve, v);
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
    
    vector<float> norm = crossprod(ucurvePointDeriv[1], vcurvePointDeriv[1]);
    norm = norm*(1/(sqrt(sqr(norm[0])+sqr(norm[1])+sqr(norm[2]))));
    
    vector<vector<float> > retVec;
    retVec.push_back(ucurvePointDeriv[0]);
    retVec.push_back(norm);

    return retVec;
}

void savesurfpointnorm(vector<float> vec, vector<float> vec2) {
    unifpoints.push_back(vec);
    unifpoints2.push_back(vec2);
}


void subdividepatch(vector<vector<vector<float> > > patch) {
    int numdiv = ((1+0.001)/step);
    //cout<<"\nNUMDIV = "<<numdiv;
    //cout<<"\n10 mod step= "<<10%int(10*step);
    float u, v;
    
    for (int iu = 0; iu<=numdiv+1; iu++) {
        if (iu==numdiv+1) {
            if (10%int(10*step)!=0) {
                u = 1.0;
            }
        } else {
            u = iu * step;
        }
        for (int iv = 0; iv<=numdiv+1; iv++) {
            if (iv==numdiv+1) {
                if (10%int(10*step)!=0) {
                    v = 1.0;
                }
            } else {
                v = iv * step;
            }
            p = bezpatchinterp(patch, u, v)[0];
            n = bezpatchinterp(patch, u, v)[1];
            savesurfpointnorm(p, n);
    
        }
    }
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
        //Not sure why she's clearing all points at this step
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
    cout<<"\nMYDISPLAY";
    if (flat == true) {
        glShadeModel(GL_FLAT);
    } else {
        glShadeModel(GL_SMOOTH);
    }
    if (wireframe == true) {
        //draw with wireframe
    } else {
        //draw filled
    }
	
	color();
	
	glutSwapBuffers();

}

//Used to break from the glutMainLoop at a Spacebar press
void idleInput (unsigned char key, int xmouse, int ymouse) {

    //keypad(stdscr, TRUE);
    switch (key)
    
    {
        case 's': //toggle flat/smooth shading
            if (flat == true) {
                flat = false;
            } else {
                flat = true;
            }
            glutDisplayFunc(myDisplay);
            break;
            
        case 'w': //toggle filled/wireframe mode
            if (wireframe == true) {
                wireframe = false;
            } else {
                wireframe = true;
            }
            glutDisplayFunc(myDisplay);
            break;

		//case KEY_LEFT: //figure out special case for arrow keys
            //cout << "LEFT";
            //exit(0);
            
        case '+': //zoom in
            exit(0);
        case '-': //zoom out
            exit(0);
        
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
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    
    // Initalize theviewport size
    viewport.w = 400;
    viewport.h = 400;
    
    
    //The size and position of the window
    glutInitWindowSize(viewport.w, viewport.h);
    glutInitWindowPosition(800,0);
    glutCreateWindow("as3");
    
    
    initScene();
    glClear(GL_COLOR_BUFFER_BIT);// clear the color buffer

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-4, 4, -4, 4, 4, -4);

        gluLookAt(0, -1, 1, //4 for z/* look from camera XYZ */ //0, -4, 1 for teapot
                  0, 0, 0, /* look at the origin */
                  0, 1, 0); /* positive Y up vector */
    
    
    glMatrixMode(GL_MODELVIEW);	        // indicate we are specifying camera transformations
    glLoadIdentity();
    
    //glutDisplayFunc(myDisplay);

    
    //SET INPUTFILE TO GET PATCHES FROM HERE
    ifstream inputfile(argv[1]);
    step = atof(argv[2]);
    if (argc == 3) {
        uniform = true;
    } else if (strcmp(argv[3], "-a") == 0) {
        adaptive = true;
    }
    /*if (strcmp(argv[3], "-u") == 0) {
        uniform = true;
    }*/
    cout << "\nFile: " << inputfile;
    cout << "\nParameter: " << step;
    cout << "\nAdaptive: " << adaptive;
    cout << "\nUniform: " << uniform;
    cout << "\n";
    std::string s;

    string numPatches;
    getline(inputfile, numPatches, '\n');
    while (getline(inputfile, s, '\n')) {

        cout<<"\n\n calling patches ";
        patchesfromfile(s);

    }
	color();
    
    glutDisplayFunc(myDisplay);				// function to run when its time to draw something
    glutReshapeFunc(myReshape);				// function to run when the window gets resized
    glutKeyboardFunc(idleInput);
    glutMainLoop();							// infinite loop that will keep drawing and resizing

    
    return 0;
}

