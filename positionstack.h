#ifndef _MOVE_STACK_H
#define _MOVE_STACK_H

#include "position.h"
#include <assert.h>

#define STACK_SIZE 4
#define LARGE_STACK_SIZE NUM_POSITIONS

class PositionStack
{
private:
	Position *_positions[STACK_SIZE];
	int _offset;

public:
	//
	PositionStack()
	{
		clear();
	}

	//
	void push(Position *p)
	{
		assert(count() < STACK_SIZE);
		
		_positions[--_offset] = p;
	}

	Position* pop()
	{
		assert(count() > 0);
		
		return _positions[_offset++];
	}

	void clear()
	{
		_offset = STACK_SIZE;
	}

	//
	int count()
	{
		return STACK_SIZE - _offset;
	}

	bool is_empty()
	{
		return _offset == STACK_SIZE;
	}
};


class LargePositionStack
{
private:
	Position *_positions[LARGE_STACK_SIZE];
	int _offset;

public:
	//
	LargePositionStack()
	{
		clear();
	}

	//
	void push(Position *p)
	{
		assert(count() < LARGE_STACK_SIZE);
		
		_positions[--_offset] = p;
	}

	Position* pop()
	{
		assert(count() > 0);
		
		return _positions[_offset++];
	}

	void clear()
	{
		_offset = LARGE_STACK_SIZE;
	}

	//
	int count()
	{
		return LARGE_STACK_SIZE - _offset;
	}

	bool is_empty()
	{
		return _offset == LARGE_STACK_SIZE;
	}
};

#endif