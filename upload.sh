#!/bin/sh
SSH_HOST=vlnasjan@star.fit.cvut.cz #or "username@host"

make clean
rsync -rlvze 'ssh' --delete --exclude-from 'rsync-exclude' . $SSH_HOST:~/par
