#ifndef _MERSENNE_H
#define _MERSENNE_H

#include <time.h>
#include <math.h>
#include <limits.h>

class Mersenne
{
private:
	typedef unsigned long uint32;  // unsigned integer type, at least 32 bits
	
	enum { N = 624 };       // length of state vector
	enum { SAVE = N + 1 };  // length of array for save()
	enum { M = 397 };  // period parameter
	
	uint32 _state[N];   // internal state
	uint32 *_p_next;     // next value to get from state
	int _left;          // number of values left before reload needed

public:
	Mersenne()
	{
		seed( hash(time(NULL), clock()) );
	}

	void seed(const uint32 seed)
	{
		initialize(seed);
		reload();
	}

	uint32 hash( time_t t, clock_t c )
	{
		static uint32 differ = 0;  // guarantee time-based seeds will change

		uint32 h1 = 0;
		unsigned char *p = (unsigned char *) &t;
		for( size_t i = 0; i < sizeof(t); ++i )
		{
			h1 *= UCHAR_MAX + 2U;
			h1 += p[i];
		}
		uint32 h2 = 0;
		p = (unsigned char *) &c;
		for( size_t j = 0; j < sizeof(c); ++j )
		{
			h2 *= UCHAR_MAX + 2U;
			h2 += p[j];
		}
		return ( h1 + differ++ ) ^ h2;
	}

	uint32 rand(unsigned int n)
	{
		uint32 used = n;	
		used |= used >> 1;
		used |= used >> 2;
		used |= used >> 4;
		used |= used >> 8;
		used |= used >> 16;
		
		// Draw numbers until one is found in [0,n]
		uint32 i;
		do
			i = rand() & used;  // toss unused bits to shorten search
		while( i > n );
		
		assert(0 <= i && i <= n);
		return i;
	}

	uint32 rand()
	{
		if( _left == 0 ) reload();
		--_left;
			
		register uint32 s1;
		s1 = *_p_next++;
		s1 ^= (s1 >> 11);
		s1 ^= (s1 <<  7) & 0x9d2c5680UL;
		s1 ^= (s1 << 15) & 0xefc60000UL;
		return ( s1 ^ (s1 >> 18) );
	}
	
private:
	uint32 hiBit( const uint32& u ) const 
	{ 
		return u & 0x80000000UL; 
	}

	uint32 loBit( const uint32& u ) const 
	{ 
		return u & 0x00000001UL; 
	}
	
	uint32 loBits( const uint32& u ) const 
	{ 
		return u & 0x7fffffffUL; 
	}
	
	uint32 mixBits( const uint32& u, const uint32& v ) const
	{ 
		return hiBit(u) | loBits(v); 
	}

	uint32 twist( const uint32& m, const uint32& s0, const uint32& s1 ) const
	{ 
		return m ^ (mixBits(s0,s1)>>1) ^ (loBit(s1) & 0x9908b0dfUL); 
	}

	void initialize(const uint32 seed)
	{
		register uint32 *s = _state;
		register uint32 *r = _state;
		register int i = 1;
		
		*s++ = seed & 0xffffffffUL;
		for( ; i < N; ++i )
		{
			*s++ = ( 1812433253UL  *( *r ^ (*r >> 30) ) + i ) & 0xffffffffUL;
			r++;
		}
	}

	void reload()
	{
		register uint32 *p = _state;
		register int i;
		for( i = N - M; i--; ++p )
		{
			*p = twist( p[M], p[0], p[1] );
		}
		for( i = M; --i; ++p )
		{
			*p = twist( p[M-N], p[0], p[1] );
		}
		*p = twist( p[M-N], p[0], _state[0] );

		_left = N, _p_next = _state;
	}
};

#endif