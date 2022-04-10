#! /bin/bash

if ! [[ -x my-look ]]; then
    echo "mylook executable does not exist"
    exit 1
fi
#see ../../tester/run-tests.sh -h for running specific tests
../../tester/run-tests.sh $*
echo done
