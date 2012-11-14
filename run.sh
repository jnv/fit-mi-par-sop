#!/bin/sh
SSH_HOST=vlnasjan@star.fit.cvut.cz

ssh $SSH_HOST <<ENDSSH
cd ~/par
export IFILE=$1
make par
[ $? -eq 0 ] && qrun.sh 4c 8 fast parallel_job.sh
qstat
ENDSSH
