assert_equal()
{
    local buf="$1"
    local msg="$2"

    diff -u <(cat /proc/ramlog) <(echo -en "$buf") || err "Wrong output: $msg"
}

echo -n aaa > /proc/ramlog # aaa
assert_equal 'aaa\n' 'aaa'

echo -n aaa > /proc/ramlog # aaa
echo -n bbb > /proc/ramlog # aaa - bbb
assert_equal 'aaa\nbbb\n' 'aaa - bbb'

echo -n aaa > /proc/ramlog # aaa
echo -n bbb > /proc/ramlog # aaa - bbb
echo -n ccc > /proc/ramlog # aaa - ccc
assert_equal 'aaa\nbbb\nccc\n' 'aaa - ccc'

echo -n aaa > /proc/ramlog # aaa
echo -n bbb > /proc/ramlog # aaa - bbb
echo -n ccc > /proc/ramlog # aaa - ccc
echo -n ddd > /proc/ramlog # aaa - ddd
assert_equal 'aaa\nbbb\nccc\nddd\n' 'aaa - ddd'

echo -n aaa > /proc/ramlog # aaa
echo -n bbb > /proc/ramlog # aaa - bbb
echo -n ccc > /proc/ramlog # aaa - ccc
echo -n ddd > /proc/ramlog # aaa - ddd
echo -n eee > /proc/ramlog # bbb - eee
assert_equal 'bbb\nccc\nddd\neee\n' 'bbb - eee'

echo -n aaa > /proc/ramlog # aaa
echo -n bbb > /proc/ramlog # aaa - bbb
echo -n ccc > /proc/ramlog # aaa - ccc
echo -n ddd > /proc/ramlog # aaa - ddd
echo -n eee > /proc/ramlog # bbb - eee
echo -n fff > /proc/ramlog # ccc - fff
assert_equal 'ccc\nddd\neee\nfff\n' 'ccc - fff'

echo -n aaa > /proc/ramlog # aaa
echo -n bbb > /proc/ramlog # aaa - bbb
echo -n ccc > /proc/ramlog # aaa - ccc
echo -n ddd > /proc/ramlog # aaa - ddd
echo -n eee > /proc/ramlog # bbb - eee
echo -n fff > /proc/ramlog # ccc - fff
echo -n ggg > /proc/ramlog # ddd - ggg
assert_equal 'ddd\neee\nfff\nggg\n' 'ddd - ggg'

echo -n aaa > /proc/ramlog # aaa
echo -n bbb > /proc/ramlog # aaa - bbb
echo -n ccc > /proc/ramlog # aaa - ccc
echo -n ddd > /proc/ramlog # aaa - ddd
echo -n eee > /proc/ramlog # bbb - eee
echo -n fff > /proc/ramlog # ccc - fff
echo -n ggg > /proc/ramlog # ddd - ggg
echo -n hhh > /proc/ramlog # eee - hhh
assert_equal 'eee\nfff\nggg\nhhh\n' 'eee - hhh'
