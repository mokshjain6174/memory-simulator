@echo off

if not exist output mkdir output

echo ===== BUDDY TEST =====  > output\all_tests.txt
memsim.exe < test\buddy.txt  >> output\all_tests.txt

echo ===== LINEAR TEST ===== >> output\all_tests.txt
memsim.exe < test\linear.txt >> output\all_tests.txt

echo ===== CACHE TEST =====  >> output\all_tests.txt
memsim.exe < test\cache.txt  >> output\all_tests.txt

echo ===== VM TEST =====     >> output\all_tests.txt
memsim.exe < test\vm.txt     >> output\all_tests.txt

echo All tests completed.
