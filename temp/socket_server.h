#ifndef SOCKET_SERVER
#define SOCKET_SERVER

#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>

// Socket server
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>	//write
#include <vector>
using namespace std;
//This is a separate thread class running independent of the main imagenet_streaming TensoRT thread

class socket_server
{
public:
    socket_server();

    socket_server(pthread_mutex_t* mut);
    ~socket_server();
    int get_max_image_size(void);
    void set_image_data_size(int);
    int file_error;
    

    // *********** Shared memory **************
    //This data have to be handle with mutex to multithread shared memory
    vector<uint8_t> socket_send;
    vector<int> socket_receive_int;
    int socket_initialized;
   // int nrofheatpsignals;
    // *********** End of shared memory **************
    /// Start the thread
    bool Start(void);

    /// POSIX id (handle) for the thread
    pthread_t threadId_;

    /// Function seen by POSIX as thread function
    static void* ThreadWrapper(void* data); 
    void debug(void);
    private:
  //  vector<uint8_t> socket_receive;
    /// Mutex reference
    pthread_mutex_t *mut_;
    vector<int> payload_local;

    /// This function does the thread work
    void Thread(void);


};
#endif// SOCKET_SERVER
