#!/bin/zsh
typeset -a names
typeset -a mangled

case "$*" in
  ""|-h|--help)
  cat <<EOF
Usage $0 <executable/object file> [<function name pattern>]
EOF
exit 1
;;
esac

if (($# == 2)); then
  filter=(grep "$2")
else
  filter=(grep -v '\(std::_\|\.hidden\|register_tm_clones\|frame_dummy\|_start\|\.text$\|_GLOBAL\|_global_dtors\)')
fi

  objdump -Cj .text -t "$1" \
    | $filter \
    | awk '/^0/ { printf("%s|%s\n", $1, substr($0, 62)); }' \
    | while read i; do
    addr="${i%|*}"
    m="$(objdump -j .text -t "$1"|grep $addr|cut -c62-)"
    echo "$m"|grep -q '^_ZN\?K\?St' && continue
    names+="${i#*|}"
    mangled+="$m"
  done

  if ((${#names} == 1)); then
    selection=1
  else
    for ((i=1; i<=${#names}; ++i)); do
      echo "$i: ${names[i]}" | sed 's/std::experimental::parallelism_v2::/stdx::/g'
    done

    echo -n "Pick a number: "
    read selection
  fi

  if (($selection >= 1 && $selection <= ${#names})); then
    name="${names[$selection]}"
    symbol="${mangled[$selection]}"
  else
    exit 1
  fi
{
  objdump --source-comment -SwC -M intel -j .text --visualize-jumps \
    --no-addresses --no-show-raw-insn --disassemble="$name" $1 \
    | pygmentize -f 256 -l asm -P style=monokai
  echo
  echo " ---------------------------- "
  echo
  llvm-objdump -w -j .text --no-show-raw-insn --disassemble-symbols="${mangled[$selection]}" --no-leading-addr $1 \
    | grep '^ ' \
    | sed 's/<.*>//' \
    | llvm-mca -bottleneck-analysis -output-asm-variant=1 -timeline 2>&1
} | less -R
