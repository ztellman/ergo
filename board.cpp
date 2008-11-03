#include "board.h"
#include "positionset.h"
#include "positionstack.h"
#include "mersenne.h"

#include <stdlib.h>
#include <iostream>

using namespace std;

Board::Board()
{
	_score[White] = 0;
	_score[Black] = 0;
	
	_curr_hash = 0;
	_hashes[0] = 0;

	int index = 0;
	int count = 0;
	for (int x = 0; x < BOARD_SIZE; x++)
	{
		for (int y = 0; y < BOARD_SIZE; y++)
		{
			count = 0;
			if (x > 0)
			{
				_neighbors[index][count++] = get_position(x-1, y);
			}
			if (x < BOARD_SIZE-1)
			{
				_neighbors[index][count++] = get_position(x+1, y);
			}
			if (y > 0)
			{
				_neighbors[index][count++] = get_position(x, y-1);
			}
			if (y < BOARD_SIZE-1)
			{
				_neighbors[index][count++] = get_position(x, y+1);
			}
			_num_neighbors[index++] = count;
		}		
	}

	for (int i = 0; i < NUM_POSITIONS; i++)
	{
		_positions[i] = Position(i);		
		_positions[i].parent = &_positions[i];		
	}

	Mersenne mersenne;
	for (int i = 0; i < NUM_POSITIONS; i++)
	{
		_zobrist[i] = mersenne.rand() ^ (mersenne.rand() << 32);				
	}
}

void Board::register_simulator(Simulator *simulator)
{
	_simulator = simulator;
}

///
void Board::save_state()
{
	memcpy(_storage, _positions, BOARD_SAVE_SIZE);
}

void Board::load_state()
{
	memcpy(_positions, _storage, BOARD_SAVE_SIZE);
}
///
Color Board::winner()
{
	for (int i = 0; i < NUM_POSITIONS; i++)
	{
		if (_eyes.contains(i))
		{
			_score[get_neighbor(&_positions[i], 0)->stone]++;
		}
	}

	return ((_score[White]+KOMI) > _score[Black]) ? White : Black;
}

int Board::score(Color stone)
{
	return _score[stone];
}
///
bool Board::move(Color stone, int position)
{	
	assert(position >= 0 && position < NUM_POSITIONS);
	assert(_positions[position].stone == Empty);
		
	Position *p = &_positions[position];
	p->parent = p;	

	do_move(p, stone);

	//Remove surrounded groups	
	int points = 0;
	while (!_dead_groups.is_empty())
	{
		Position *n = _dead_groups.pop();
			
		int num_stones = clear(n);					
		if (num_stones == 1)
		{
			_new_eyes.push(n);
		}
		points += num_stones;
	}

	//Check for ko
	if (points == 1 && check_ko())
	{	
		//Undo the move, and place the captured stone back on the board
		retract_move(p);
		_simulator->retract_capture();
		do_move(_new_eyes.pop(), opponent(stone));		

		return false;		
	}

	//Only add score once we've excluded ko
	_score[stone] += points; 

	//Is this suicide
	if (points == 0 && p->liberties == 0)
	{
		retract_move(p);
		return false;
	}

	//Have we invalidated the opponent's eye
	while (!_invalid_eyes.is_empty())
	{
		remove_eye(_invalid_eyes.pop());
	}

	//Have we invalidated our eye
	//(this goes below the opponent's eye because we only save one atari, 
	//and we want to skew towards capture rather than defense)
	if (in_atari(p))
	{		
		remove_eye(p);
	}

	//Check if we've created any new eyes
	while (!_new_eyes.is_empty())
	{
		add_eye(_new_eyes.pop());				
	}

	advance_hash();

	return true;
}

void Board::do_move(Position *p, Color stone)
{
	assert(p->stone == Empty);
	
	update_zobrist(p);

	p->stone = stone;
	p->liberties = 0;
	p->sum = 0;
	p->sum_of_squares = 0;

	_dead_groups.clear();
	_invalid_eyes.clear();
	_new_eyes.clear();

	Position *n;
	foreach_neighbor(p, n)
	{		
		n->neighbors[stone]++;
		
		if (n->stone == stone)
		{
			p->liberties--;
			p->sum -= p->value;
			p->sum_of_squares -= p->value*p->value;
			
			Position *parent = get_parent_safe(n);
			if (parent != p)
			{
				parent->parent = p;

				p->liberties += parent->liberties;
				p->sum += parent->sum;
				p->sum_of_squares += parent->sum_of_squares;
			}
		}
		else if (n->stone == opponent(stone))
		{
			Position *parent = get_parent(n);
			
			parent->liberties--;
			parent->sum -= p->value;
			parent->sum_of_squares -= p->value*p->value;

			if (parent->liberties == 0)
			{
				_dead_groups.push(parent);
			}
			else if (in_atari(parent))
			{
				_invalid_eyes.push(parent);
			}
		}
		else
		{
			p->liberties++;
			p->sum += n->value;
			p->sum_of_squares += n->value*n->value;

			if (n->neighbors[stone] == neighbor_count(n))
			{
				_new_eyes.push(n);
			}
		}
	}	
}
void Board::retract_move(Position *p)
{
	Color stone = p->stone;		
	p->stone = Empty;

	update_zobrist(p);

	Position *n;
	foreach_neighbor(p, n)
	{		
		n->neighbors[stone]--;

		if (n->stone == stone && get_parent_safe(n) == p)
		{
			trim_parent(n);
		}
		else if (n->stone == opponent(stone))
		{
			Position* parent = get_parent(n);
			
			parent->liberties++;
			parent->sum += p->value;
			parent->sum_of_squares += p->value*p->value;
		}
	}	
}
///
bool Board::add_eye(Position *p)
{
	bool is_eye = true;

	Position *n;
	foreach_neighbor(p, n)
	{
		is_eye &= !in_atari(get_parent(n));
	}
	if (is_eye)
	{
		_eyes.add(p);
		_simulator->set_eye(p->value, true);
	}
	
	return is_eye;
}

void Board::remove_eye(Position *p)
{
	//The group's already been captured
	if (p->stone == Empty)
	{
		return;
	}
	
	int atari = atari_position(p);
	_eyes.remove(atari);	
	_simulator->set_eye(atari, false);
}		

///
Position* Board::get_parent(Position *p)
{
	if (p->parent->parent == p->parent)
	{
		return p->parent;
	}	
	Position *parent = get_parent(p->parent);
	p->parent = parent;	
	
	return parent;
}

Position* Board::get_parent_safe(Position *p)
{
	if (p->parent->parent == p->parent)
	{
		return p->parent;
	}	
	return get_parent_safe(p->parent);	
}

void Board::trim_parent(Position *p)
{
	if (p->parent->parent == p->parent)
	{
		p->parent = p;
		return;
	}
	trim_parent(p->parent);
}

///
int Board::clear(Position *p)
{
	PositionSet set;
	set.add(p);
	
	LargePositionStack stack;
	stack.push(p);

	Color stone = p->stone;
	int points = 0;
	while (!stack.is_empty())
	{
		points++;
				
		Position *position = stack.pop();
		position->stone = Empty;
				
		update_zobrist(position);

		_simulator->do_capture(position->value);
		
		Position *n;
		foreach_neighbor(position, n)
		{		
			n->neighbors[stone]--;
			
			if (n->stone == stone && set.try_add(n))
			{
				stack.push(n);
			}
			else if (n->stone == opponent(stone))
			{
				Position *parent = get_parent(n);
				parent->liberties++;
				parent->sum += position->value;
				parent->sum_of_squares += position->value*position->value;
			}
		}
	}

	return points;
}

///
bool Board::verify_state()
{
	PositionSet set;
	for (int i = 0; i < NUM_POSITIONS; i++)
	{
		Position *p = &_positions[i];

		if (p->stone == Empty)
		{
			continue;
		}

		char neighbors[2];
		neighbors[White] = neighbors[Black] = 0;
		Position *n;
		foreach_neighbor(p, n)
		{
			if (n->stone != Empty)
			{
				neighbors[n->stone]++;
			}
		}
		assert(neighbors[White] == p->neighbors[White]);
		assert(neighbors[Black] == p->neighbors[Black]);

		Position* parent = get_parent(p);
		if (!set.try_add(parent))
		{
			continue;
		}
		
		short liberties = 0;
		unsigned int sum = 0;
		long long sum_of_squares = 0;
		
		PositionSet set;
		set.add(parent);

		LargePositionStack stack;
		stack.push(parent);

		while (!stack.is_empty())
		{
			Position *position = stack.pop();
			Position *n;
			foreach_neighbor(position, n)
			{
				if (n->stone == Empty)
				{
					liberties++;
					sum += n->value;
					sum_of_squares += n->value*n->value;
				}
				else if (n->stone == parent->stone && set.try_add(n))
				{
					stack.push(n);
				}
			}
		}

		assert(parent->liberties == liberties);
		assert(parent->liberties > 0);
		assert(parent->sum == sum);
		assert(parent->sum_of_squares == sum_of_squares);
	}

	return true;
}
void Board::print()
{
	for (int i = 0; i < BOARD_SIZE; i++)
	{
		for (int j = 0; j < BOARD_SIZE; j++)
		{
			char c = '-';
			if (get_position(i,j)->stone != Empty)
			{
				c = get_position(i,j)->stone == Black ? '#' : 'O';				
			}
			else if (_eyes.contains(get_position(i,j)))
			{
				c = '=';
			}			
			cout << c << " ";			
		}
		cout << endl;		
	}
	cout << endl;
}
					
				



		

