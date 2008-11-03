#include "simulator.h"

#include <iostream>

using namespace std;

#define GAMES 10000

void simulate_set(int num_games);

string weights;

int main()
{
	weights = "";
	
	for (int i = 2; i < 100000000; i*=2)
	{
		simulate_set(i);
	}
	
	getchar();
}


void simulate_set(int num_games)
{
	long start = clock();
	
	int games[2];
	games[0] = games[1] = 0;

	unsigned long num_moves = 0;
	
	Simulator simulator("", weights);
	for (int i = 0; i < num_games; i++)
	{
		Color winner = simulator.play_game();
		games[winner]++;
		
		num_moves += simulator.num_moves();
	}
	weights = simulator.encode_weights();	

	long end = clock();

	cout << "Number of games: " << num_games << endl;
	cout << num_games/((float)(end-start)/CLOCKS_PER_SEC) << " games per second" << endl;
	cout << "White: " << games[White]*100.0/num_games << "% Black: " << games[Black]*100.0/num_games << "%" << endl;
	cout << "Average moves per game: " << num_moves/num_games << endl;
	cout << endl;
}
