#!/bin/bash
(
sleep 3
echo "pgtbltest"
sleep 10
printf "\x01"  # Ctrl-A
echo "x"       # Exit QEMU
) | make qemu CPUS=1 2>&1
