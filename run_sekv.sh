#!/bin/sh
SSH_HOST=vlnasjan@star.fit.cvut.cz

ssh $SSH_HOST <<ENDSSH
cd ~/par
export IFILE=$1
make sekv
[ $? -eq 0 ] && qrun.sh 4c 1 serial serial_job.sh
qstat
ENDSSH
