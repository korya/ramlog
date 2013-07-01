echo -n aaa > /proc/ramlog
echo -n bbb > /proc/ramlog
echo -n ccc > /proc/ramlog

diff -u <(cat /proc/ramlog) <(echo -en 'aaa\nbbb\nccc\n') || err 'Wrong output'

echo -n ddd > /proc/ramlog

diff -u <(cat /proc/ramlog) <(echo -en 'ddd\n') || err 'Wrong output'

echo -n eee > /proc/ramlog
echo -n fff > /proc/ramlog
echo -n ggg > /proc/ramlog

diff -u <(cat /proc/ramlog) <(echo -en 'eee\nfff\nggg\n') || err 'Wrong output'
