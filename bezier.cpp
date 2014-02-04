#include "bezier.h"

Bezier::Bezier()
{

}

void Bezier::init()
{
	GL_COLOR_CLEAR_VALUE(0.0, 0.0, 0.0, 0.0);
	GL_SHADE_MODEL(GL_FLAT);
}


void Bezier::parseInput(std::string inputFile)
{
	std::ifstream inpFile(inputFile.c_str());
	if (!inpFile.is_open())
	{
		std::cout << "File was unable to open" << std::endl;
	} else {
		std::string line;

		while (inpFile.good())
		{
			std::vector<std::string> splitline;
			std::string buff;
			std::getline(inpFile, line);
			std::stringstream ss(line);

			while (ss >> buff)
			{
				splitline.push_back(buff);
			}
			if (splitline.size() == 0)
			{
				continue;
			}
			if (splitline[0][0] == '#')
			{
				continue;
			} 
			else if(splitline[0][0] == 'int')
			{

			}
			else 
			{
				std::cerr << "Unknown parsing error:" << splitline[0] << std::endl;
			}
		}
		inpFile.close();
	}
}

void Bezier::drawCurve(Point point1, Point point2, Point point3)
{


}

void Bezier::setPixel(Point point, Color color)
{
	//color in pixel

}