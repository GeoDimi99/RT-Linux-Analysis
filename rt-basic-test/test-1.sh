# Test 1
tmux \
    new-session  'echo vboxuser | sudo -S ./task -t 1 -p fifo -r 1 -c green' \; \
    split-window -h 'sleep 3 ; echo vboxuser | sudo -S ./task -t 2 -p fifo -r 1 -c red'

# Test 2
tmux \
    new-session  'echo vboxuser | sudo -S ./task -t 1 -p rr -r 1 -c green' \; \
    split-window -h 'sleep 3 ; echo vboxuser | sudo -S ./task -t 2 -p rr -r 1 -c red'

# Test 3
tmux \
    new-session  'echo vboxuser | sudo -S ./task -t 1 -p rr -r 1 -c green' \; \
    split-window -h 'sleep 3 ; echo vboxuser | sudo -S ./task -t 2 -p rr -r 2 -c red'

# Test 4
tmux \
    new-session  'echo vboxuser | sudo -S ./task -t 1 -p rr -r 1 -c green' \; \
    split-window -h 'sleep 3 ; echo vboxuser | sudo -S ./task -t 2 -p rr -r 1 -c red' \; \
    split-window -h 'sleep 5 ; echo vboxuser | sudo -S ./task -t 3 -p fifo -r 1 -c yellow'


# Test 5
tmux \
    new-session  'echo vboxuser | sudo -S ./task -t 1 -p other -c green' \; \
    split-window -h 'sleep 3 ; echo vboxuser | sudo -S ./task -t 2 -p fifo -r 1 -c red' 