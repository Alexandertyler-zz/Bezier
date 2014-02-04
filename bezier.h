#include <GL/gl.h>
#include <GL/glu.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream> 

class Bezier
{
public:
	Bezier();
	void init();
	void parseInput(std::string inputFile);
	void setPixel(Point point, Color color);


private:
	//this is a vector of point vectors that gets added to in parse input
	std::vector<std::vector<Point>> pointsVector;

};

class Point
{
public:
	Point();
	Point(double x, double y, double z);

private:
	double x, y, z;
};

class Color
{
public:
	Color();
	Color(double r, double g, double b);

private:
	double r, g, b;

};