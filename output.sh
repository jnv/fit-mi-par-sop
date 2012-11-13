#!/bin/sh
SSH_HOST=vlnasjan@star.fit.cvut.cz

ssh $SSH_HOST <<'ENDSSH'
cd ~/par
for f in parallel_job.sh.*
do
  echo "$f ================================"
  cat $f
done
ENDSSH
