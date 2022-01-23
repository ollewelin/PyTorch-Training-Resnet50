#include "socket_server.h"
#include <vector>
#include <iostream>

#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
//#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IMAGE_DATA_MAX_SIZE 3000000
#define SOCKET_IMG_DATA_SIZE 1000
#define SEND_HEADER_size 10

#define SEND_HEADER_INDX_NR_OF_BYTES_THIS_PACKAGE 0
#define SEND_HEADER_INDX_NR_OF_PACKAGES_TOTAL 1
#define SEND_HEADER_INDX_SEND_PACKAGE 2
#define SEND_HEADER_INDX_IMAGE_CLASS 3
#define SEND_HEADER_INDX_spare4 4
#define SEND_HEADER_INDX_spare5 5
#define SEND_HEADER_INDX_spare6 6
#define SEND_HEADER_INDX_spare7 7
#define SEND_HEADER_INDX_spare8 8
#define SEND_HEADER_INDX_spare9 9

union int_u8_union
{
    int int_data;
    uint8_t uint8t_data[sizeof(int_data)];
};

int socket_server::get_max_image_size(void)
{
    return IMAGE_DATA_MAX_SIZE;
};
#define REPLY_DATA_SIZE 20

void socket_server::set_image_data_size(int set_size)
{

    if (set_size > 0 && set_size <= IMAGE_DATA_MAX_SIZE)
    {
        printf("Image data set_size is is set to = %d\n", set_size);
    }
    else
    {
        printf("ERORR! set_size = %d was outside range\n", set_size);
        set_size = IMAGE_DATA_MAX_SIZE;
        printf("set_size is forced to MAX = %d\n", set_size);
    }
    socket_send.clear();
    for (int i = 0; i < set_size; i++)
    {
        socket_send.push_back(0);
    }
    printf("Size of socket_send =%d\n", (int)socket_send.size());
};
socket_server::~socket_server()
{
    pthread_cancel(threadId_);
    printf("socket_server is deleted.\n");
};
socket_server::socket_server(pthread_mutex_t *mut)
    : mut_(mut)

{
    printf("Constructor socket_server thread class \n");
    socket_send.clear();
    socket_receive_int.clear();

    file_error = 0;
    socket_initialized = 0;
};

/// Function seen by POSIX as thread function
void *socket_server::ThreadWrapper(void *data)
{
    // data is a pointer to a Thread, so we can call the
    // real thread function
    (static_cast<socket_server *>(data))->Thread(); //Here is where the magic happend to connect this thread object wrapper to the tempsens::Thread(void) funtion below
    return 0;
}

int readable_timeo(int fd, int sec)
{
    fd_set rset;
    struct timeval tvrd;

    FD_ZERO(&rset);
    FD_SET(fd, &rset);

    tvrd.tv_sec = sec;
    tvrd.tv_usec = 0;

    return (select(fd + 1, &rset, NULL, NULL, &tvrd));
    /* > 0 if descriptor is readable */
}
int writeable_timeo(int fd, int sec)
{
    fd_set rset;
    struct timeval tvwr;

    FD_ZERO(&rset);
    FD_SET(fd, &rset);

    tvwr.tv_sec = sec;
    tvwr.tv_usec = 0;

    return (select(fd + 1, &rset, NULL, NULL, &tvwr));
    /* > 0 if descriptor is readable */
}

void socket_server::Thread(void)
{
    union int_u8_union union_d_int_u8;
    int sended_bytes = 0;
    int received_bytes = 0;

    int nr_of_rec_ints = REPLY_DATA_SIZE / sizeof(union_d_int_u8.int_data);

    //

    const int img_socket_port = 2300;
    int socket_desc, client_sock, c, read_size;
    struct sockaddr_in server, client;

    uint8_t client_message[SOCKET_IMG_DATA_SIZE]; //1Mpixel

    printf("socket_server thread program started\n");
    file_error = 0;
    pthread_mutex_lock(mut_);
    socket_initialized = 0;
    pthread_mutex_unlock(mut_);

    uint8_t reply_message[REPLY_DATA_SIZE];

    for (int i = 0; i < REPLY_DATA_SIZE; i++)
    {
        // socket_receive.push_back(0);
        reply_message[i] = 0;
    }
    socket_receive_int.clear();
    for (int i = 0; i < nr_of_rec_ints; i++)
    {
        socket_receive_int.push_back(0);
    }

    //Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(img_socket_port);

    //Bind
    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        //	return 1;
    }
    puts("bind done");

    //Listen
    listen(socket_desc, 3);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    //accept connection from an incoming client
    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c);
    if (client_sock < 0)
    {
        perror("accept failed");
        //	return 1;
    }
    puts("Connection accepted");

    for (int i = 0; i < 10; i++)
    {
        client_message[i] = i + 100;
    }
    int wr_bytes = 0;
    int rd_bytes = 0;
    int socket_send_size = 0;
    while (1)
    {
        printf("Thread loop\n");
        //   int error = 0;
        //   int retval = 0;
        //   socklen_t len = sizeof (error);
        //   retval = getsockopt(client_sock, SOL_SOCKET, SO_ERROR, &error, &len);
        int check_rd_timeout = readable_timeo(client_sock, 2);
        if (check_rd_timeout > 0)
        {
            rd_bytes = read(client_sock, reply_message, REPLY_DATA_SIZE);
        }
        else
        {
            if (check_rd_timeout == 0)
            {
                printf("Recieve socket tiemout ====  rd timeout   ============================= \n");
            }
            if (check_rd_timeout == -1)
            {
                printf("Recieve socket tiemout E ====  rd timeout ERROR  ============================= \n");
            }
        }

        pthread_mutex_lock(mut_);
        for (int i = 0; i < nr_of_rec_ints; i++)
        {
            int size_int = sizeof(union_d_int_u8.int_data);
            for (int k = 0; k < size_int; k++)
            {
                union_d_int_u8.uint8t_data[k] = reply_message[(i * size_int) + k];
            }
            socket_receive_int[(i / size_int)] = union_d_int_u8.int_data;
        }
        pthread_mutex_unlock(mut_);

        for (int i = 0; i < nr_of_rec_ints; i++)
        {
            printf("socket_receive_int[%d] = %d\n", i, socket_receive_int[i]);
        }
        pthread_mutex_lock(mut_);
        socket_send_size = socket_send.size();
        if (socket_send_size >= SOCKET_IMG_DATA_SIZE)
        {
            for (int i = 0; i < SOCKET_IMG_DATA_SIZE; i++)
            {
                //TODO
                client_message[i] = socket_send[i];
            }
        }
        pthread_mutex_unlock(mut_);
        //int check_wr_timeout = writeable_timeo(client_sock, 10);
        wr_bytes = write(client_sock, client_message, SOCKET_IMG_DATA_SIZE);

        /*
       //  check_wr_timeout = 1;
        if( check_wr_timeout > 0){

        wr_bytes = write(client_sock , client_message , SOCKET_IMG_DATA_SIZE);
        }else{
            if(check_wr_timeout == 0){
                printf("Send socket tiemout ====  wr timeout  ----------------------------------- \n");
            }
            if(check_wr_timeout == -1){
                printf("Send socket tiemout E ====  wr timeout ERROR  -----------------------------\n");
            }
        }*/

        printf("Send x bytes. wr_bytes =%d\n", wr_bytes);

        //usleep(100000);//Sleep inside this thread.
    }
    close(client_sock);
}
void socket_server::debug(void)
{

}

bool socket_server::Start(void)
{
    // Start the thread, send it the this pointer (points to this class
    // instance)

    //pthread_create() takes 4 arguments.
    //The first argument is a pointer to thread_id which is set by this function.
    //The second argument specifies attributes. If the value is NULL, then default attributes shall be used.
    //The third argument is name of function to be executed for the thread to be created.
    //The fourth argument is used to pass arguments to the function.
    printf("Create and start the socket_server threa\n");
    return (pthread_create(&threadId_,    // Pointer to the thread handle
                           NULL,          // Optional ptr to thread settings
                           ThreadWrapper, // Thread function
                           this) == 0);   // Argument passed to thread func
}
