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
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include "mpi.h"
#include "common.h"
#include "Node.h"
using namespace std;

int n = 0, c = 0, a = 0;
int _upperBound = 0;
int * _inputSet; // pole vstupnich dat

/**
 * Main solve cycle
 */
void doSolve()
{
	stack<Node*> stack; // work stack

	// Create empty, and the temporary best, solution
	Node * solution = new Node();
	stack.push(solution);

	while (!stack.empty())
	{
		// Get the top node
		Node * node = stack.top();
		stack.pop();

		// Node is (c-1)*a, time to end
		if (node->hasMaxPrice())
		{
			cout << "Maximum price solution" << endl;
			delete solution;
			solution = node;
			break;
		}

		// Node has higher price than the current solution
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

		// For each element in the input set...
		// (Ignore previous members of set)
		for (int i = node->start; i < n; ++i)
		{
			int add = _inputSet[i];
			node->setTombstone(i); // Mark the current member as unused (will be overriden by Node::place)
			node->start = i + 1; // ...and set the start to the next element

			// For each subset...
			for (int subset = 0; subset < a; subset++)
			{
				// If the member can be added to the subset...
				if (node->isFeasible(subset, add))
				{
					// Create new node with the member placed into the subset
					Node * newNode = new Node(*node);
					newNode->place(subset, i);
					stack.push(newNode);
				}
			}
		}

		if (node != solution)
			delete node;
	}

	cout << "----------" << endl;

	if (solution->hasZeroPrice())
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
		cout << "Usage: sekv <file>" << endl;
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

