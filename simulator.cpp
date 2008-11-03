#include "simulator.h"

#include <assert.h>
#include <iostream>
#include <sstream>

using namespace std;

///
Simulator::Simulator(string move_list, string weight_list)
{
	_board.register_simulator(this);
	
	for (int i = 0; i < NUM_POSITIONS; i++)
	{
		_positions.add(i);				 		
	}
	_atari = -1;
	_player = Black;

	parse_weights(weight_list);
	
	//Run through the move sequence
	stringstream moves(move_list);
	while (moves.peek() != EOF)
	{
		int position;
		moves >> position;

		if (position > 0)
		{
			try_move(_player, _positions.get(_positions.find(position)));			
		}
		next_player();
	}

	save_state(); //Lock our initial state		
}

///
Color Simulator::play_game()
{
	load_state();
	
	_num_moves = 0;
	_do_move_logging = true;
	_rejected.clear();
	
	int passes = 0;
	while (passes < 2)
	{
		while (_positions.count() > 0)
		{
			//_board.print();

			//If we're sure enough about the outcome, call it right now
			/*_prediction = make_prediction();
			if (_prediction != Empty)
			{
				finalize_game(_prediction);
				return _prediction;
			}*/
			
			//Generate a move
			int position = _positions.get(get_random_index(_player));
			if (try_move(_player, position))
			{
				passes = 0;
				next_player();
			}
						
			assert(verify_state() && _board.verify_state());			
		}
		
		passes++;		
		next_player();
	}

	//return the results of the game
	Color winner = _board.winner();
	finalize_game(winner);
	return winner;
}

bool Simulator::try_move(Color stone, int position)
{
	disallow_move(position); 

	if (_eyes.contains(position))
	{
		_rejected_eyes.add(position);
	}
	else if (_board.move(_player, position))
	{
		_num_moves++;		
		if (_do_move_logging)
		{
			_moves[_player].add(position);
		}
		assert(_board.verify_state());

		return true;
	}
	else
	{
		_rejected.add(position);
	}

	return false;
}

void Simulator::next_player()
{
	while (_rejected.count() > 0)
	{
		allow_move(_rejected.get());
	}
	_player = opponent(_player);
}
///
Color Simulator::make_prediction()
{
	if (_board.score(White) - _board.score(Black) > BOARD_SIZE*2)
	{
		return White;
	}
	else if (_board.score(Black) - _board.score(White) > BOARD_SIZE*2)
	{
		return Black;
	}

	return Empty;
}
///
bool Simulator::verify_state()
{
	int total[2];
	total[0] = total[1] = 0;

	for (int i = 0; i < _positions.count(); i++)
		for (int j = 0; j < 2; j++)
		{
			total[j] += _weight[j][_positions.peek(i)];
		}

	assert(total[White] == _total[White] && total[Black] == _total[Black]);

	return true;
}
	
///
int Simulator::num_moves()
{
	return _num_moves;
}

///
int Simulator::get_random_index(Color stone)
{
	//return _mersenne.rand(_positions.count()-1);
	
	if (_atari > 0 && (_mersenne.rand() & 1) > 0)
	{		
		int index = _positions.find(_atari);		
		_atari = -1;
		
		return index;
	}
	else
	{	
		_atari = -1;

		long r = _mersenne.rand(_total[stone]-1);
		int i = 0;
		while (r >= 0)
		{
			r -= _weight[stone][_positions.peek(i)];
			i++;
		}
		return i-1;
	}
}

///
void Simulator::load_state()
{
	_board.load_state();

	memcpy(&_eyes, _storage, SIMULATOR_SAVE_SIZE);

	_total[White] = _initial_total[White];
	_total[Black] = _initial_total[Black];

	_moves[White].clear();
	_moves[Black].clear();	
}

void Simulator::save_state()
{
	_board.save_state();

	_initial_total[White] = _total[White];
	_initial_total[Black] = _total[Black];

	memcpy(_storage, &_eyes, SIMULATOR_SAVE_SIZE);
}

///
void Simulator::parse_weights(string weights)
{
	if (weights.length() == 0)
	{
		for (int color = 0; color < 2; color++)
		{
			for (int i = 0; i < NUM_POSITIONS; i++)
			{
				_weight[color][i] = 1;
			}
			_initial_total[color] = _total[color] = NUM_POSITIONS;
		}

		assert(verify_state());
		return;
	}
	
	stringstream in(weights);
	for (int color = 0; color < 2; color++)
	{
		_initial_total[color] = 0;
		for (int i = 0; i < NUM_POSITIONS; i++)
		{
			assert(in.peek() > EOF);
			
			int weight = 0;
			in >> weight;
			weight /= 2;

			_weight[color][i] = 1 + weight;
			_initial_total[color] += 1 + weight;
		}
		_total[color] = _initial_total[color];
	}

	assert(verify_state());
}

string Simulator::encode_weights()
{
	stringstream out;
	for (int color = 0; color < 2; color++)
		for (int i = 0; i < NUM_POSITIONS; i++)
		{
			out << _weight[color][i] << " ";			
		}

	return out.str();
}
///
void Simulator::allow_move(int position)
{
	_positions.add(position);
	_total[White] += _weight[White][position];
	_total[Black] += _weight[Black][position];

	assert(verify_state());
}

void Simulator::disallow_move(int position)
{
	int prev = _total[White];
	
	_total[White] -= _weight[White][position];
	_total[Black] -= _weight[Black][position];	
	
	assert(verify_state());
}

void Simulator::finalize_game(Color winner)
{
	for (int i = 0; i < NUM_POSITIONS; i++)		
	{
		int check = _moves[winner].contains(i);
		_weight[winner][i] += check;
		_initial_total[winner] += check;		
	}
}
///
void Simulator::do_capture(int position)
{
	allow_move(position);
	
	_do_move_logging = false;
}

void Simulator::retract_capture()
{
	disallow_move(_positions.get());
}

void Simulator::set_eye(int position, bool state)
{
	if (state)
	{
		_eyes.add(position);
	}
	else
	{
		assert(_positions.contains(position) || _rejected.contains(position) || _rejected_eyes.contains(position));		
		_atari = position;

		assert(!_eyes.contains(position) || (_positions.contains(position) ^ _rejected_eyes.contains(position)));
		_eyes.remove(position);
		if (_rejected_eyes.try_remove(position))
		{
			allow_move(position);
		}
	}
}

