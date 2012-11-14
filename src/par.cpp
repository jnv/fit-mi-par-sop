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
#include "mpiutil.h"
#include "common.h"
#include "Node.h"
#include "ModifiedStack.h"
using namespace std;

int n = 0, c = 0, a = 0;
int _upperBound = 0;
int * _inputSet; // pole vstupnich dat
ModifiedStack<Node> _stack;
State _state;
//deque<Node*> _stack;

double solveStart, solveStop;

int _thisRank;
int _procCnt;

Node* _currentBest;

FILE* _logFile;

#define LOG_OUTPUT _logFile // cerr
void end()
{
	solveStop = MPI_Wtime();
	log("| End. Total time: %f\n", solveStop - solveStart);
	MPI_Finalize();

	//if (isInitProc())
	//{
	log("Best found solution:\n%s\n", _currentBest->toString().c_str());
	//}
	delete _currentBest;

	fclose(_logFile);
	delete[] _inputSet;

	exit(0);
}

/**
 * Expand one level
 * @param node Node to expand from
 * @return number of generated nodes
 */
int expand(Node * node)
{
	int expanded = 0;
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
				expanded++;
			}
		}
	}
	return expanded;
}

/**
 * Main solve cycle
 */
void doSolve()
{
	_state = ACTIVE;
	int flag;
	MPI_Status status;
	bool initTokenSent = false;
	int donor = 0;
	bool hasToken = false;
	TokenColor incColor = WHITE, ourColor = WHITE;

	while (true)
	{
		MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
		if (flag)
		{
			//prisla zprava, je treba ji obslouzit
			//v promenne status je tag (status.MPI_TAG), cislo odesilatele (status.MPI_SOURCE)
			//a pripadne cislo chyby (status.MPI_ERROR)
			switch (status.MPI_TAG)
			{
			case BETTER:
				// lepsi reseni
				// porovnat s nasim _currentBest a bud prijmout, nebo zahodit
				log("> Received BETTER from %d...\n", status.MPI_SOURCE);
				Node * tmp;
				tmp = rcvNode(status.MPI_SOURCE, BETTER);
				log("%s\n", tmp->toString().c_str());
				if (tmp->isBetterThan(_currentBest))
				{
					logc("Replacing our current best solution\n");
					delete _currentBest;
					_currentBest = tmp;
					if (tmp->hasMaxPrice())
					{
						logc("! Received solution has max price\n");
						if (isInitProc())
						{
							log("Received the best solution from %d\n",
									status.MPI_SOURCE);
							bcastEnd();
						}
						end();
					}
				}
				else
				{
					logc("Ignoring received solution\n");
					delete tmp;
				}

				break;
			case WORK_REQ:
				// zadost o praci, prijmout a dopovedet
				// zaslat rozdeleny zasobnik a nebo odmitnuti MSG_WORK_NOWORK
				break;
			case WORK_IN:
				// prisel rozdeleny zasobnik, prijmout
				// deserializovat a spustit vypocet
				break;
			case WORK_NONE:
				// odmitnuti zadosti o praci
				// zkusit jiny proces
				// a nebo se prepnout do pasivniho stavu a cekat na token
				break;
			case TOKEN:
				//ukoncovaci token, prijmout a nasledne preposlat
				// - bily nebo cerny v zavislosti na stavu procesu
				incColor = rcvToken(status.MPI_SOURCE);
				if (incColor == WHITE)
				{
					if (_state == ACTIVE)
					{
						logc("> WHITE token\n");
						hasToken = true;
					}
					else // WHITE + IDLE
					{
					}
				}
				else // BLACK
				{
					logc("> BLACK token\n");
					if (_state == ACTIVE)
					{
						if (isInitProc())
						{
							logc("Starting new round; ourColor = WHITE\n");
							hasToken = true; // start new round
							ourColor = WHITE;
						}
						else
						{
							logc("ourColor = BLACK\n");
							hasToken = true;
							ourColor = BLACK;
						}
					}
					else // BLACK + IDLE
					{
						if(isInitProc())
						{
							initTokenSent = false;
						}
						else
						{
							sendToken(BLACK);
						}
					}
				}
				break;
			case END:
				//konec vypoctu - proces 0 pomoci tokenu zjistil, ze jiz nikdo nema praci
				//a rozeslal zpravu ukoncujici vypocet
				//mam-li reseni, odeslu procesu 0
				//nasledne ukoncim spoji cinnost
				//jestlize se meri cas, nezapomen zavolat koncovou barieru MPI_Barrier (MPI_COMM_WORLD)
				end();
				break;
			default:
				log("> Received unknown tag: %d", status.MPI_TAG);
				break;
			}
		} //endif(flag)

		Node * node = NULL;
		if (_stack.isEmpty())
		{
			if (_state != IDLE)
			{
				logc("| Stack empty. Going to IDLE state\n");
				_state = IDLE;
			}
			else
			{
				logc("I\n");
			}

			/** IDLE
			 * WHITE -> WHITE
			 *  - initial process broadcasts
			 */
			if (hasToken)
			{
				hasToken = false;
				sendToken(ourColor);
			}

			if (isInitProc() && !initTokenSent)
			{
				logc("< Sending initial WHITE token\n");
				initTokenSent = true;
				sendToken(WHITE);
			}

			continue;
		}

		/***** Handle the node from stack here: *****/
		node = _stack.pop();

		if (node->hasMaxPrice())
		{
			logc("! Found node with max price\n");
			bcastNode(node, BETTER);
			delete _currentBest;
			_currentBest = node;
			end();
		}

		if (node->isBetterThan(_currentBest))
		{
			logc("< Found better solution, broadcasting\n");
			log("%s\n", node->toString().c_str());
			delete _currentBest;
			_currentBest = node;
			bcastNode(node, BETTER);
		}

		int expanded = expand(node);
		if (node != _currentBest)
		{
			delete node;
		}
	}
// expanze stavu
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

void initFirstProc(char * fname)
{
	if (!loadSet(fname))
	{
		logc("Failed to load input file, exiting\n");
		bcastEnd();
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
	log("< Sent initialization array: n %d, c %d, a %d, upperBound %d\n",
			n, c, a, _upperBound);

	_currentBest = new Node();
	Node * node = _currentBest;

// Expand first level
	int expanded = expand(node);
	int workLoad = expanded / _procCnt;
	if (workLoad == 0)
	{
		log("Stack size after first expansion was %d, aborting\n",
				_stack.getSize());
		bcastEnd();
		MPI_Barrier(MPI_COMM_WORLD );
		bcastEnd();
		end();
	}

	logc("| Arrived at barrier before expansion\n");
	MPI_Barrier(MPI_COMM_WORLD );

// For each processor...
	for (int i = 1; i < _procCnt; i++)
	{
		sendInt(workLoad, i, INT); // Send the expected number of nodes
		for (int j = 0; j < workLoad; j++)
		{
			sendNode(_stack.pop_front(), i, n);
		}
	}
	log("< Sent %d nodes to all processes\n", workLoad);

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
		logc("> Received END, aborting {1}\n");
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
		logc("> Received END after barrier, aborting {2}\n");
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
		log("> Received node: \n%s", node->toString().c_str());
		_stack.push(node->start, node);
	}
	delete[] setbuffer;
}

int main(int argc, char ** argv)
{

	MPI_Init(&argc, &argv); /* start up MPI */
	MPI_Comm_rank(MPI_COMM_WORLD, &_thisRank);
	MPI_Comm_size(MPI_COMM_WORLD, &_procCnt);

	char logFileName[20];
	sprintf(logFileName, "logs/%d-%d.log", _procCnt, _thisRank);
	_logFile = fopen(logFileName, "w");

	log("Proc %d out of %d\n", _thisRank, _procCnt);

	if (isInitProc())
	{
		if (argc != 2)
		{
			cout << "Usage: par <file>" << endl;
			bcastEnd();
			end();
		}
		initFirstProc(argv[1]);
	}
	else
	{
		initOtherProc();
	}

	MPI_Barrier(MPI_COMM_WORLD );
	solveStart = MPI_Wtime();
	log("| Solve started at %f\n", solveStart);
	doSolve();

	logc("Ending in main. This shouldn't happen...");
	return 1;

}

