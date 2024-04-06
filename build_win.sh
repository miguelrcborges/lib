if [ -z "$CC" ]
then
	CC="cc"
fi

if [ -z "$CFLAGS" ]
then
	CFLAGS="-O2 -flto -ftree-vectorize -pipe -g"
fi

CFLAGS="$CFLAGS -I. -nostdlib"

mkdir tmp
for f in src/*.c; do
	$CC $CFLAGS -c $f -o "tmp/$(echo $f | sed 's/\//_/g').o" &
done
for f in src/win32/*.c; do
	$CC $CFLAGS -c $f -o "tmp/$(echo $f | sed 's/\//_/g').o" &
done
for f in src/win32/*.S; do
	$CC $CFLAGS -c $f -o "tmp/$(echo $f | sed 's/\//_/g').o" &
done

wait
ar -rcs liblib.a tmp/*.o
rm -r tmp
