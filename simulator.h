#ifndef _SIMULATOR_H
#define _SIMULATOR_H

#include "positionset.h"
#include "indexlist.h"
#include "board.h"
#include "mersenne.h"

#define SIMULATOR_SAVE_SIZE (sizeof(PositionSet)*2 + sizeof(IndexList) + sizeof(Color) + sizeof(int))

class Simulator
{
private:
	Board _board;
	
	//These need to be saved
	PositionSet _eyes,
			    _rejected_eyes;	
	IndexList _positions;
	Color _player;
	int _atari;

	//This where they're saved to
	char _storage[SIMULATOR_SAVE_SIZE];

	//Mid-simulation state
	IndexList _rejected;
	int _num_moves;
	bool _do_move_logging;

	//Pan-simulation state
	Mersenne _mersenne;
	
	PositionSet _moves[2];
	unsigned long _weight[2][NUM_POSITIONS];
	unsigned long _total[2],
				  _initial_total[2];
	
public:
	Simulator(std::string move_list, std::string weight_list);

	Color play_game();
	int num_moves();

	std::string encode_weights();

	bool verify_state();

	//Callbacks for the board
	void set_eye(int position, bool state);	
	void do_capture(int position);	
	void retract_capture();	
		
private:
	//
	void save_state();
	void load_state();

	//
	bool try_move(Color stone, int position);
	void next_player();	
	void allow_move(int position);
	void disallow_move(int position);
	void finalize_game(Color winner);

	//
	Color make_prediction();

	//
	void parse_weights(std::string weights);
	
	//
	int get_random_index(Color stone);
};

#endif