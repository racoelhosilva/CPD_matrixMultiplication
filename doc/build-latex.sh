#!/bin/sh

ROOT=$(dirname "$(realpath "$0")")

print_usage() {
    cat << EOF
Usage: $0 [options]

Compiles the report.

Options:
    -h --help   Display this help and exit
    -c --clean  Clean build directory and exit
EOF
}

build() {
    if ! [ -x "$(command -v xelatex)" ]; then
        printf "\033[31m[fail] xelatex not available, aborting\033[0m\n"
        exit 1
    fi
    if ! [ -x "$(command -v bibtex)" ]; then
        printf "\033[31m[fail] bibtex not available, aborting\033[0m\n"
        exit 1
    fi

    cd "$ROOT" || exit
    mkdir -p build
    cp -rf latex/* build

    cd "$ROOT/build" || exit
    xelatex -interaction=nonstopmode main  # Generate auxiliary files
    bibtex main                            # Generate bibliographic references
    xelatex -interaction=nonstopmode main  # Update auxiliary files
    xelatex -interaction=nonstopmode main  # Generate final PDF

    if [ $? -ne 0 ]; then
        printf "\033[31m[fail] an error occurred while compiling the project\033[0m\n"
        exit 1
    fi

    cd "$ROOT" || exit
    cp build/main.pdf report.pdf
    printf "\033[36m[info] Report compiled successfully\033[0m\n"
}

clean() {
    rm -rf "$ROOT/build"
    printf "\033[36m[info] Build cleaned\033[0m\n"
}

ARGS=$(getopt -o 'hc' --long 'help,clean' -n "$0" -- "$@" 2> /dev/null)
if [ $? -ne 0 ]; then
    print_usage
    exit 1
fi

eval set -- "$ARGS"

while true; do
    case "$1" in
        -h | --help)
            print_usage
            exit 0
            ;;
        -c | --clean)
            clean
            exit 0
            ;;
        --)
            shift
            break
            ;;
        *)
            print_usage
            exit 1
    esac
done

build
