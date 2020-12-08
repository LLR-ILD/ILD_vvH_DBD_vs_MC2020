#!/bin/bash

OLD_DIR=$PWD
CPP_DIR=$(cd `dirname $0` && pwd)
cd $CPP_DIR

STEERING_PY=$1
if [ "$STEERING_PY" == "" ]; then
    echo "The python steering script's location should be given as parameter."
    STEERING_PY="$CPP_DIR/steerer.py"
    echo "We will try $STEERING_PY."
fi
if ! [ -f $STEERING_PY ]; then
    echo "The specified file does not exist: $STEERING_PY."
    exit
fi

unset MARLIN_DLL
source ./init_ilcsoft.sh
export MARLIN_DLL=$MARLIN_DLL:$CPP_DIR/lib/libvvH-DBD-vs-MC2020.so:

cd build; make install; cd ..
PYTHONPATH=$PYTHONPATH:$CPP_DIR/pySteer
python3 $STEERING_PY
cd $OLD_DIR