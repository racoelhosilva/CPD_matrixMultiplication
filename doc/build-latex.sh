#!/bin/sh

ROOT=$(dirname $(realpath $0))

function print_usage() {
    cat << EOF
Usage: $0 [options]

Compiles the report.

Options:
    -h --help   Display this help and exit
    -c --clean  Clean build directory and exit
EOF
}

function build() {
    if ! [ -x "$(command -v xelatex)" ]; then
        echo -e "\x1b[31m[fail] xelatex not available, aborting"
        exit 1
    fi
    if ! [ -x "$(command -v bibtex)" ]; then
        echo -e "\x1b[31m[fail] bibtex not available, aborting"
        exit 1
    fi

    cd "$ROOT"
    mkdir -p build
    cp -rf latex/* build

    cd "$ROOT/build"
    xelatex main  # Generate auxiliary files
    bibtex main   # Generate bibliographic references
    xelatex main  # Update auxiliary files
    xelatex main  # Generate final PDF
    if [ $? -ne 0 ]; then
        echo -e "\x1b[31m[fail] an error occurred while compiling the project"
        exit 1
    fi
    
    cd "$ROOT"
    cp build/main.pdf report.pdf
    echo -e "\x1b[36m[info] Report compiled successfully\x1b[0m"
}

function clean() {
    rm -rf "$ROOT/build"
    echo -e "\x1b[36m[info] Build cleaned\x1b[0m"
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
