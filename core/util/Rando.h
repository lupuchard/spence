#ifndef SPENCE_RANDO_H
#define SPENCE_RANDO_H

#include <cstdint>

class Rando {
public:
	Rando(uint64_t seed);

	uint64_t rand();
	int64_t rand(int64_t min, int64_t max); // [min, max)
	double rand_float();

	uint64_t static min();
	uint64_t static max();
	inline uint64_t operator()() {
		return rand();
	}

private:
	uint64_t s[4];
};

#endif //SPENCE_RANDO_H
