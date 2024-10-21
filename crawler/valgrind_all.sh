echo
echo "Checking tests"
FILES=*.c

for f in $FILES; do
    echo
    echo "Checking test: ${f%.c}"
    valgrind --leak-check=full "${f%.c}"
done;

