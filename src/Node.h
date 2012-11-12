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

//#define TWOLINE //two-line output

struct Node
{
	/**
	 * Placement of set's members into subsets, e.g.:
	 * _inputSet: 1  2  3  4  5  6
	 * placement: 0  1  X  1  N  N
	 * Where:
	 * 0, 1 - subset 0, 1
	 * N - PLACE_NONE - member was not yet placed
	 * X - PLACE_TOMBSTONE - member will be ignored
	 */
	int * placement;

	/**
	 * Array of sums of subsets, e.g.
	 * _inputSet: 1  2  3  4  5  6
	 * placement: 0  1  X  1  N  N
	 * sums[0] 1
	 * sums[1] 6
	 */
	int * sums;

	/**
	 * Total price of this node
	 */
	int price;

	/**
	 * Starting element, used for caching
	 * All previous placed members and tombstones should be ignored
	 *
	 * E.g., for the following placement:
	 * 0  1  X  1  N  N
	 * start can be calculated as 4
	 */
	int start;

	/**
	 * Standard constructor
	 */
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

	/**
	 * Construct node from an array of placements
	 * Great for unpacking!
	 * @param inputPlacement
	 */
	Node(int * inputPlacement)
	{
		init();

		// We have to do this ourselves
		for (int i = 0; i < a; i++)
		{
			sums[i] = 0;
		}

		bool startWasSet = false;
		// For each number in inputSet
		int i;
		for (i = 0; i < n; i++)
		{
			int set = inputPlacement[i]; // set is 0..a or PLACE_NONE or PLACE_TOMBSTONE

			placement[i] = set; // Copy it

			// Tombstones and NONE shouldn't be calculated into the sums and total price
			if (set == PLACE_TOMBSTONE)
			{
				continue;
			}
			if (set == PLACE_NONE)
			{
				if (!startWasSet) // set the start to the index of the first PLACE_NONE
				{
					start = i;
					startWasSet = true;
				}
				continue;
			}

			int value = _inputSet[i];
			sums[set] += value;
			price += value;
		}

		// There's no PLACE_NONE in the input solution
		if(!startWasSet)
		{
			start = i;
		}
	}

	/**
	 * Copy constructor
	 * @param o
	 */
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

	/**
	 * Initialization
	 * Set-up sums, placement arrays, price and start
	 */
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

	/**
	 * Whether the given number can be added to a subset
	 * (the resulting sum must be lower than c)
	 * @param subset
	 * @param number
	 * @return
	 */
	bool isFeasible(const int subset, const int number) const
	{
		return (sums[subset] + number) < c;
	}

	/**
	 * Whether more nodes can be generated from this one
	 * @return
	 */
	bool isFinished() const
	{
		return (start >= n);
	}

	/**
	 * Place the number on the index of the _inputSet
	 * into a given subset
	 * Also adds the number to a sum of the subset (in subsets array)
	 * and total price of the node
	 * @param subset
	 * @param index
	 */
	void place(const int subset, const int index)
	{
		int addition = _inputSet[index];
		placement[index] = subset;
		sums[subset] += addition;
		price += addition;
	}

	/**
	 * Sets the number on the index as tombstone
	 * @param index
	 */
	void setTombstone(const int index)
	{
		placement[index] = PLACE_TOMBSTONE;
	}

	bool empty()
	{
		return (price == 0);
	}

	/**
	 * @param o
	 * @return True if the price of this node is higher than the o node
	 */
	bool isBetterThan(const Node * o) const
	{
		return (price > o->price);
	}

	inline int maxPrice() const
	{
		return _upperBound;
	}

	bool hasZeroPrice() const
	{
		return (price == 0);
	}

	/**
	 * @return True if this is possibly the best solution
	 */
	bool hasMaxPrice() const
	{
		return price == maxPrice();
	}

	/**
	 * String representation of the node
	 * Outputs _inputSet's members placements, eg:
	 * <samp>1:X 3:0 4:N 7:X</samp>
	 * Where: X is tombstone, N is unused member, integer is a member's subset
	 * @return
	 */
	string toString()
	{
		stringstream ret;
		stringstream * subsets = new stringstream[a];

		int width = 5;

#ifdef TWOLINE
		for (int i = 0; i < n; i++)
		{
			ret << setw(width) << _inputSet[i];
		}
		ret << endl;
#endif

		for (int i = 0; i < n; i++)
		{
			int place = placement[i];

#ifndef TWOLINE
			ret << setw(width) << _inputSet[i] << ':';
			ret << setw(0);
#else
			ret << setw(width);
#endif

			if (place == PLACE_NONE)
			{
				ret << 'N';
			}
			else if (place == PLACE_TOMBSTONE)
			{
				ret << 'X';
			}
			else
			{
				subsets[place] << _inputSet[i] << " ";
				ret << place;
			}
		}
		ret << endl;
		ret << setw(0);

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

};

#endif /* NODE_H_ */
