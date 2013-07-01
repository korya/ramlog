#!/bin/bash

# Arguments:
#   $1 -- path to module
#   $2 -- path to test
export MODULE="$1"
export TEST="$2"
export TEST_NAME=$(echo "$TEST" | rev | cut -d/ -f1 | rev)

init()
{
  PATH=/sbin:/usr/sbin:$PATH rt insmod $MODULE output=raw order=4
}

uninit()
{
  PATH=/sbin:/usr/sbin:$PATH rt rmmod $MODULE
}

init
(
  info() { echo "$@" >&2; }
  err() { info "[-] FAILED: $@"; exit 1; }

  info -n "  $TEST_NAME: running...  "
  . "$TEST"
  info "[+] PASSED"
  exit 0
)
rc=$?
uninit

exit $rc
