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
#include <sys/resource.h>
#include "common.h"
#include "mpiutil.h"
#include "Node.h"
#include "ModifiedStack.h"
using namespace std;

int n = 0, c = 0, a = 0;
int _upperBound = 0;
int * _inputSet; // pole vstupnich dat
ModifiedStack<Node> _stack;

int _thisRank;
int _procCnt;

FILE* _logFile;

#define LOG_OUTPUT _logFile // cerr
// http://stackoverflow.com/questions/1644868/c-define-macro-for-debug-printing
#define logc(fmt) \
		do { fprintf(LOG_OUTPUT, fmt); } while (0)
#define log(fmt, ...) \
        do { fprintf(LOG_OUTPUT, fmt, __VA_ARGS__); } while (0)

void end()
{
	bcastEnd();
	fclose(_logFile);

	MPI_Finalize();

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
	for (int i = 1; i < _procCnt; i++) // 1 intentionally
	{
		MPI_Send(initArr, 4, MPI_INT, i, INIT_ARR, MPI_COMM_WORLD );
		MPI_Send(_inputSet, n, MPI_INT, i, INIT_SET, MPI_COMM_WORLD );
	}
	log("> Sent initialization array: n %d, c %d, a %d, upperBound %d",
			n, c, a, _upperBound);

}

void initOtherProc()
{
	int initArr[4];
	MPI_Status status;
	MPI_Recv(initArr, 4, MPI_INT, INIT_PROC, INIT_ARR, MPI_COMM_WORLD, &status);

	n = initArr[0];
	c = initArr[1];
	a = initArr[2];
	_upperBound = initArr[3];
	log("> Received initialization array: n %d, c %d, a %d, upperBound %d\n",
			n, c, a, _upperBound);

	_inputSet = new int[n];
	MPI_Recv(_inputSet, n, MPI_INT, INIT_PROC, INIT_SET, MPI_COMM_WORLD,
			&status);
	log("> Received %d numbers for input set\n", n);
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

	start = MPI_Wtime();
	//doSolve();
	stop = MPI_Wtime();
	delete[] _inputSet;

	//cout << "Solve time: " << stop - start << endl;

	end();
	return 0;

}

