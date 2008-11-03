#ifndef _POSITION_SET_H
#define _POSITION_SET_H

#include "config.h"
#include "position.h"

#include <iostream>

#define BIT_SHIFT 5
#define BIT_MASK 0x1F
#define ARRAY_SIZE ((NUM_POSITIONS >> BIT_SHIFT)+1)

class PositionSet
{
private:
	unsigned int _positions[ARRAY_SIZE];
	short _count;
	
public:
	//
	PositionSet()
	{
		clear();
	}
		
	//
	void add(Position *p)
	{
		add(p->value);
	}
	
	void add(int position)
	{
		//_positions[position >> BIT_SHIFT] |= (1 << (position & BIT_MASK));
		try_add(position);
	}

	bool try_add(Position *p)
	{
		return try_add(p->value);
	}
	
	bool try_add(int position)
	{
		int index = position >> BIT_SHIFT;
		int value = 1 << (position & BIT_MASK);

		bool priorValue = (_positions[index] & value) > 0;	
		_positions[index] |= value;
		
		_count += !priorValue;
		return !priorValue;
	}
	
	void remove(Position *p)	
	{
		remove(p->value);
	}

	void remove(int position)
	{
		//_positions[position >> BIT_SHIFT] &= ~(1 << (position & BIT_MASK));
		try_remove(position);
	}

	bool try_remove(int position)
	{
		int index = position >> BIT_SHIFT;
		int value = 1 << (position & BIT_MASK);

		bool priorValue = (_positions[index] & value) > 0;	
		_positions[index] &= ~value;
		
		_count -= priorValue;
		return priorValue;
	}
	
	bool contains(Position *p)
	{
		return contains(p->value);
	}
	
	bool contains(int position)
	{
		return (_positions[position >> BIT_SHIFT] & (1 << (position & BIT_MASK))) > 0;
	}

	void clear()
	{
		for (int i = 0; i < ARRAY_SIZE; i++)
		{
			_positions[i] = 0;
		}
		_count = 0;
	}

	//
	unsigned int count()
	{
		return _count;
	}

	//
	void print()
	{
		for (int i = 0; i < ARRAY_SIZE; i++)
		{
			for (int j = 0; j < 32; j++)
			{
				if (_positions[i] & 1 << j)
				{
					std::cout << i*32 + j << ",";
				}
			}
		}
		std::cout << std::endl;
	}
};

#endif