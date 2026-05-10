_myprog_impl() {
  local cur="$1" prev="$2"

  case "$prev" in
  --o2 | --opt-two)
    # shellcheck disable=SC2207
    COMPREPLY=($(compgen -W "key\!7 key\"2 key#8 key\$3 key&4 key'1 key(9 key)0 key*6 key;2 key?1 key[5 key]6 key\`5 key{7 key|3 key}8 key~4" -- "$cur"))
    return 0
    ;;
  --o1 | --opt-one)
    # shellcheck disable=SC2207
    COMPREPLY=($(compgen -f -- "$cur"))
    return 0
    ;;
  --o3 | --opt-three)
    # shellcheck disable=SC2207
    COMPREPLY=($(compgen -f -- "$cur"))
    return 0
    ;;
  esac

  if [[ "$cur" == --*=* ]]; then
    local opt="${cur%%=*}"
    local val="${cur#*=}"
    case "$opt" in
    --o2)
      # shellcheck disable=SC2207
      COMPREPLY=($(compgen -W "key\!7 key\"2 key#8 key\$3 key&4 key'1 key(9 key)0 key*6 key;2 key?1 key[5 key]6 key\`5 key{7 key|3 key}8 key~4" -- "$val"))
      return 0
      ;;
    --opt-two)
      # shellcheck disable=SC2207
      COMPREPLY=($(compgen -W "key\!7 key\"2 key#8 key\$3 key&4 key'1 key(9 key)0 key*6 key;2 key?1 key[5 key]6 key\`5 key{7 key|3 key}8 key~4" -- "$val"))
      return 0
      ;;
    esac
  fi

  if [[ "$cur" == -* ]]; then
    local opts="--f1 --flag-one --f2 --flag-two --no-f2 --no-flag-two --o1 --opt-one --o2 --opt-two --o3 --opt-three"
    # shellcheck disable=SC2207
    COMPREPLY=($(compgen -W "$opts" -- "$cur"))
    return 0
  fi

  return 0
}

_myprog() {
  local cur prev cmd i
  cur="${COMP_WORDS[COMP_CWORD]}"
  prev="${COMP_WORDS[COMP_CWORD - 1]}"
  cmd=""
  for ((i = 1; i < COMP_CWORD; i++)); do
    case "${COMP_WORDS[$i]}" in
    -*)
      continue
      ;;
    esac
  done

  if [[ -n "$cmd" ]]; then
    "$cmd" "$cur" "$prev"
  else
    _myprog_impl "$cur" "$prev"
  fi
}

complete -F _myprog myprog
