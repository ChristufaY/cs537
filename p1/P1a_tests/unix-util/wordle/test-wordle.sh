#! /bin/bash

if ! [[ -x wordle ]]; then
    echo "wordle executable does not exist"
    exit 1
fi

../../tester/run-tests.sh $*
echo done
