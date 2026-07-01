# SHA-2048
A sha 2048 hash generator and checksum. The possibility for a duplicate has is 1 in 16^(512). 
Enjoy. you can use in any software. You can also now plug in to.any software. Just **#include** the .h file in your code. 

Building in Termux
---

it's seriously easy just download termux and install git.

```termux code
pkg install git
```
then after that clone it.
```termux code
git clone https://github.com/SHA-2048/SHA-2048.git
```
then go to the directory.
```termux code
cd SHA-2048
```
then compile.
```termux code
pkg install clang
clang++ -O3 -pthread main.cpp -o sha2048
cp sha2048 $PREFIX/bin/
```
Done! You should now be able to use it. 
---
try this out:
```termux code
sha2048 README.md
sha2048 --checksum README.md main.cpp
```



