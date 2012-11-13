#include "mpi.h"
#include "Node.h"
#include "common.h"

#ifndef MPIUTIL_H_
#define MPIUTIL_H_

#define INIT_PROC 0

extern int _thisRank;
extern int _procCnt;

enum Tag
{
	END,
	INT,
	BETTER,
	WORK_IN,
	WORK_REQ,
	WORK_NONE,
	INIT_ARR,
	INIT_SET,
	INIT_PACK,
	NODE
};

void sendNode(Node * node, int dest, int size, Tag tag = NODE)
{
	MPI_Send(node->placement, size, MPI_INT, dest, tag, MPI_COMM_WORLD);
}

void sendInt(int in, int dst, Tag tag)
{
	MPI_Send(&in, 1, MPI_INT, dst, tag, MPI_COMM_WORLD );
}


void bcastInt(int in, Tag tag)
{
	for (int i = 0; i < _procCnt; i++)
	{
		if (i == _thisRank)
			continue;
		sendInt(in, i, END);
	}
}

void bcastEnd()
{
	bcastInt(1, END);
}

bool probeEnd()
{
	int flag;
	MPI_Status status;
	MPI_Iprobe(0, END, MPI_COMM_WORLD, &flag, &status);

	if (flag)
		return true;
	return false;
}

#endif // MPIUTIL_H_
