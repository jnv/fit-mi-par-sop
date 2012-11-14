#!/bin/sh
SSH_HOST=vlnasjan@star.fit.cvut.cz

ssh $SSH_HOST <<'ENDSSH'
cd /mnt/data/$USER
less *.log
ENDSSH
