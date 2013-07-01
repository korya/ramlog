out=$(cat /proc/ramlog)
[ -n "$out" ] && err "ramlog is not empty"
