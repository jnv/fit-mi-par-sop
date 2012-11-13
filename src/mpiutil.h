#include "mpi.h"
#include "Node.h"

#ifndef MPIUTIL_H_
#define MPIUTIL_H_

#define INIT_PROC 0

extern int _thisRank;
extern int _procCnt;

enum Tag
{
	END, INT, BETTER, WORK_IN, WORK_REQ, WORK_NONE, INIT_ARR
};

void sendNode(Node * node, int dest, Tag tag)
{

}

void bcastEnd()
{

}

void bcastInt(int in, Tag tag)
{
	for(int i = 0; i < _procCnt; i++)
	{
		if(_thisRank == i)
			continue;
		MPI_Send(&in, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
	}
}

#endif // MPIUTIL_H_
