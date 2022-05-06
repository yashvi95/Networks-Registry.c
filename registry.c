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
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_PORT "5432"
#define MAX_PENDING 5
#define MAX_FILES 10
#define MAX_FILENAME_LEN 255



/*
 * Create, bind and passive open a socket on a local interface for the provided service.
 * Argument matches the second argument to getaddrinfo(3).
 *
 * Returns a passively opened socket or -1 on error. Caller is responsible for calling
 * accept and closing the socket.
 */

struct peer_entry {
    uint32_t id;                             // ID of peer
    int socket_descriptor;                   // Socket descriptor for connection to peer
    char files[MAX_FILES][MAX_FILENAME_LEN]; // Files published by peer
    struct sockaddr_in address;              // Contains IP address and port number
	uint32_t numFile;
};


int bind_and_listen(const char *service);

int main(int argc, char *argv[]) {

    
	struct peer_entry Peers[5];
	int currPeers = 0;
	//int returnval = 0;
	//listening socket
	int s;
	fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number
	socklen_t addrlen;
	struct sockaddr_storage remoteaddr; // client address
	int lenofmessage = 0; 
	//uint32_t buf[3];

	/* Bind socket to local interface and passive open */
	if ((s = bind_and_listen(SERVER_PORT) ) < 0 ) {
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

    	read_fds = master;	// copy master list for select() check
		if ( select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1 ) {	//get list of available sockets
            perror("select");
            exit(1);
        }
         
      	// run through the existing connections looking for data to read
        for ( int i = 0; i <= fdmax; i++ ) {

			if (FD_ISSET(i, &read_fds)) { // fd at i is ready to read

                if (i == s) {	// if a new connection is trying to be made on our primary listening socket (new peer)
					// handle new peer connection
					addrlen = sizeof(&remoteaddr);
					//Accepting a connection
					int newSocket;
					if ((newSocket = accept( s, (struct sockaddr *)&remoteaddr, &addrlen) ) < 0 ) {
							perror( "stream-talk-server: accept" );	//error accepting new connection
							close(s);
							exit(1);

					} else {	// if accept successful
						FD_SET((newSocket), &master); // add to master set
						if ((newSocket) > fdmax) {    // keep track of the max
							fdmax = (newSocket);
						}
					}
					//}
				} else {	// other peer is sending data through it's connection socket

					uint8_t dataBuff[1200];
					//convert to host byte order

					if ((lenofmessage = recv( i, dataBuff, sizeof(dataBuff), 0 )) > 0 ) {

						if (dataBuff[0] == 0) {	// JOIN request
							//could check if Peers is full first and refuse connection if so
							struct peer_entry newPeer;
							uint8_t id = ntohl(dataBuff[1]); 
							memcpy(&newPeer.id, &id, sizeof(uint8_t));
							newPeer.socket_descriptor = i;
							newPeer.numFile= 0;
							//struct sockaddr_in addr;
							socklen_t len = sizeof(newPeer.address);
							int ret = getpeername(i, (struct sockaddr*)&newPeer.address, &len);
							if(ret == -1){
								printf("error");
							}

							Peers[currPeers] = newPeer;
							currPeers++;
							printf("TEST] JOIN %d\n", htons(newPeer.address.sin_port));

     					} else if ( dataBuff[0] == 1 ) {	// PUBLISH request
						// comes in as Network order

							//find which peer is connected to socket i
							int peerIndex;
							for (peerIndex= 0; peerIndex < currPeers; peerIndex++) {
								if ( Peers[peerIndex].socket_descriptor == i ) {
									peerIndex = i;
									break;
								}
							}
							uint32_t files;
							memcpy(&files, &dataBuff[1], sizeof(uint32_t));
							uint32_t numFiles = ntohl(files);

							Peers[peerIndex].numFile = numFiles;
							uint8_t pointer = 5;
							uint8_t counter = pointer;
						
							printf("TEST] PUBLISH %u ", Peers[peerIndex].numFile);
							for ( uint32_t i= 0; i< numFiles; i++) {

								while ( dataBuff[counter] != '\0') {
									counter++;
								}
								memcpy(&(Peers[peerIndex].files[i][0]), &dataBuff[pointer], ((sizeof(uint8_t))*counter+1));
								pointer = counter+1;
								printf("%s ", &(Peers[peerIndex].files[i][0]));
							}
							printf("\n");


						} else if (dataBuff[0] == 2) {// SEARCH request
						// response must by in Network byte order, print locally in Host byte order
						    //find which peer is connected to socket i
							int peerIndex;
							for (peerIndex= 0; peerIndex < currPeers; peerIndex++) {
								if ( Peers[peerIndex].socket_descriptor == i ) {
									peerIndex = i;
									break;
								}
							}
							uint8_t counter = 1;

							while (dataBuff[counter] != '\0') {
									counter++;
								}
							char fileName[counter+1];
							memcpy(&fileName, &dataBuff[1], (sizeof(uint8_t))*counter+1);
							printf("Searching for   %s   \n", fileName);


							printf("looking for peer %d \n", currPeers);
							
							for (int j = 0; j < currPeers; j++) {
								printf("looking for peer %d \n", j);

								uint32_t number = 0;

								if ((Peers[j].socket_descriptor != i) && (Peers[j].numFile != number)) {
									
									int index = Peers[j].numFile;
									
									printf("looking for numfiles %u \n", Peers[j].numFile);
									
									for (int k = 0; k < index; k++) {
										printf("Looking through files %u \n", k);

										//not sure if it's actually comparing

										if (strncmp(fileName, (Peers[j].files[k]), sizeof(fileName) == 0)){
											printf("found");
											//just setup the send message, 4bytes, 4bytes, 2bytes
											//not sure what the first argument in the send should be
											uint8_t mesgpeer[10];
											memcpy(&mesgpeer[0],&Peers[j].id,4);
											memcpy(&mesgpeer[3],&Peers[j].address.sin_addr,4);
											memcpy(&mesgpeer[7],&Peers[j].address.sin_port,2);

											send(Peers[peerIndex].socket_descriptor,mesgpeer,sizeof(mesgpeer),0);

											
										} 
									
										else {
											printf("not found");
										}
									}
										

							      }

								 else{
									 printf("not correct peer \n");
								 }
								
							}

						//strcmp to files in peer struct


					} else {	// error or no data received, close connection
						perror( "streak-talk-server: recv" );
						close( i );
						FD_CLR(i, &master); // remove from master set
						//remove from Peer[] set
						exit( 1 );
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
