
pick_time()
{
    seq 0.1 0.1 0.9 | while read i; do echo $RANDOM $i; done | \
	sort -n | cut -d' ' -f2 | head -1
}

pick_string()
{
    local len=$1

    cat /dev/urandom  | tr -d -c '[:graph:]' | head -c $len
}

for i in $(seq 1 10); do
    export ID=$i
    (
	while sleep $(pick_time)s; do
	    echo "$ID: $(pick_string $[ $RANDOM % 128 ])" > /proc/ramlog
	do
    ) &
done
