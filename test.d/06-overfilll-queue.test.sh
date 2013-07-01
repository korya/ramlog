echo -n aaa > /proc/ramlog # aaa
echo -n bbb > /proc/ramlog # aaa - bbb
echo -n ccc > /proc/ramlog # aaa - ccc
echo -n ddd > /proc/ramlog # aaa - ddd
echo -n eee > /proc/ramlog # bbb - eee
echo -n fff > /proc/ramlog # ccc - fff
echo -n ggg > /proc/ramlog # ddd - ggg
echo -n hhh > /proc/ramlog # eee - hhh

diff -u <(cat /proc/ramlog) <(echo -en 'eee\nfff\nggg\nhhh\n') || err 'Wrong output'
