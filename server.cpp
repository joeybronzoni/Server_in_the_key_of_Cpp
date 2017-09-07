#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <iostream>
#include <tuple>
#include <thread>
#include <vector>
#include <memory>

using namespace std;

/********************************************************************************
 * Client
 */

class Client {
public:
  // constructor
  Client() : socket(-1), addr(), len(sizeof(sockaddr_in)) {}

  // data members
  int socket;
  sockaddr_in addr;
  socklen_t len;
};

/********************************************************************************
 * EchoServer
 */

class EchoServer {
public:
  // constructor, kind of a no-op
  EchoServer() {}

  // EchoServer::handle_client =================================================
  static void handle_client(Client client) {
    vector<char> v(1000); // allocate a buffer of 1000 characters
    while (true) { // 'forever' 
      int n_read = read(client.socket, v.data(), v.size()); // read some characters
      int i = 0;
      while (i < n_read) {
	// write out what we read
	int n_write = write(client.socket, v.data() + i, n_read - i);
	i += n_write;
      }
    }
  }
  
  // EchoServer::run =================================================
  void run(unsigned short port) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // make it easier to reuse the same listen port over and over
    int value = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));
    
    auto addr = sockaddr_in();
    
    addr.sin_family	 = AF_INET; // AF_INET => 'address family internet'
    addr.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY => 'don't care which network interface'
    addr.sin_port	 = htons(port); // htons means 'host to network short integer'
    
    bind(server_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)); // bind to port

    vector<shared_ptr<thread>> threads; // storage for threads to handle clients
    while (true) { // i.e. 'forever'
      listen(server_socket, 5); // listen at the server socket and allow up to 5 connects to be queued
      // we woke up! there must be a client connecting. Declare a client object.
      Client client;
      // accept the client connection
      client.socket = accept(server_socket,
			     reinterpret_cast<sockaddr*>(&client.addr), // caller id, so to speak
			     &client.len);
      // create a new thread to handle this specific client
      threads.push_back(shared_ptr<thread>(new thread(handle_client, client)));
      
    }
  }
  
}; /* end class EchoServer */

// main ==============================================================================
int main(int argc, char* argv[])
{
  if (argc != 2) {
    cerr << "usage: " << argv[0] << " <port>" << endl;
    exit(-1);
  }
  EchoServer app; // create the server
  app.run(atoi(argv[1])); // atoi means 'ascii string to integer'
  return 0;
}
