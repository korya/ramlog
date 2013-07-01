echo -n aaa > /proc/ramlog
echo -n bbb > /proc/ramlog
echo -n ccc > /proc/ramlog
echo -n ddd > /proc/ramlog
echo -n eee > /proc/ramlog

diff -u <(cat /proc/ramlog) <(echo -en 'bbb\nccc\nddd\neee\n') || err 'Wrong output'
