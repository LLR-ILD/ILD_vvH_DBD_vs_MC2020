#!/bin/bash

help="""
##############################################################################
#
#  Only run this script from within its folder.
#
##############################################################################
"""

FULL_SCRIPT_DIR=$(cd `dirname $0` && pwd)  # Only changes dir inside the call.
if [ $FULL_SCRIPT_DIR != $PWD ]; then
    echo "The setup script must be called from the folder where it is located."
    echo "Try again from within $FULL_SCRIPT_DIR."
    echo "Script exits without action."
    exit
fi
INITIAL_DIR=$PWD


cpp_setup () {
    INIT_ILCSOFT=/cvmfs/ilc.desy.de/sw/x86_64_gcc49_sl6/v02-00-02/init_ilcsoft.sh

    CPP_DIR=$PWD/make_rootfile
    IN_PROJECT_INIT=$CPP_DIR/init_ilcsoft.sh

    ln -s $INIT_ILCSOFT $IN_PROJECT_INIT
    if ! [ -e $IN_PROJECT_INIT ]; then
        echo "Not a valid link: $IN_PROJECT_INIT."
    fi

    mkdir -p data
    mkdir -p $CPP_DIR/data
    if ! [ -e $CPP_DIR/data/LCFIPlusConfig ]; then
        echo "Setting up the LCFIPlus configuration locally."

        LOCAL_WEIGHTS_DIR=$CPP_DIR/data/LCFIPlusConfig/lcfiweights
        mkdir -p $LOCAL_WEIGHTS_DIR
        cp /cvmfs/ilc.desy.de/sw/ILDConfig/v02-02/LCFIPlusConfig/lcfiweights/*.tar.gz $LOCAL_WEIGHTS_DIR
        cd $LOCAL_WEIGHTS_DIR
        for compressed in $LOCAL_WEIGHTS_DIR/*; do
            EXTRACT_TO=$(basename $compressed .tar.gz)
            mkdir $EXTRACT_TO
            tar -C $EXTRACT_TO -zxvf $compressed
        done
    else
        echo "LCFIPlus configuration seems to exist locally already. Skipped."
    fi

    echo "Prepare the C++ files..."
    cd make_rootfile
    ./build.sh
    if ! [ -d pySteer ]; then
        git clone git@github.com:kunathj/pySteer.git
    fi
    cd ..

    cd $FULL_SCRIPT_DIR
}

cpp_setup