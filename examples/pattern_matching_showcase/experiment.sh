PYTHON_VERSION="0"

which python3 >> /dev/null
if [ $? == 0 ]; then
    python3 python/test_spore.py >> /dev/null
    if [ $? == 0 ]; then
        PYTHON_VERSION="3"
        echo "SPORE is installed for python 3.x"
    fi
fi

which python >> /dev/null
if [ $? == 0 ] && [ "${PYTHON_VERSION}" == 0 ]; then
    python python/test_spore.py >> /dev/null
    if [ $? == 0 ]; then
        PYTHON_VERSION="2"
        echo "SPORE is installed for python 2.x"
    fi
fi

if [ "${PYTHON_VERSION}" == 0 ] && [ $? == 0 ]; then
    echo "SPORE does not seem to be set up correclty!"
fi

./python/interface.py &> /dev/null &
INTERFACE_PID=$!
echo "SHELL: Starting MUSIC/NEST simulation"
mpirun -n 3 music `readlink -f run.music`
kill -2 $INTERFACE_PID
