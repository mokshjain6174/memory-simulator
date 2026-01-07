#!/bin/bash

mkdir -p output
OUT=output/all_tests.txt

echo "===== BUDDY TEST ====="  >  $OUT
./memsim.exe < test/buddy.txt  >> $OUT

echo "===== LINEAR TEST =====" >> $OUT
./memsim.exe < test/linear.txt >> $OUT

echo "===== CACHE TEST ====="  >> $OUT
./memsim.exe < test/cache.txt  >> $OUT

echo "===== VM TEST ====="     >> $OUT
./memsim.exe < test/vm.txt     >> $OUT

echo "All tests completed."
