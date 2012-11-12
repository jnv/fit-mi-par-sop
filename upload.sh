#!/bin/sh
SSH_HOST=vlnasjan@star.fit.cvut.cz #or "username@host"

rsync -rlvze 'ssh' --exclude-from 'rsync-exclude' . $SSH_HOST:~/par
