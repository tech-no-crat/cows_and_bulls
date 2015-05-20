# Cows and Bulls
This is a C implementation of the game [Cows and Bulls](http://en.wikipedia.org/wiki/Bulls_and_cows) as a remote game played over a TCP connection. I wrote it to brush up my C, networking and multithreading skills. This program is essentially a multithreaded TCP server with the game implemented on top. To play:

1. Clone the repo.
2. `make && ./server`.
3. From another terminal tab, run `nc localhost 8888`.

