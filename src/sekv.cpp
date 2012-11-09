/*
 ============================================================================
 Name        : MI-PAR-SOP Sekv
 Author      :
 Description :
 ============================================================================
 */
#include <cstdlib>
#include <iostream>
#include <stack>
#include <vector>
#include <set>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include "mpi.h"
using namespace std;

#define DEBUG 1

#define PLACE_NONE -1
#define PLACE_TOMBSTONE -2

int n = 0, c = 0, a = 0;
int _upperBound = 0;
int * _inputSet; // pole vstupnich dat
set<int> S;

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

//	int next(int after)
//	{
//
//		for (int i = after; i < n; ++i)
//		{
//			if ((*placement)[i] != PLACE_NONE)
//			{
//				continue;
//			}
//			(*placement)[i] = PLACE_TOMBSTONE;
//			return i;
//		}
//		return n;
//	}

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
//		for (int i; i < a; i++)
//		{
//			subsets[i].clear();
//		}

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

		ret << "Price " << price << endl;

		for (int i = 0; i < a; ++i)
		{
			ret << "Set " << i << " ";
			ret << "[" << sums[i] << "] ";
//			for (vector<int>::iterator it = subsets[i].begin();
//					it < subsets[i].end(); it++)
//				ret << " " << *it;
			ret << subsets[i].str();
			ret << endl;
		}
		delete[] subsets;

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

void doSolve()
{
	stack<Node*> stack;
	//bool found = false;

	Node * solution = new Node();
	stack.push(solution);

	while (!stack.empty())
	{
		Node * node = stack.top();
		stack.pop();

		if (node->hasMaxPrice())
		{
			cout << "Maximum price solution" << endl;
			delete solution;
			solution = node;
			break;
		}

		if (node->isBetterThan(solution))
		{
			cout << "Better solution:" << endl;
			cout << node->toString() << endl;

			delete solution;
			solution = node;
		}

		/*if (node->empty())
		 {
		 cout << "Empty set." << endl;
		 continue;
		 }*/

		for (int i = node->start; i < n; ++i)
		{
			int add = _inputSet[i]; // for each element in the input set...
			node->setTombstone(i);
			node->start = i + 1; // + 1;
			for (int subset = 0; subset < a; subset++)
			{
				if (node->isFeasible(subset, add)) // try if it can be added
				{
					Node * newNode = new Node(*node); // and add it
					newNode->place(subset, i);
					stack.push(newNode);
					cout << newNode->toString() << endl;
				}
			}
		}

#ifdef DEBUG
//		cout << node->toString();
#endif
		if (node != solution)
			delete node;
	}

	cout << "----------" << endl;

	if (solution->zeroPrice())
	{
		cout << "Found nothing." << endl;
	}
	else
	{
		cout << "Found solution: " << endl << solution->toString() << endl;
	}

	delete solution;
}

bool loadSet(char * fname)
{
	ifstream f;
	f.open(fname);

	if (!f.is_open())
	{
		cout << "Cannot open input file: " << fname << endl;
		return false;
	}

	f >> n;
#ifndef DEBUG
	if(n < 20)
	{
		cout << "n must be 20 or more";
		f.close();
		return false;
	}
#endif

	f >> c;
	f >> a;
#ifndef DEBUG
	if(a < 2 || a > n / 10)
	{
		cout << "a must be more than 1 and less than n/10.";
		f.close();
		return 0;
	}
#endif

	cout << "c (max price per subset): " << c << endl
			<< "a (number of subsets): " << a << endl;

	_inputSet = new int[n];

	cout << "Loading " << n << " numbers" << endl;

	for (int i = 0; i < n; i++)
	{
		int in;
		f >> in;
		_inputSet[i] = in;
		S.insert(in);
	}

	cout << "Loaded: ";
	for (int i = 0; i < n; i++)
	{
		cout << _inputSet[i] << " ";
	}
	cout << endl;

	_upperBound = a * (c - 1);
	cout << "Upper bound is: " << _upperBound << endl;

	return true;
}

int main(int argc, char ** argv)
{
	double start, stop;
	if (argc != 2)
	{
		cout << "Usage: sop <file>" << endl;
		return 0;
	}

	if (!loadSet(argv[1]))
	{
		return 1;
	}

	MPI_Init(&argc, &argv); /* start up MPI */
	start = MPI_Wtime();
	doSolve();
	stop = MPI_Wtime();
	delete[] _inputSet;

	cout << "Solve time: " << stop - start << endl;

	return 0;

}

