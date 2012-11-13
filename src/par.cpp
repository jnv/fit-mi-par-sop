/*
 ============================================================================
 Name        : MI-PAR-SOP Paralelni reseni
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
#include "common.h"
#include "mpiutil.h"
#include "Node.h"
#include "ModifiedStack.h"
using namespace std;

int n = 0, c = 0, a = 0;
int _upperBound = 0;
int * _inputSet; // pole vstupnich dat
ModifiedStack<Node> _stack;
//deque<Node*> _stack;

int _thisRank;
int _procCnt;

Node* _currentBest;

FILE* _logFile;

#define LOG_OUTPUT _logFile // cerr
// http://stackoverflow.com/questions/1644868/c-define-macro-for-debug-printing
#define logc(fmt) \
		do { fprintf(LOG_OUTPUT, fmt); fflush(LOG_OUTPUT); } while (0)
#define log(fmt, ...) \
        do { fprintf(LOG_OUTPUT, fmt, __VA_ARGS__); fflush(LOG_OUTPUT); } while (0)

void end()
{
	bcastEnd();
	fclose(_logFile);

	MPI_Finalize();

	delete[] _inputSet;

	exit(0);
}

/**
 * Main solve cycle
 */
void doSolve()
{

}

bool loadSet(char * fname)
{
	ifstream f;
	f.open(fname);

	if (!f.is_open())
	{
		log("Cannot open input file: %s\n", fname);
		return false;
	}

	f >> n;
#ifndef DEBUG
	if(n < 20)
	{
		log("n must be 20 or more\n");
		f.close();
		return false;
	}
#endif

	// limit by number of bytes
	// e.g. 1024 / 4 = 256
	int limit = MAX_N / sizeof(int);
	if (n > limit)
	{
		log("n must be lower than %d\n", limit);
		f.close();
		return false;
	}

	if ((n / _procCnt) == 0)
	{
		log("not enough work for %d processes", _procCnt);
		f.close();
		return false;
	}

	f >> c;
	f >> a;
#ifndef DEBUG
	if(a < 2 || a > n / 10)
	{
		log("a must be more than 1 and less than n/10.\n");
		f.close();
		return 0;
	}
#endif

	log("c (max price per subset): %d\n  a (number of subsets): %d\n", c, a);

	_inputSet = new int[n];

	log("Loading %d numbers\n", n);

	for (int i = 0; i < n; i++)
	{
		int in;
		f >> in;
		_inputSet[i] = in;
	}

	for (int i = 0; i < n; i++)
	{
		log("%d ", _inputSet[i]);
	}
	logc("\n");

	_upperBound = a * (c - 1);
	log("Upper bound is: %d\n", _upperBound);

	return true;
}

void expand()
{
}

void initFirstProc(char * fname)
{
	if (!loadSet(fname))
	{
		logc("Failed to load input file, exiting\n");
		end();
	}

	int initArr[4] =
	{ n, c, a, _upperBound };

	// Broadcast initialization attributes
	// initArr and _inputSet

	int pos = 0;
	char buffer[MAX_LEN];
	MPI_Pack(initArr, 4, MPI_INT, buffer, MAX_LEN, &pos, MPI_COMM_WORLD );
	MPI_Pack(_inputSet, n, MPI_INT, buffer, MAX_LEN, &pos, MPI_COMM_WORLD );

	for (int i = 1; i < _procCnt; i++) // 1 intentionally
	{
		MPI_Send(buffer, pos, MPI_PACKED, i, INIT_PACK, MPI_COMM_WORLD );
		//MPI_Send(initArr, 4, MPI_INT, i, INIT_ARR, MPI_COMM_WORLD );
		//MPI_Send(_inputSet, n, MPI_INT, i, INIT_SET, MPI_COMM_WORLD );
	}
	log("> Sent initialization array: n %d, c %d, a %d, upperBound %d\n",
			n, c, a, _upperBound);

	_currentBest = new Node();
	Node * node = _currentBest;

	// Expand first level
	for (int i = node->start; i < n; ++i)
	{
		int add = _inputSet[i];
		node->setTombstone(i); // Mark the current member as unused (will be overriden by Node::place)
		node->start = i + 1; // ...and set the start to the next element

		for (int subset = 0; subset < a; subset++)
		{
			// If the member can be added to the subset...
			if (node->isFeasible(subset, add))
			{
				// Create new node with the member placed into the subset
				Node * newNode = new Node(*node);
				newNode->place(subset, i);
				_stack.push(newNode->start, newNode);
			}
		}
	}
	int workLoad = _stack.getSize() / _procCnt;
	if (workLoad == 0)
	{
		log("Stack size after first expansion was %d, aborting\n",
				_stack.getSize());
		bcastEnd();
		MPI_Barrier(MPI_COMM_WORLD);
		end();
	}

	logc("| Arrived at barrier before expansion\n");
	MPI_Barrier(MPI_COMM_WORLD);

	// For each processor...
	for (int i = 1; i < _procCnt; i++)
	{
		sendInt(workLoad, i, INT); // Send the expected number of nodes
		for (int j = 0; j < workLoad; j++)
		{
			sendNode(_stack.pop_front(), i, n);
		}
	}
	log("> Sent %d nodes to all processes\n", workLoad);

	//_stack.push(0, new Node());

}

void initOtherProc()
{
	int initArr[4];
	MPI_Status status;
	//MPI_Recv(initArr, 4, MPI_INT, INIT_PROC, INIT_ARR, MPI_COMM_WORLD, &status);
	int pos = 0;
	char buffer[MAX_LEN];

	if (probeEnd())
	{
		logc("< Received END, aborting {1}\n");
		end();
	}

	MPI_Recv(buffer, MAX_LEN, MPI_PACKED, INIT_PROC, INIT_PACK, MPI_COMM_WORLD,
			&status);

	MPI_Unpack(buffer, MAX_LEN, &pos, initArr, 4, MPI_INT, MPI_COMM_WORLD );
	n = initArr[0];
	c = initArr[1];
	a = initArr[2];
	_upperBound = initArr[3];
	log("> Received initialization array: n %d, c %d, a %d, upperBound %d\n",
			n, c, a, _upperBound);
	fflush(_logFile);

	_inputSet = new int[n];
	//MPI_Recv(_inputSet, n, MPI_INT, INIT_PROC, INIT_SET, MPI_COMM_WORLD,&status);
	MPI_Unpack(buffer, MAX_LEN, &pos, _inputSet, n, MPI_INT, MPI_COMM_WORLD );
	log("> Received %d numbers for input set: ", n);
	for (int i = 0; i < n; i++)
	{
		log(" %d", _inputSet[i]);
	}
	logc("\n");

	logc("| Arrived at barrier, waiting for nodes\n");
	MPI_Barrier(MPI_COMM_WORLD );

	if (probeEnd())
	{
		logc("< Received END after barrier, aborting {2}\n");
		end();
	}

	int workLoad;
	MPI_Recv(&workLoad, 1, MPI_INT, INIT_PROC, INT, MPI_COMM_WORLD, &status);
	log("Will receive %d nodes\n", workLoad);

	_currentBest = new Node();

	int * setbuffer = new int[n];
	for (int i = 0; i < workLoad; i++)
	{
		MPI_Recv(setbuffer, n, MPI_INT, 0, NODE, MPI_COMM_WORLD, &status);
		Node * node = new Node(setbuffer);
		log("< Received node: \n%s", node->toString().c_str());
		_stack.push(node->start, node);
	}
	delete[] setbuffer;
}

int main(int argc, char ** argv)
{
	double start, stop;

	MPI_Init(&argc, &argv); /* start up MPI */
	MPI_Comm_rank(MPI_COMM_WORLD, &_thisRank);
	MPI_Comm_size(MPI_COMM_WORLD, &_procCnt);

	char logFileName[20];
	sprintf(logFileName, "logs/log-%d-%d.log", _thisRank, _procCnt);
	_logFile = fopen(logFileName, "w");

	log("Proc %d out of %d\n", _thisRank, _procCnt);

	//_currentBest = new Node();

	if (_thisRank == 0)
	{
		if (argc != 2)
		{
			cout << "Usage: par <file>" << endl;
			end();
		}
		initFirstProc(argv[1]);
	}
	else
	{
		initOtherProc();
	}

	MPI_Barrier(MPI_COMM_WORLD);
	start = MPI_Wtime();
	log("| Solve started at %f\n", start);
	//doSolve();
	stop = MPI_Wtime();
	log("| Finished. Total time: %f\n", stop - start);

	//cout << "Solve time: " << stop - start << endl;

	end();
	return 0;

}

