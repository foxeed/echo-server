### echo-server
small mockup of of client-server environment, supports only a small set of commands

— decided to leave it as it is, with no multithreading
— got somewhat working parsing of arguments, message should not contain any spaces :(

### how to build and use

option one:
1. add it to Qt Creator (or any similar IDE) as CMake project, just open the root CMakeLists.txt
2. build as out-of-source

option two:
1. git clone the repo to, let's say, `~/rep`
2. `mkdir ~/build-rep && ch ~/build-dir`
3. `cmake ../rep -G "Unix Makefiles"`
4. `make`

### how to use
(server and client both support running with parameters `-h` or `--help` to get directions)

— in one shell run `~/build-rep/server/server`
— in other run `~/build-rep/cliend/client`
