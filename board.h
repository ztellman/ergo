#ifndef _BOARD_H
#define _BOARD_H

#include "position.h"
#include "positionset.h"
#include "positionstack.h"
#include "config.h"

class Simulator;

#define NUM_HASHES 3

#define get_position(x,y)		(&_positions[(x)*BOARD_SIZE + (y)])
#define get_neighbor(p,i)		(_neighbors[((p)->value)][i])
#define neighbor_count(p)		(_num_neighbors[(p->value)])

#define advance_hash()			_hashes[(_curr_hash + 1)%NUM_HASHES] = _hashes[_curr_hash]; _curr_hash = (_curr_hash + 1) % NUM_HASHES;
#define update_zobrist(p)		_hashes[_curr_hash] ^= _zobrist[(p->value)];
#define check_ko()				(_hashes[(_curr_hash + (NUM_HASHES-2))%NUM_HASHES] == _hashes[_curr_hash])

#define foreach_neighbor(p, n)	int z = 0, n_count = neighbor_count(p); \
								Position **neighbor_p; \
								for (neighbor_p = &_neighbors[p->value][0], n = *neighbor_p; z < n_count; n = *(++neighbor_p), z++)

#define BOARD_SAVE_SIZE			(sizeof(Position)*NUM_POSITIONS + sizeof(PositionSet)+ sizeof(int)*3 + sizeof(long long)*NUM_HASHES)

class Board
{
private:
	//These get saved
	Position _positions[NUM_POSITIONS];	
	PositionSet _eyes;
	
	int _score[2];

	int _curr_hash;
	long long _hashes[NUM_HASHES];

	//This is what they get saved to
	char _storage[BOARD_SAVE_SIZE];

	//These are transient, and don't need to get saved
	PositionStack _dead_groups,
			      _invalid_eyes,
			      _new_eyes;

	Position *_neighbors[NUM_POSITIONS][4];
	int _num_neighbors[NUM_POSITIONS];

	long long _zobrist[NUM_POSITIONS];

	Simulator *_simulator;

public:
	//
	Board();
	void register_simulator(Simulator* simulator);

	//
	void save_state();
	void load_state();
	
	//
	Color winner();
	int score(Color stone);

	//
	bool move(enum Color stone, int position);
		
	//
	void print();
	bool verify_state();

private:
	//	
	Position* get_parent(Position *p);	
	Position* get_parent_safe(Position *p);  //Doesn't use path compression for easy move retraction
	void trim_parent(Position *p);
	
	//
	void remove_eye(Position *p);
	bool add_eye(Position *p);	

	//
	void do_move(Position *p, enum Color stone);
	void retract_move(Position *p);

	//
	int clear(Position *p);		
};

#include "simulator.h"

#endif