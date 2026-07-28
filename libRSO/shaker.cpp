// Copyright (C) 1990-2026,  Roberto Battiti and Mauro Brunato
// Released under Creative Commons CC-BY-SA 4.0; see LICENSE.txt

#include "shaker.h"

Shaker::Shaker (int d, unsigned long seed) :
	n (d),
	x (new double[d]),
	current_value (1.0e30),
	delta (new double[d]),
	Rand (seed),
	norms(new double[d])
	{}

Shaker::~Shaker()
{
	delete[] delta;
	delete[] x;
	delete[] norms;
}