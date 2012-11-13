#!/bin/sh
SSH_HOST=vlnasjan@star.fit.cvut.cz

ssh $SSH_HOST <<'ENDSSH'
cd par/logs
less *.log
ENDSSH
