/*
 * Node.h
 *
 *  Created on: 9.11.2012
 *      Author: j
 */

#include <cstdlib>
#include <iostream>
#include <stack>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include "common.h"
using namespace std;

#ifndef NODE_H_
#define NODE_H_

struct Node
{

	int * placement;
	int * sums;
	int price;
	int start;

	Node()
	{
		init();
		for (int i = 0; i < a; i++)
		{
			sums[i] = 0;
		}
		for (int i = 0; i < n; i++)
		{
			placement[i] = PLACE_NONE;
		}
	}

	Node(const Node & o)
	{
		init();
		price = o.price;
		start = o.start;
		for (int i = 0; i < a; i++)
		{
			sums[i] = o.sums[i];
		}
		for (int i = 0; i < n; i++)
		{
			placement[i] = o.placement[i];
		}
	}

	void init()
	{
		sums = new int[a];
		placement = new int[n];
		price = 0;
		start = 0;
	}

	~Node()
	{
		delete[] sums;
		delete[] placement;
	}

	bool isFeasible(const int subset, const int number) const
	{
		return (sums[subset] + number) < c;
	}

	void place(const int subset, const int index)
	{
		int addition = _inputSet[index];
		placement[index] = subset;
		sums[subset] += addition;
		price += addition;
	}

	void setTombstone(const int index)
	{
		placement[index] = PLACE_TOMBSTONE;
	}

	int begin() const
	{
		return start;
	}

	bool empty()
	{
		return (price == 0);
	}

	string toString()
	{
		stringstream ret;
		stringstream * subsets = new stringstream[a];

		int width = 5;

		for (int i = 0; i < n; i++)
		{
			ret << setw(width) << _inputSet[i];
		}
		ret << endl;

		for (int i = 0; i < n; i++)
		{
			int place = placement[i];

			if (place == PLACE_NONE)
			{
				ret << setw(width) << 'N';
			}
			else if (place == PLACE_TOMBSTONE)
			{
				ret << setw(width) << 'X';
			}
			else
			{
				subsets[place] << _inputSet[i] << " ";
				ret << setw(width) << place;
			}
		}
		ret << endl;

		for (int i = 0; i < a; ++i)
		{
			ret << "Set " << i << " ";
			ret << "[" << sums[i] << "] ";
			ret << subsets[i].str();
			ret << endl;
		}
		delete[] subsets;

		ret << "Price " << price << endl;

		return ret.str();
	}

	bool isBetterThan(const Node * o) const
	{
		return (price > o->price);
	}

	inline int maxPrice() const
	{
		return _upperBound;
	}

	bool zeroPrice() const
	{
		return (price == 0);
	}

	bool hasMaxPrice() const
	{
		return price == maxPrice();
	}

};

#endif /* NODE_H_ */
