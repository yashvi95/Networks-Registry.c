//Networks-Prog4 Yashvi Siddhapura & Reema Shahid

/* This code is an updated version of the sample code from "Computer Networks: A Systems
 * Approach," 5th Edition by Larry L. Peterson and Bruce S. Davis. Some code comes from
 * man pages, mostly getaddrinfo(3). */
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SERVER_PORT "5432"
#define MAX_PENDING 5
#define MAX_FILES 10
#define MAX_FILENAME_LEN 255


struct peer_entry {
    uint32_t id;                             // ID of peer
    int socket_descriptor;                   // Socket descriptor for connection to peer
    char files[MAX_FILES][MAX_FILENAME_LEN]; // Files published by peer
    struct sockaddr_in address;              // Contains IP address and port number
};


/*
 * Create, bind and passive open a socket on a local interface for the provided service.
 * Argument matches the second argument to getaddrinfo(3).
 *
 * Returns a passively opened socket or -1 on error. Caller is responsible for calling
 * accept and closing the socket.
 */

int bind_and_listen(const char *service);

int main(int argc, char *argv[]) {

    struct peer_entry peer;
	//int returnval = 0;
	//listening socket
	int s;
	fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number
	socklen_t addrlen;
	struct sockaddr_storage remoteaddr; // client address
	int lenofmessage = 0; 
	uint32_t buf[3];

	/* Bind socket to local interface and passive open */
	if ((s = bind_and_listen( SERVER_PORT) ) < 0 ){
		exit( 1 );
	}

  // clear the master and temp sets
   FD_ZERO(&master); 
   FD_ZERO(&read_fds);
  
   //add s to filedescriptor set
   FD_SET(s, &master);
   //keep track of biggest file descriptor
   fdmax = s;

	/* Wait for connection, then receive and print text */
	while ( 1 ) {

       read_fds = master;
		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(1);
        }
         
      // run through the existing connections looking for data to read
        for(int i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == s) {
				 // handle new connections
				 addrlen = sizeof(&remoteaddr);

    
				//Accepting a connection
				if ((peer.socket_descriptor = accept( s, (struct sockaddr *)&remoteaddr, &addrlen) ) < 0 ) {
					perror( "stream-talk-server: accept" );
					close(s);
					exit(1);
                 }
				 else{
					 FD_SET((peer.socket_descriptor), &master); // add to master set
                        if ((peer.socket_descriptor) > fdmax) {    // keep track of the max
                            fdmax = (peer.socket_descriptor);
                        }
						//have to extract from new connection

				 }
				}
				else{
                  //have to recv data
					while ((lenofmessage = recv(i, buf, sizeof(buf), 0 )) ) {
						     //connection is closed
							if ( lenofmessage < 0 ) {
								perror( "streak-talk-server: recv" );
								close( s );
								FD_CLR(i, &master); // remove from master set
								exit( 1 );
								}
							else{
								if(lenofmessage == 5)
								if(buf[0] == 0){
									printf("JOIN succesful");
									uint32_t id = ntohl(buf[1]); 
									memcpy(&peer.id, &id, sizeof(uint32_t));
								}

						// 		// we got some data from a client
                        //         for(int j = 0; j <= fdmax; j++) {
                        //         // send to everyone!
                        //         if (FD_ISSET(j, &master)) {
                        //         // except the listener and ourselves
                        //         if (j != s && j != i) {
                        //             if (send(j, buf, lenofmessage, 0) == -1){
						// 			 perror("send");
						// 		}

						// 		}
						// 	}
						// }
					}
				}
			}
		}
	}	     
}

return 0;
 
 }



















int bind_and_listen( const char *service ) {
	struct addrinfo hints;
	struct addrinfo *rp, *result;
	int s;

	/* Build address data structure */
	memset( &hints, 0, sizeof( struct addrinfo ) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;

	/* Get local address info */
	if ( ( s = getaddrinfo( NULL, service, &hints, &result ) ) != 0 ) {
		fprintf( stderr, "stream-talk-server: getaddrinfo: %s\n", gai_strerror( s ) );
		return -1;
	}

	/* Iterate through the address list and try to perform passive open */
	for ( rp = result; rp != NULL; rp = rp->ai_next ) {
		if ( ( s = socket( rp->ai_family, rp->ai_socktype, rp->ai_protocol ) ) == -1 ) {
			continue;
		}

		if ( !bind( s, rp->ai_addr, rp->ai_addrlen ) ) {
			break;
		}

		close( s );
	}
	if ( rp == NULL ) {
		perror( "stream-talk-server: bind" );
		return -1;
	}
	if ( listen( s, MAX_PENDING ) == -1 ) {
		perror( "stream-talk-server: listen" );
		close( s );
		return -1;
	}
	freeaddrinfo( result );

	return s;

}
