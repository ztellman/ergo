#ifndef _POSITION_H
#define _POSITION_H

#include <stdlib.h>

enum Color
{
	Black,
	White,
	Empty	
};

#define opponent(stone)		(Color(stone^1))

#define in_atari(p)			((p)->liberties <= 4 && (long long)((p)->sum * (p)->sum) == ((p)->liberties * (p)->sum_of_squares))
#define atari_position(p)	((p)->sum/(p)->liberties)

class Position
{
public:
	Position()
	{
	}
	
	Position(int i)
	{
		value = i;
		stone = Empty;
		
		liberties = 0;
		sum = 0;
		sum_of_squares = 0;

		neighbors[0] = 0;
		neighbors[1] = 0;
	}
	
	short value;
	Color stone;
	
	short liberties;
	unsigned int sum;
	long long sum_of_squares;
			
	Position *parent;
	char neighbors[2];	
};

#endif