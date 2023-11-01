find . -type f -name "*.o" -exec rm {} +

files=()

while IFS= read -r -d '' file; do
    echo "Compiling file: $file"

    gcc -g -c "$file" -o "${file%.c}.o" -DDEBUG -DUNIX -I ./
    if [ $? -ne 0 ]; then
        echo "Compilation error occurred. Stopping."
        exit 1
    fi
    files+=("${file%.c}.o")
done < <(find . -type f -name "*.c" -print0)

echo "Linking... ${files[@]}"

gcc -o build/bin/main "${files[@]}"
if [ $? -ne 0 ]; then
    echo "Linking error occurred. Stopping."
    exit 1
fi

build/bin/main