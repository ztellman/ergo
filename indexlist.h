#ifndef _MOVE_LIST_H
#define _MOVE_LIST_H

#include "config.h"

#include <iostream>
#include <assert.h>

class IndexList
{
private:
	int _indices[NUM_POSITIONS];		
	int _offset;	

public:
	//
	IndexList()
	{
		clear();
	}

	//
	int get(int i)
	{
		assert(_offset+i < NUM_POSITIONS);
		assert(count() > 0);
				
		int index = _offset+i;
		int value = _indices[index];
		_indices[index] = _indices[_offset++];

		return value;
	}

	int get()
	{
		assert(count() > 0);
		
		return _indices[_offset++];
	}

	int find(int value)
	{
		assert(contains(value));

		for (int i = _offset; i < NUM_POSITIONS; i++)
		{
			if (_indices[i] == value)
			{
				return i - _offset;
			}
		}		
		return -1;
	}

	int peek()
	{
		return peek(0);
	}

	int peek(int i)
	{
		assert((_offset+i) < NUM_POSITIONS);
		
		return _indices[_offset + i];
	}
			
	void add(int i)
	{
		assert(_offset > 0);
		assert(!contains(i));
		assert(i >= 0);
		
		_indices[--_offset] = i;		
	}

	void clear()
	{
		_offset = NUM_POSITIONS;
	}

	void print()
	{
		for (int i = _offset; i < NUM_POSITIONS; i++)
		{
			if (i != _offset)
			{
				std::cout << ",";
			}
			std::cout << _indices[i];
		}
		std::cout << std::endl;
	}
	
	//
	int count()
	{
		return NUM_POSITIONS - _offset;
	}

	bool contains(int value)
	{
		for (int i = _offset; i < NUM_POSITIONS; i++)
		{
			if (_indices[i] == value)
			{
				return true;
			}
		}
		return false;
	}
};

#endif