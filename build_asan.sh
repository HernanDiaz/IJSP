#!/bin/bash
# Build FuzzyFW_asan with AddressSanitizer
cd /home/diazhernan/IJSP

# Create ASan Makefile
cp Makefile Makefile.asan
# Replace optimization flags with ASan flags
sed -i 's|CXXFLAGS = -O3 -march=native -mtune=native -ffast-math|CXXFLAGS = -g -fsanitize=address -fno-omit-frame-pointer|' Makefile.asan
sed -i 's|LDFLAGS = -lm|LDFLAGS = -fsanitize=address -lm|' Makefile.asan
sed -i 's|EXE = \.\./FuzzyFW|EXE = ../FuzzyFW_asan|' Makefile.asan

# Clean .o and .d files and build
rm -f *.o *.d
make -f Makefile.asan -j4 2>&1 | tail -8
echo "BUILD_EXIT:$?"
ls -la ../FuzzyFW_asan 2>/dev/null
