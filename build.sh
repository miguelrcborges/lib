if [ -z "$CC" ]
then
	CC="clang"
fi

if [ -z "$CFLAGS" ]
then
	CFLAGS="-O2 -flto -s -D_FORTIFY_SOURCE=1"
fi
CFLAGS="$CFLAGS -ffreestanding -nostlib -c -I."
