gcc lib/fileops_noposix.c -fPIC -shared -o libnoposix.so
cp libnoposix.so /usr/lib/
