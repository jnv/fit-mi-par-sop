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
#include <set>
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

void end()
{
	logc("| Arrived to final barrier\n");
	MPI_Barrier(MPI_COMM_WORLD);
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

bool handleWorkReq(int source)
{
	int flag = 1, k = 0;
	//bool reqProcs[32] =	{ false };
	set<int> reqProcs;
	set<int>::iterator it;
	MPI_Status status;

	bool ret = false;
	// collect all work requests
	while (flag)
	{
		int reqProc;
		MPI_Recv(&reqProc, 1, MPI_INT, source, WORK_REQ, MPI_COMM_WORLD,
				&status);
		reqProcs.insert(reqProc);

		MPI_Iprobe(MPI_ANY_SOURCE, WORK_REQ, MPI_COMM_WORLD, &flag, &status);
	}

	k = (int) reqProcs.size();
	log("Total work requests: %d\n", k);

	//int workReqPerProc = 0;
	//int workPerProc = _stack.getSize() / 2;

	int cnt;
	// For each requesting processor...
	for (it = reqProcs.begin(); it != reqProcs.end(); it++)
	{
		int requestingProc = *it;
		int remains = _stack.getSize() - _stack.getBcount();
		log("Bottom count: %d, remains: %d\n", _stack.getBcount(), remains);

		cnt = _stack.getBcount();
		if (remains <= 2 * CUT_LEVEL)
		{
			cnt /= 2;
		}
		cnt /= k;

		if (cnt == 0)
		{
			log("< Sorry punk %d, no work for ya\n", requestingProc);
			sendInt(_thisRank, requestingProc, WORK_NONE);
			reqProcs.erase(it);
			k--;
		}

		if (cnt > 0) // Once we get rid enough procs...
		{
			break;
		}
	}

	for (it = reqProcs.begin(); it != reqProcs.end(); it++)
	{
		int requestingProc = *it;
		log("< Sending %d nodes to %d\n", cnt, requestingProc);

		sendInt(cnt, requestingProc, WORK_IN);
		for (int i = 0; i < cnt; i++)
		{
			Node * node = _stack.pop_front();

			sendNode(node, requestingProc, n, WORK_IN);
			delete node;
		}

		if (requestingProc < _thisRank)
		{
			ret = true;
		}
	}

	return ret;

	//MPI_Recv(&leech, )
	return true;
}

int incDonor(int donor)
{
	donor = (donor + 1) % _procCnt;
	if (donor == _thisRank)
	{
		donor = (_thisRank + 1) % _procCnt;
	}
	return donor;
}

void collectNodes()
{
	logc("--------------\nNo best solution was found, collecting partial solutions.\n");
	for (int i = 1; i < _procCnt; i++)
	{
		Node * node = rcvNode(i, BETTER);
		if (node->isBetterThan(_currentBest))
		{
			delete _currentBest;
			_currentBest = node;
		}
		else
		{
			delete node;
		}
	}
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
	int donor = (_thisRank + 1) % _procCnt;
	bool hasToken = false;
	TokenColor sendColor;
	bool workRequested = false;
	int checkCtr = 0;

	int workReqCtr = 0;
	int initialDonor = donor;

	int checkMsgAmount = CHECK_MSG_AMOUNT; // Variable so we can probe more often when IDLE

	log("Initial donor will be: %d\n", donor);

	while (true)
	{
		checkCtr++;
		if ((checkCtr % checkMsgAmount) == 0)
		//if(true)
		{
			MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag,
					&status);
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
					log("> WORK_REQ from %d\n", status.MPI_SOURCE);

					if (_state == IDLE)
					{
						int reqProc;
						logc("< In IDLE state, rejecting\n");
						MPI_Recv(&reqProc, 1, MPI_INT, status.MPI_SOURCE,
								WORK_REQ, MPI_COMM_WORLD, &status);
						sendInt(_thisRank, reqProc, WORK_NONE);
					}
					else
					{
						_stack.recountBcount();

						int threshold;
						threshold = n - _stack.getBLevel();

						if (threshold > CUT_LEVEL)
						{
							log("Will handle work request, %d > %d\n",
									threshold, CUT_LEVEL);
							if (handleWorkReq(status.MPI_SOURCE))
							{
								sendColor = BLACK; // Nodes were sent back, we could end prematurely
							}
						}
						else
						{
							int reqProc;
							log("< Rejecting WORK_REQ because %d <= %d\n",
									threshold, CUT_LEVEL);
							MPI_Recv(&reqProc, 1, MPI_INT, status.MPI_SOURCE,
									WORK_REQ, MPI_COMM_WORLD, &status);
							sendInt(_thisRank, reqProc, WORK_NONE);
						}
					}

					break;
				case WORK_IN:
					// prisel rozdeleny zasobnik, prijmout
					// deserializovat a spustit vypocet
					log("> WORK_IN from %d\n", status.MPI_SOURCE);
					int cnt;

					MPI_Status stmp;

					MPI_Recv(&cnt, 1, MPI_INT, status.MPI_SOURCE, WORK_IN,
							MPI_COMM_WORLD, &stmp);
					log("> %d nodes will be loaded\n", cnt);
					for (int i = 0; i < cnt; i++)
					{
						Node * node = rcvNode(status.MPI_SOURCE, WORK_IN);
						_stack.push(node->start, node);
					}
					logc("Getting ACTIVE\n");
					_state = ACTIVE;
					workRequested = false;
					break;
				case WORK_NONE:
					// odmitnuti zadosti o praci
					// zkusit jiny proces
					// a nebo se prepnout do pasivniho stavu a cekat na token
					log("> Got rejected by %d\n", status.MPI_SOURCE);
					dropMsg(status.MPI_SOURCE, WORK_NONE);

					workRequested = false;

					/*if (donor == initialDonor)
					 {
					 break;
					 }
					 else
					 {*/
					/*log("< Requesting work from %d\n", donor);
					 sendInt(_thisRank, donor, WORK_REQ);
					 donor = incDonor(donor);
					 }*/
					break;
				case TOKEN:
					//ukoncovaci token, prijmout a nasledne preposlat
					// - bily nebo cerny v zavislosti na stavu procesu
					TokenColor incColor;
					incColor = rcvToken(status.MPI_SOURCE);
					hasToken = true;

					if (incColor == WHITE)
					{
						logc("> WHITE token\n");
						if (isInitProc())
						{
							logc("Received WHITE token, ending\n");
							bcastEnd();
							collectNodes();
							end();
						}

						if (_state == ACTIVE)
						{
							logc("Recoloring token to BLACK\n");
							sendColor = BLACK;
						}
						else // IDLE
						{
							sendColor = WHITE;
						}
					}
					else // (incColor == BLACK)
					{
						if (isInitProc())
						{
							initTokenSent = false; // Resend initial token once InitProc becomes IDLE
						}
						else
						{
							logc("> BLACK token\n");
							sendColor = BLACK; // Resend black, no matter what our color is
						}
					}

					break;
				case END:
					//konec vypoctu - proces 0 pomoci tokenu zjistil, ze jiz nikdo nema praci
					//a rozeslal zpravu ukoncujici vypocet
					//mam-li reseni, odeslu procesu 0
					//nasledne ukoncim spoji cinnost
					//jestlize se meri cas, nezapomen zavolat koncovou barieru MPI_Barrier (MPI_COMM_WORLD)
					sendNode(_currentBest, INIT_PROC, n, BETTER);
					end();
					break;
				default:
					log("> Received unknown tag: %d", status.MPI_TAG);
					break;
				}
			} //endif(flag)
		}

		Node * node = NULL;

		if (!isInitProc() && hasToken) // InitProc sends token only when going to Idle
		{
			hasToken = false;

			log("< Sending %s token\n", COLORS[sendColor].c_str());
			sendToken(sendColor);
		}

		/***** IDLE STATE ****/
		if (_stack.isEmpty())
		{
			if (_state != IDLE)
			{
				logc("| Stack empty. Going to IDLE state\n");
				_state = IDLE;
			}
			else
			{
#if VERBOSE
				logc("I\n");
#endif
			}

			if (isInitProc() && !initTokenSent)
			{
				logc("< Sending initial WHITE token\n");
				initTokenSent = true;
				sendToken(WHITE);
			}

			// Get moar work
			if (!workRequested)
			{
				log("< Requesting work from %d\n", donor);
				sendWorkReq(donor);
				donor = incDonor(donor);
				workRequested = true;
			}

			checkMsgAmount = 1;

			continue;
		}
		else if (_state != ACTIVE)
		{
			_state = ACTIVE;
			checkMsgAmount = CHECK_MSG_AMOUNT;
			logc("| Coming to ACTIVE state\n");
		}

		/***** Handle the node from stack here: *****/
		node = _stack.pop();

		if (node->hasMaxPrice())
		{
			logc("!!! Found node with max price\n");
			bcastNode(node, BETTER);
			delete _currentBest;
			_currentBest = node;
			end();
		}

		if (node->isBetterThan(_currentBest))
		{
			logc("< Found better solution\n");
			log("%s\n", node->toString().c_str());
			delete _currentBest;
			_currentBest = node;
			//bcastNode(node, BETTER);
		}

		int expanded = expand(node);

		if (node != _currentBest)
		{
			delete node;
		}
#if VERBOSE
		logc("A\n");
#endif
	}
// expanze stavu
}

bool loadSet(char * fname)
{
	ifstream f;
	f.open(fname);

	if (!f.is_open())
	{
		cout << "Cannot open input file: " << fname;
		return false;
	}

	f >> n;
#if !DEBUG
	if(n < 20)
	{
		cout << "n must be 20 or more" << endl;
		f.close();
		return false;
	}
#endif

// limit by number of bytes
// e.g. 1024 / 4 = 256
	int limit = MAX_N / sizeof(int);
	if (n > limit)
	{
		cout << "n must be lower than " << limit << endl;
		f.close();
		return false;
	}

	if ((n / _procCnt) == 0)
	{
		cout << "not enough work for " << _procCnt << " processes";
		f.close();
		return false;
	}

	f >> c;
	f >> a;
#if !DEBUG
	if(a < 2 || a > n / 10)
	{
		cout << "a must be more than 1 and less than n/10" << endl;
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
		cout << "Failed to load input file, exiting" << endl;
		bcastEnd();
		exit(1);
		//end();
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
	char * logDir;
	logDir = getenv("LOG_DIR");
	if (!logDir)
	{
		logDir = "logs";
	}

	sprintf(logFileName, "%s/%d-%d.log", logDir, _procCnt, _thisRank);
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

