#!/bin/sh
# default emacs friendly colorful autumn murphy manni monokai perldoc pastie borland trac native fruity bw vim vs tango rrt xcode igor paraiso-light paraiso-dark lovelace algol algol_nu arduino rainbow_dash abap

usage() {
  cat <<EOF
Usage: $0 [options] <source>

Options:
    -h, --help     This message.
    -d, --direct   Print asm output of GCC instead of objdump disasm.
    -m, --mangled  Show mangled names.

The CXX and CXXFLAGS environment variables are used for compiling the <source>.

SPDX-License-Identifier: GPL-3.0-or-later
Copyright © 2022-2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
                      Matthias Kretz <m.kretz@gsi.de>
EOF
}

demangle=true
direct=false
while [ $# -gt 0 ]; do
  case "$1" in
    -h|--help)
      usage()
      exit
      ;;
    -d|--direct)
      direct=true
      ;;
    -m|--mangled)
      demangle=false
      ;;
    *)
      break
      ;;
  esac
  shift
done

if [ $BACKGROUND = "light" ]; then
  style=colorful
else
  style=monokai
fi

if $direct; then
  mycxxfilt() {
    if $demangle; then
      c++filt
    else
      cat
    fi
  }

  $CXX -S -masm=intel -O2 -std=gnu++2b -march=native $CXXFLAGS -o - "$@" \
    | grep -v '^\.LF' \
    | grep -v '^\s\+\.\(file\|intel\|text\|p2align\|globl\|type\|cfi_\|size\|section\|align\|ident\)' \
    | pygmentize -f 256 -l asm -P style=$style \
    | sed 's/m_Z/m### _Z/g' \
    | mycxxfilt \
    | sed 's/m### /m/g'

  exit
fi

if $demangle; then
  demangle=C
else
  demangle=
fi

file=$(mktemp).o
$CXX -c -O2 -std=gnu++2b -march=native $CXXFLAGS -o $file "$@"
objdump $file -Sw$demangle -M intel -j .text --visualize-jumps \
  --no-addresses --no-show-raw-insn -d \
  | sed 1,5d \
  | pygmentize -f 256 -l asm -P style=$style
rm $file
