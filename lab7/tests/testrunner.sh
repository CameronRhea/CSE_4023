#!/bin/bash

echo "===== VALID K0 TESTS ====="
for file in k0/*.kt
do
    echo "Running $file"
    ../k0 "$file"
    echo
done

echo "===== ERROR TESTS ====="
for file in errors/*.kt
do
    echo "Running $file"
    ../k0 "$file"
    echo
done

echo "===== KOTLIN-NOT-K0 TESTS ====="
for file in kotlin/*.kt
do
    echo "Running $file"
    ../k0 "$file"
    echo
done