./python/interface.py &> /dev/null &
INTERFACE_PID=$!
echo "SHELL: Starting MUSIC/NEST simulation"
mpirun -n 3 music `readlink -f run.music`
kill -2 $INTERFACE_PID

