#include"Maze.h"
#include"MazeToTga.h"
#include"GenerateMaze.h"
#include<ctime>
#include<cstdlib>
int main()
{
	std::srand(int(std::time(0)));
	GenerateMaze gen(40,40);
	Maze m = gen.generate();
	MazeToTga mtt;
	mtt.backgroundColor = {219,202,125};
	mtt.convert(m).save();
	
}