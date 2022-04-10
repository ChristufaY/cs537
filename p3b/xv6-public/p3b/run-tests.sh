#!/bin/bash

/p/course/cs537-swift/tests/tester/run-tests.sh -d /p/course/cs537-swift/tests/p3b $*
make -f Makefile.test clean
