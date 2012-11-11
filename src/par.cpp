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
using namespace std;

int n = 0, c = 0, a = 0;
int _upperBound = 0;
int * _inputSet; // pole vstupnich dat

int _thisRank;
int _procCnt;

FILE* _logFile;

#define LOG_OUTPUT _logFile // cerr

// http://stackoverflow.com/questions/1644868/c-define-macro-for-debug-printing
#define log(fmt, ...) \
            do { fprintf(LOG_OUTPUT, fmt, __VA_ARGS__); } while (0)

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

void initFirstProc(char * fname)
{

}

void initOtherProc()
{

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
		initFirstProc(argv[1]);
	}
	else
	{
		initOtherProc();
	}

	if (argc != 2)
	{
		cout << "Usage: par <file>" << endl;
		return 0;
	}

	if (!loadSet(argv[1]))
	{
		return 1;
	}

	start = MPI_Wtime();
	//doSolve();
	stop = MPI_Wtime();
	delete[] _inputSet;

	//cout << "Solve time: " << stop - start << endl;

	fclose(_logFile);

	return 0;

}

