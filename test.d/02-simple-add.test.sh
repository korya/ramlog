echo -n aaa > /proc/ramlog
echo -n bbb > /proc/ramlog
echo -n ccc > /proc/ramlog

diff -u <(cat /proc/ramlog) <(echo -en 'aaa\nbbb\nccc\n') || err 'Wrong output'
