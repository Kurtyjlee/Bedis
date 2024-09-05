## Bedis
A Redis replica meant for me to learn C++ and Redis internals. 

This project is done in c++17 for unix systems, so best of luck windows :o

## Setup and testing
1. Clone the repository
2. Run the server using `g++ server.cpp utils.cpp eventloop.cpp commands.cpp -o server -std=c++17` then `./server`
3. Run the client using `g++ client.cpp utils.cpp clientUtils.cpp -o client -std=c++17` then `./client`

## Testing with Bash script
Perform regression testing with the bash script
1. `chmod +x test.sh` 
2. `./test.sh`
