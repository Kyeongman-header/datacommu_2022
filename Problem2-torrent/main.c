/*
    NXC Lab Torrent-esque P2P File Sharing System Assignment (2022 Fall) 


    //////////////////////    INTRODUCTION    ///////////////////////////

    In this assignment, you will implement a torrent-esque P2P file-sharing system.
    This system implements a custom network protocol for file sharing.

    Our torrent system partitions a file into blocks and distributes them to peers.
    The torrents are referenced using a hash value of the file in our torrent network.
    Management of the torrent information and data is not the focus of this assignment, and thus 
    it is implemented for you. You can find the details in torrent_functions.h.


    The network protocol is implemented as follows:

    - A peer first requests a torrent info from a seeder using 
    the hash value of the torrent, using REQUEST_TORRENT command.

    - The seeder responds with the torrent info, using PUSH_TORRENT command.
    The requester receives the torrent info, creates a new torrent_file struct and adds it to its global torrent list. (Refer to torrent_functions.h)
    The seeder is also added to the peer list of the new torrent.

    - The requester requests block info of the seeder using REQUEST_BLOCK_INFO command.
    The seeder responds with the block info of the torrent, using PUSH_BLOCK_INFO command.

    - With the block info of the seeder, the requester can now check which block the seeder has.
    The requester then requests its missing blocks from the seeder using REQUEST_BLOCK command.
    The seeder responds with the requested block, using PUSH_BLOCK command.

    - At random intervals, the requester can request a peer list from a random peer(seeder) using REQUEST_PEERS command.
    The random peer responds with its peer list of the torrent, using PUSH_PEERS command.
    This way, the requester can discover new peers of the torrent.

    - The requester can also request block info of the newly discovered peer using REQUEST_BLOCK_INFO command,
    and request missing blocks from those new peers using REQUEST_BLOCK command.

    - Of course, being a P2P system, any requester can also act as a seeder, and any seeder can also act as a requester.

    The requester routines are implemented in client_routines() function, while the seeder routines are implemented in server_routines() function.
    Please refer to the comments in network_functions.h and the code structure of the skeleton function for more details.


    //////////////////////    PROGRAM DETAILS    ///////////////////////////

    To compile: 
    (X86 LINUX)           gcc -o main main.c torrent_functions.o network_functions.o
    (Apple Silicon MAC)   gcc -o main main.c torrent_functions_MAC.o network_functions_MAC.o (Use brew to install gcc on MAC)

    First, test the program by running the following commands in two different terminals:
        ./main 127.0.0.1 1
        ./main 127.0.0.1 2
    They should connect to each other and exchange torrent information and data.

    Run the same commands AFTER compiling the program with silent mode disabled 
    (silent_mode = 0; in main function)
    You should see detailed information about the program's operation.

    These are implemented using TA's model implementations.

    For this assignment, you must implement the following functions and 
    achieve the same functionalities as the TA's implementation...

    - request_torrent_from_peer         (Already implemented for you!)
    - push_torrent_to_peer              (Already implemented for you!)
    - request_peers_from_peer           (Points: 5)
    - push_peers_to_peer                (Points: 5)
    - request_block_info_from_peer      (Points: 5)
    - push_block_info_to_peer           (Points: 5)
    - request_block_from_peer           (Points: 5)
    - push_block_to_peer                (Points: 5)
    - server_routine                    (Points: 40)
    - client_routine                    (Points: 30)

    Total of 100 points.
    Points will be deducted if the functions are not implemented correctly. 
    (Crash/freezing, wrong behavior, etc.)

    Helper functions are provided in network_functions.h and torrent_functions.h, and you are free to use them for your implementation.
    You can of course implement additional functions if you need to.

    TA's model implementations of the above functions are declared in network_functions.h with _ans suffix.
    Use them to test your implementation, but make sure to REMOVE ALL _ans functions from your implementation before submitting your code.

    Reference network_functions.h file to get more information about the functions and the protocol.
    Functions for torrent manipulation are declared & explained in torrent_functions.h.

    Your hand-in file will be tested for its functionalities using the same main function but with your implementation of the above functions.
    (server_routine_ans() and client_routine_ans() in main() will be replaced with your implementation of server_routine() and client_routine() respectively.)

    Your program will be tested with more than 2 peers in the system. You can use ports 12781 to 12790 for testing. 
    Change the port number in the main() function's listen_socket() and request_from_hash() functions.
    listen_socket()'s input port defines the running program's port, 
    while request_from_hash()'s port defines the port of the peer(seeder) which the program requests the torrent from.

    //////////////////////    CLOSING REMARKS    ///////////////////////////

    The program can be tested with multiple machines, and with arbitrary files up to 128MiB. (And can even be used to share files with your friends!) 
    Just input ./main <IP address of the seeder machine> <Peer mode> and replace the corresponding file name/path and hash values in the main function.
    However, being a P2P system, additional port management for firewall and NAT port forwarding to open your listen port to outside connections may be required.
    The details for NAT port forwarding will be covered in the lecture. Google "P2P NAT hole punching" if you are curious about how P2P systems work with NAT.

    PLEASE READ network_functions.h AND torrent_functions.h CAREFULLY BEFORE IMPLEMENTING YOUR CODE.
    ALSO, PLEASE CAREFULLY READ ALL COMMENTS, AS THEY CONTAIN IMPORTANT INFORMATION AND HINTS.

    JS Park.
*/


#include <stdio.h>
#include <time.h> 

#include "torrent_functions.h"
#include "network_functions.h"

#define SLEEP_TIME_MSEC 5000

/////////////////////////// START OF YOUR IMPLEMENTATIONS ///////////////////////////

// Message protocol: "REQUEST_TORRENT [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH]"
int request_torrent_from_peer(char *peer, int port, unsigned int torrent_hash)
{
    if (silent_mode == 0)
        printf ("INFO - COMMAND REQUEST_TORRENT to peer %s:%d, Torrent %x\n"
        , peer, port, torrent_hash);
    int sockfd = connect_socket(peer, port);
    if (sockfd < 0) 
    {
        return -1;
    }
    char buf[STRING_LEN] = {0};
    sprintf(buf, "REQUEST_TORRENT %d %x %x", listen_port, id_hash, torrent_hash);
    send_socket(sockfd, buf, STRING_LEN);
    close_socket(sockfd);
    return 0;
}

// Message protocol: "PUSH_TORRENT [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH]"[TORRENT_INFO]
int push_torrent_to_peer(char *peer, int port, torrent_file *torrent)
{
    if (silent_mode == 0)
        printf ("INFO - COMMAND PUSH_TORRENT to peer %s:%d, Torrent %x\n"
        , peer, port, torrent->hash);
    int sockfd = connect_socket(peer, port);
    if (sockfd < 0) 
    {
        return -1;
    }
    char buf[STRING_LEN] = {0};
    torrent_info info;
    copy_torrent_to_info (torrent, &info);
    sprintf(buf, "PUSH_TORRENT %d %x %x", listen_port, id_hash, info.hash);
    send_socket(sockfd, buf, STRING_LEN);
    int result=send_socket(sockfd, (char *)&info, sizeof(torrent_info));
    
	
//printf("push torrent??????, send??? info ??????. name %s, hash %d, size %d, block_num %d, block_size %d, block info %s\n",info.name,info.hash,info.size,info.block_num,info.block_size,info.block_info);
    close_socket(sockfd);
    return 0;
}

// Message protocol: "REQUEST_PEERS [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH]"
int request_peers_from_peer(char *peer, int port, unsigned int torrent_hash)
{
    if (silent_mode == 0)
        printf ("INFO - COMMAND REQUEST_PEERS to peer %s:%d, Torrent %x \n"
            , peer, port, torrent_hash);
    
    int sockfd = connect_socket(peer, port);
    if (sockfd < 0)
    {
        return -1;
    }
    char buf[STRING_LEN] = {0};
    sprintf(buf, "REQUEST_PEERS %d %x %x", listen_port, id_hash, torrent_hash);
    send_socket(sockfd, buf, STRING_LEN);
    close_socket(sockfd);
    // TODO: Implement (5 Points)
    return 0;
}

// Message protocol: "PUSH_PEERS [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH] [TORRENT_NUM_PEERS]"[PEER_IPS][PEER_PORTS]
int push_peers_to_peer(char *peer, int port, torrent_file *torrent)
{
    if (silent_mode == 0)
        printf ("INFO - COMMAND PUSH_PEERS list to peer %s:%d, Torrent %x \n"
        , peer, port, torrent->hash);
    int sockfd = connect_socket(peer, port);
    if (sockfd < 0)
    {
        return -1;
    }
    char buf[STRING_LEN] = {0};
    char temp_peer_ip[MAX_PEER_NUM][STRING_LEN]; // IP address of each peer, receiving peer??? ???????????? ?????? ????????? ????????? ?????????.
    int temp_peer_port[MAX_PEER_NUM];
    int count=0;
	
	//remove the receiving peer if exists!
	int minus=0;
    for(int i=0;i<torrent->num_peers;i++)
    {	
	if(torrent->peer_port[i]==port && strcmp(torrent->peer_ip[i],peer)==0)
        {
		minus=1;
                continue;
        }
	
	strcpy(temp_peer_ip[count],torrent->peer_ip[i]);
	temp_peer_port[count]=torrent->peer_port[i];
	count=count+1;
    }
    sprintf(buf, "PUSH_PEERS %d %x %x %d", listen_port, id_hash, torrent->hash, (torrent->num_peers) - minus);
    send_socket(sockfd, buf, STRING_LEN);
    send_socket(sockfd, (char *)temp_peer_ip, sizeof(temp_peer_ip));
    send_socket(sockfd, (char *)temp_peer_port, sizeof(temp_peer_port));
    
    close_socket(sockfd);
    // TODO: Implement (5 Points)
    return 0;
}

// Message protocol: "REQUEST_BLOCK_INFO [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH]"
int request_block_info_from_peer(char *peer, int port, unsigned int torrent_hash)
{
    if (silent_mode == 0)
        printf ("INFO - COMMAND REQUEST_BLOCK_INFO to peer %s:%d, Torrent %x\n"
        , peer, port, torrent_hash);
    // TODO: Implement (5 Points)
    int sockfd = connect_socket(peer, port);
    if (sockfd < 0)
    {
        return -1;
    }
    char buf[STRING_LEN] = {0};
    sprintf(buf, "REQUEST_BLOCK_INFO %d %x %x", listen_port, id_hash, torrent_hash);
    send_socket(sockfd, buf, STRING_LEN);
    close_socket(sockfd);
    return 0;
}

// Message protocol: "PUSH_BLOCK_INFO [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH]"[MY_BLOCK_INFO]
int push_block_info_to_peer(char *peer, int port, torrent_file *torrent)
{
    if (silent_mode == 0)
        printf ("INFO - COMMAND PUSH_BLOCK_INFO to peer %s:%d, Torrent %x\n"
        , peer, port, torrent->hash);
    int sockfd = connect_socket(peer, port);
    if (sockfd < 0)
    {
        return -1;
    }
    char buf[STRING_LEN] = {0};


    sprintf(buf, "PUSH_BLOCK_INFO %d %x %x", listen_port, id_hash, torrent->hash);
    send_socket(sockfd, buf, STRING_LEN);
    send_socket(sockfd, (char *)torrent->block_info, sizeof(torrent->block_info));

    close_socket(sockfd);
    // TODO: Implement (5 Points)
    return 0;
}

// Message protocol: "REQUEST_BLOCK [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH] [BLOCK_INDEX]"
int request_block_from_peer(char *peer, int port, torrent_file *torrent, int block_idx)
{
    if (silent_mode == 0)
        printf ("INFO - COMMAND REQUEST_BLOCK %d to peer %s:%d, Torrent %x\n"
       , block_idx, peer, port , torrent->hash);
    int sockfd = connect_socket(peer, port);
    if (sockfd < 0)
    {
        return -1;
    }
    char buf[STRING_LEN] = {0};
    sprintf(buf, "REQUEST_BLOCK %d %x %x %d", listen_port, id_hash, torrent->hash, block_idx);
    send_socket(sockfd, buf, STRING_LEN);
    close_socket(sockfd);
    // TODO: Implement (5 Points)
    return 0;
}

// Message protocol: "PUSH_BLOCK [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH] [BLOCK_INDEX]"[BLOCK_DATA]
int push_block_to_peer(char *peer, int port, torrent_file *torrent, int block_idx)
{
    if (silent_mode == 0)
        printf ("INFO - COMMAND PUSH_BLOCK %d to peer %s:%d, Torrent %x\n"
       , block_idx, peer, port , torrent->hash);
    // TODO: Implement (5 Points)
    
    int sockfd = connect_socket(peer, port);
    if (sockfd < 0)
    {
        return -1;
    }
    char buf[STRING_LEN] = {0};


    sprintf(buf, "PUSH_BLOCK %d %x %x %d", listen_port, id_hash, torrent->hash,block_idx);
    send_socket(sockfd, buf, STRING_LEN);
    send_socket(sockfd, torrent->block_ptrs[block_idx], torrent->block_size);
	// block_ptrs(block_idx)??? ?????? ??? ????????? char * ???.
    close_socket(sockfd);
    // Hint: You can directly use the pointer to the block data for the send buffer of send_socket() call.
    return 0;
}

int server_routine (int sockfd)
{
    struct sockaddr_in client_addr;
    socklen_t slen = sizeof(client_addr);
    int newsockfd;
    while ((newsockfd = accept_socket(sockfd, &client_addr, &slen, max_listen_time_msec)) >= 0)
    {
        char buf[STRING_LEN]={0};                                            // Buffer for receiving commands
        recv_socket(newsockfd, buf, STRING_LEN);                                // Receive data from socket
        
	//printf("?????? buffer : %s\n",buf);
	
	char peer[INET_ADDRSTRLEN];
	//printf("?????? ?????? ??????.\n");                                             // Buffer for saving peer IP address
	inet_ntop( AF_INET, &client_addr.sin_addr, peer, INET_ADDRSTRLEN );     // Convert IP address to string
        if (inet_ntop( AF_INET, &client_addr.sin_addr, peer, INET_ADDRSTRLEN ) == NULL)
      	//printf("?????? inet addr: %s\n", peer);
	{
      	perror("inet_ntop");
      	exit(EXIT_FAILURE);
   	}
	
	if (silent_mode == 0)
            printf ("INFO - SERVER: Received command %s ", buf);
	
	char * End;
	char *name=strtok(buf," \t\n\r");
  	char *_port=strtok(NULL," \t\n\r");//int number???
	char *_id_hash=strtok(NULL," \t\n\r");//?????? 16?????? num
	//printf("com : %s\n",name);
	//printf("port : %s\n",_port);
	//printf("hash : %s\n",_id_hash);
	//char *_torrent_hash=strtok(NULL," \t\n\r"); // 16?????? num
	
	unsigned int port=(unsigned int) strtoul(_port,&End,10);
	unsigned int peer_id_hash=(unsigned int) strtoul(_id_hash,&End,16);
	
	//unsigned int torrent_hash=(unsigned int) strtoul(_torrent_hash,%End, 16);
	//unsigned int torrent_num_peers; //int num
	//unsigned int block_index; //int num
	
	/*
	if(strcmp(name,"PUSH_PEERS")==0){//????????? ??? ???????????????...
		char *_torrent_num_peers=strtok(NULL," \t\n\r");
		torrent_num_peers=(unsigned int) strtoul(_torrent_num_peers, &End,10);
		
	}
	else if(strcmp(name,"REQUEST_BLOCK")==0)
	{
		char *_block_index=strtok(NULL," \t\n\r");
		block_index=(unsigned int) strtoul(_block_index,&End, 10);
		
	}
	else if(strcmp(name,"PUSH_BLOCK")==0)
	{
		char *_block_index=strtok(NULL," \t\n\r")
		block_index=(unsigned int) strtoul(_block_index,&End, 10);
	}
	*/
        // TODO: Parse command (HINT: Use strtok, strcmp, strtol, stroull, etc.) (5 Points)
        char cmd[1024]={0,};
	strcpy(cmd,name);//cmd??? name
        int peer_port=port;
	
        
        if (silent_mode == 0)
            printf ("from peer %s:%d\n", peer, peer_port);

        // TODO: Check if command is sent from myself, and if it is, ignore the message. (HINT: use id_hash) (5 Points)
	if (peer_id_hash==id_hash) //????????? ????????? ????????? ??? ????????? ????????? ???????????? ??????.
	{
		close(newsockfd);
		printf("this is command from myself. close.\n");
		continue;
	}
	//printf("command : %s\n",cmd);
        // Take action based on command.
        // Dont forget to close the socket, and reset the peer_req_num of the peer that have sent the command to zero.
        // If the torrent file for the given hash value is not found in the torrent list, simply ignore the message. (Except for PUSH_TORRENT command)
        // If the torrent file exists, but the message is from an unknown peer, add the peer to the peer list of the torrent.
        if (strcmp(cmd, "REQUEST_TORRENT") == 0) 
        {
            // A peer requests a torrent info!
            // Peer's Message: "REQUEST_TORRENT [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH]"
            // HINT: You might want to use get_torrent(), push_torrent_to_peer(), or add_peer_to_torrent().
            close_socket(newsockfd);
            unsigned int torrent_hash = strtoul(strtok(NULL, " "), NULL, 16);
        
		torrent_file *torrent = get_torrent(torrent_hash);
	
            if (torrent != NULL) 
            {
                push_torrent_to_peer(peer, peer_port, torrent);             // Send torrent to peer (HINT: This opens a new socket to client...)
                //push_torrent_to_peer_ans(peer, peer_port, torrent);
		if (get_peer_idx (torrent, peer, peer_port) < 0) 
                {
                    add_peer_to_torrent(torrent, peer, peer_port, NULL);    
                }
                torrent->peer_req_num [get_peer_idx (torrent, peer, peer_port)] = 0;
            }
        }
        else if (strcmp(cmd, "PUSH_TORRENT") == 0) //?????? ????????????, ?????? ???????????? ???????????? ???????????? ???????????? ???????????? ?????? ??? ??????.
	//????????? ????????? ????????? info??? ???????????????, ????????? global torrent list??? ???????????????,
        {//peer??? ???????????? ?????? ????????? ????????? ????????? ????????? ??????(??? ???????????? ?????? ??????????????? ????????? ??????), ????????? torrent??? ????????? ???????????????.
            // A peer sends a torrent info!
            // Peer's Message: "PUSH_TORRENT [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH]"[TORRENT_INFO]
            // Hint: You might want to use get_torrent(), copy_info_to_torrent(), init_torrent_dynamic_data(), add_torrent(), or add_peer_to_torrent().
            unsigned int torrent_hash = strtoul(strtok(NULL, " "), NULL, 16);
            torrent_file *torrent = get_torrent(torrent_hash);
	    	
            if (torrent == NULL) 
            {
                torrent = (torrent_file *)calloc(1, sizeof(torrent_file));
                torrent_info info;
		
		
                recv_socket(newsockfd, (char *)&info, sizeof(torrent_info));
		
		int result=copy_info_to_torrent(torrent, &info);
		init_torrent_dynamic_data (torrent);
                
                add_torrent(torrent);
                
                add_peer_to_torrent(torrent, peer, peer_port, info.block_info);
		
		torrent->peer_req_num [get_peer_idx (torrent, peer, peer_port)] = 0;
            }
            close_socket(newsockfd);
        }
        // Refer to network_functions.h for more details on what to send and receive.
        else if (strcmp(cmd, "REQUEST_PEERS") == 0) // ??????, ???????????? ?????? ??????????????? ??? ????????? ????????? ????????????. 
        {
		close_socket(newsockfd);
		unsigned int torrent_hash = strtoul(strtok(NULL, " "), NULL, 16);
		torrent_file *torrent = get_torrent(torrent_hash);
            	if (torrent != NULL)
            	{
                	push_peers_to_peer(peer, peer_port, torrent);
                	//push_peers_to_peer_ans(peer, peer_port, torrent);
			if (get_peer_idx (torrent, peer, peer_port) < 0)
                	{
                    	add_peer_to_torrent(torrent, peer, peer_port, NULL);
                	}
                torrent->peer_req_num [get_peer_idx (torrent, peer, peer_port)] = 0;
            	}
            // A peer requests a list of peers!
            // Peer's Message: "REQUEST_PEERS [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH]"
            // Hint: You might want to use  get_torrent(), push_peers_to_peer(), or add_peer_to_torrent().
            // TODO: Implement (5 Points)
        }
        else if (strcmp(cmd, "PUSH_PEERS") == 0) 
        {
		unsigned int torrent_hash = strtoul(strtok(NULL, " "), NULL, 16);
		char *_torrent_num_peers=strtok(NULL," \t\n\r");
                unsigned int torrent_num_peers=(unsigned int) strtoul(_torrent_num_peers, &End,10);
		torrent_file *torrent = get_torrent(torrent_hash);
            if (torrent != NULL)
            {
		printf("????????? ?????? num of peers : %d\n",torrent_num_peers); // ?????? ??????????????? ????????? ????????? 0??????(?????? ?????? ?????? ?????? ????????? ?????????)
		char temp_peer_ip[MAX_PEER_NUM][STRING_LEN]; // IP address of each peer
    		int temp_peer_port[MAX_PEER_NUM];
                recv_socket(newsockfd, (char *)temp_peer_ip, sizeof(temp_peer_ip));
		recv_socket(newsockfd, (char *)temp_peer_port,sizeof(temp_peer_port));
		for(int i=0;i<torrent_num_peers;i++)
		{
			//?????????  peer list??? ?????? ???????????? ?????????, ???????????? ????????? ????????????.
			if(get_peer_idx(torrent,(char *) temp_peer_ip[i],temp_peer_port[i])<0 && temp_peer_ip[i]!=NULL && temp_peer_port[i]!=0)
			{
				add_peer_to_torrent(torrent,temp_peer_ip[i],temp_peer_port[i],NULL);
			}
		}
		if (get_peer_idx (torrent, peer, peer_port) < 0)
                {
                    add_peer_to_torrent(torrent, peer, peer_port, NULL);
                }
                torrent->peer_req_num [get_peer_idx (torrent, peer, peer_port)] = 0;
            }
            close_socket(newsockfd);

            // A peer sends a list of peers!
            // Peer's Message: "PUSH_PEERS [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH] [TORRENT_NUM_PEERS]"[PEER_IPS][PEER_PORTS]
            // Hint: You might want to use get_torrent(), or add_peer_to_torrent().
            //       Dont forget to add the peer who sent the list(), if not already added.
            // TODO: Implement (5 Points)
        }
        else if (strcmp(cmd, "REQUEST_BLOCK_INFO") == 0) 
        {
		close_socket(newsockfd);
		unsigned int torrent_hash = strtoul(strtok(NULL, " "), NULL, 16);
		torrent_file *torrent = get_torrent(torrent_hash);
                if (torrent!= NULL)
                {
                        push_block_info_to_peer(peer, peer_port, torrent);
                        //push_block_info_to_peer_ans(peer, peer_port, torrent);
			if (get_peer_idx (torrent, peer, peer_port) <0)
                        {
                        add_peer_to_torrent(torrent, peer, peer_port, NULL);
                        }
                torrent->peer_req_num [get_peer_idx (torrent, peer, peer_port)] = 0;
                }
            // A peer requests your block info!
            // Peer's Message: "REQUEST_BLOCK_INFO [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH]"
            // Hint: You might want to use  get_torrent(), push_block_info_to_peer(), or add_peer_to_torrent().
            // TODO: Implement (5 Points)
        }
        else if (strcmp(cmd, "PUSH_BLOCK_INFO") == 0)
        {
		unsigned int torrent_hash = strtoul(strtok(NULL, " "), NULL, 16);
		torrent_file *torrent = get_torrent(torrent_hash);
            if (torrent != NULL)
            {   
		char temp_block_info [MAX_BLOCK_NUM];
                recv_socket(newsockfd, (char *)temp_block_info , sizeof(temp_block_info));
                //??????  block info??? ?????? info??? ?????????????????????.
                update_peer_block_info(torrent,peer,peer_port,temp_block_info);
		
		if (get_peer_idx (torrent, peer, peer_port) <0)
                        {
                        add_peer_to_torrent(torrent, peer, peer_port, NULL);
                        }
		
                torrent->peer_req_num [get_peer_idx (torrent, peer, peer_port)] = 0;
            }
            close_socket(newsockfd);
            // A peer sends its block info!
            // Peer's Message: "PUSH_BLOCK_INFO [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH]"[BLOCK_INFO]
            // Hint: You might want to use get_torrent(), update_peer_block_info(), or add_peer_to_torrent().
            // TODO: Implement (5 Points)
        }
        else if (strcmp(cmd, "REQUEST_BLOCK") == 0) 
        {
		close_socket(newsockfd);
		unsigned int torrent_hash = strtoul(strtok(NULL, " "), NULL, 16);
		char *_block_index=strtok(NULL," \t\n\r");
                unsigned int block_index=(unsigned int) strtoul(_block_index,&End, 10);
		
		torrent_file *torrent = get_torrent(torrent_hash);
                if (torrent != NULL)
                {
                        push_block_to_peer(peer, peer_port, torrent,block_index);
                        //push_block_to_peer_ans(peer, peer_port, torrent,block_index);
			
			if (get_peer_idx (torrent, peer, peer_port)<0)
                        {
                        add_peer_to_torrent(torrent, peer, peer_port, NULL);
                        }
                torrent->peer_req_num [get_peer_idx (torrent, peer, peer_port)] = 0;
                }
            // A peer requests a block data!
            // Peer's Message: "REQUEST_BLOCK [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH] [BLOCK_INDEX]"
            // Hint: You might want to use get_torrent(), push_block_to_peer(), or add_peer_to_torrent().
            // TODO: Implement (5 Points)
        }
        else if (strcmp(cmd, "PUSH_BLOCK") == 0) 
        {
		unsigned int torrent_hash = strtoul(strtok(NULL, " "), NULL, 16);
		char *_block_index=strtok(NULL," \t\n\r");
                unsigned int block_index=(unsigned int) strtoul(_block_index,&End, 10);
		torrent_file *torrent = get_torrent(torrent_hash);
		if (torrent != NULL)
            	{
		    //???????????? ?????? ?????? temp_data
                
		
		recv_socket(newsockfd, torrent->block_ptrs[block_index], (torrent->block_size));
		
		//??????, ?????? block??? ???????????? ???????????? ???????????? ?????? block info??? ????????????,
		//??? ?????? ????????? block????????? ??????????????? ???????????? ?????????.
		//downloaded block num??? ???????????????.
		//????????? ??? block????????? ????????? ???????????? ????????? ?????????.
		//printf("?????? ???????????? ?????? ????????? : %d\n",block_index);
		torrent->block_info[block_index]=1;
		torrent->downloaded_block_num++;
		torrent->data=torrent->block_ptrs[0];
		
		
		//data??? block ptrs ?????? ???????????????  ?????? ???????????? ?????? ?????????...? ????????? ????????? ????????????.
		
		
		save_torrent_into_file(torrent,torrent->name);
		
		//for(int j=0;j<torrent->block_num;j++)
                //{
                //              printf("%d ",torrent->block_info[j]);
                //}
		//?? update peer block info??? ??? ???????????? ??? ????????????.
                //peer ??? ????????? block??? ????????? block info??? ?????? ??????????
		//????????? ?????? ????????? ??? block info??? ???????????? ?????????????
		//update_peer_block_info(torrent,peer,peer_port,torrent->peer_block_info[block_index]);
		
		if (get_peer_idx (torrent, peer, peer_port)<0)
                        {
                        add_peer_to_torrent(torrent, peer, peer_port, NULL);
                        }
                
                torrent->peer_req_num [get_peer_idx (torrent, peer, peer_port)] = 0;
            }
            close_socket(newsockfd);
            // A peer sends a block data!
            // Peer's Message: "PUSH_BLOCK [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH] [BLOCK_INDEX]"[BLOCK_DATA]
            // Hint: You might want to use get_torrent(), save_torrent_into_file(), update_peer_block_info(), or add_peer_to_torrent().
            //       You can directly use the pointer to the block data for the receive buffer of recv_socket() call.
            // TODO: Implement (5 Points)
        }
        else
        {
            if (silent_mode == 0)
                printf ("ERROR - SERVER: Received unknown command %s\n", cmd);
            close_socket(newsockfd);
        }
    }
    return 0;
}

int client_routine ()
{
    // Iterate through global torrent list and request missing blocks from peers.
    // Please DO Check network_functions.h for more information and required functions.
    char peer_reqs[MAX_PEER_NUM] = {0};
    //??? peer reqs??? ????????? ??????????????? ?????? ????????????...
    char *peer_list_ip[MAX_PEER_NUM]={0};
    int peer_list_port[MAX_PEER_NUM]={0};
    // ?????? ?????????????????? ?????? ?????? ????????? ????????? ip, port??? ?????????.
    int count=0;
    for (int i = 0; i < num_torrents; i++)
    {
        torrent_file *torrent = global_torrent_list[i];
	//print_torrent_info(torrent);
	if(torrent->downloaded_block_num==torrent->block_num)
	{
		//????????? ?????? ????????? ?????? ????????? ???????????????.(100????????? ????????? ????????????.)
		continue;
	}
	for(int j=0; j<torrent->block_num;j++)
	{
		if(torrent->block_info[j]!=1)
		{//??? ??????????????? ??? ????????? ??????
			for(int k=0; k<torrent->num_peers;k++)
			{//??? ???????????? ?????? ????????? ????????? ????????????
				
				if(torrent->peer_block_info[k][j]==1)
				{//?????? ????????? ??? ????????? ????????? ?????????
					int not_included_flag=1;
					for(int l=0;l<count;l++)
					{//?????? request ??? ?????? ????????? ??? ????????? ????????? ????????????, ?????? ????????? ????????? req ??? ??? ??? req peer ????????? ???????????? break,
					// ????????? break;
						if(strcmp(peer_list_ip[l],torrent->peer_ip[k])==0 && peer_list_port[l]==torrent->peer_port[k])
						{
							not_included_flag=0;
							break;
						}
						else{
						not_included_flag=1;
						}
					}
					if(not_included_flag==1)
					{
						//block request??? ?????????.
						int result=request_block_from_peer(torrent->peer_ip[k],torrent->peer_port[k],torrent,j);
                                         	//int result=request_block_from_peer_ans(torrent->peer_ip[k],torrent->peer_port[k],torrent,j);
						//?????? torrent??? peer_req_num ??????
                                        	
						torrent->peer_req_num [get_peer_idx (torrent, torrent->peer_ip[k], torrent->peer_port[k])] = torrent->peer_req_num [get_peer_idx (torrent, torrent->peer_ip[k], torrent->peer_port[k])]+1;
			
		
                                                //??????! req block??? ???????????????, peer_req_num??? ????????? ????????? ??????, ?????? ????????? ??????????????? ????????????.
                         if(result<0 && torrent->peer_req_num[get_peer_idx(torrent,torrent->peer_ip[k],torrent->peer_port[k])] > PEER_EVICTION_REQ_NUM)
						{//?????? num_peers ????????? ?????? ???????????? ???????????????...
                                                        remove_peer_from_torrent(torrent,torrent->peer_ip[k],torrent->peer_port[k]);
                                          	
						}
						
						peer_list_ip[count]=(char *) malloc(STRING_LEN);
						//????????? req peer list??? ?????? peer??? ip??? port??? ????????????.
						
                                        	strcpy(peer_list_ip[count],torrent->peer_ip[k]);
						
                                        	peer_list_port[count]=torrent->peer_port[k];
						
						count++;
					}
				}
			}
		}
	}
	
	
        // TODO:    Implement block request of the blocks that you don't have, to peers who have. (10 Points)
        //          Make sure that no more than 1 block data requests are sent to a same peer at once.
        // Hint:    Use peer_reqs array to keep track of which peers have been requested for a block.
        //          Use request_block_from_peer() to request a block from a peer. Increment peer_req_num for the peer.
        //          If request_block_from_peer() returns -1, the request failed. 
        //          If the request has failed AND peer_req_num is more than PEER_EVICTION_REQ_NUM, 
        //          evict the peer from the torrent using remove_peer_from_torrent().
	

    }
    // Iterate through global torrent list on "peer_update_interval_msec" and 
    // request block info update from all peers on the selected torrent.
    // Also, select a random peer and request its peer list, to get more peers.
    static unsigned int update_time_msec = 0;
    static unsigned int update_torrent_idx = 0;
    if (update_time_msec == 0)
    {
        update_time_msec = get_time_msec ();
    }
    if (update_time_msec + peer_update_interval_msec < get_time_msec () )
    {
        update_time_msec = get_time_msec ();
        update_torrent_idx++;
        if (update_torrent_idx >= num_torrents)
        {
            update_torrent_idx = 0;
        }
	
	for (int i = 0; i < num_torrents; i++)
    	{
        	torrent_file *torrent = global_torrent_list[i];
		if(torrent->downloaded_block_num==torrent->block_num)
        	{
                //????????? ?????? ????????? ?????? ????????? ???????????????.(100????????? ????????? ????????????.)
                continue;
        	}
        	for(int k=0; k<torrent->num_peers;k++)
                {//??? ???????????? ?????? ????????? ?????????
			int result=request_block_info_from_peer(torrent->peer_ip[k],torrent->peer_port[k],torrent->hash);
			
			//int result=request_block_info_from_peer_ans(torrent->peer_ip[k],torrent->peer_port[k],torrent->hash);//
			torrent->peer_req_num [get_peer_idx (torrent, torrent->peer_ip[k], torrent->peer_port[k])] ++;
                        //??????! req block info??? ???????????????, peer_req_num??? ????????? ????????? ??????, ?????? ????????? ??????????????? ????????????.
                        if(result<0 && torrent->peer_req_num[get_peer_idx (torrent, torrent->peer_ip[k], torrent->peer_port[k])] > PEER_EVICTION_REQ_NUM)
                        {//?????? num_peers ????????? ?????? ???????????? ???????????????...
                        remove_peer_from_torrent(torrent,torrent->peer_ip[k],torrent->peer_port[k]);
                        }
		}
	}
	
        // TODO:    Implement block info update on selected torrent for all peers on the peer list. (10 Points)
        // Hint:    Use request_block_info_from_peer() to request the block info. Increment peer_req_num for the peer.
        //          If request_block_from_peer() returns -1, the request failed. 
	//		?????? ?????? block_info??????
        //          If the request has failed AND peer_req_num is more than PEER_EVICTION_REQ_NUM, 
        //          evict the peer from the torrent using remove_peer_from_torrent().
	for (int i = 0; i < num_torrents; i++)
        {
        torrent_file *torrent = global_torrent_list[i];
	if(torrent->num_peers==0)
	{//?????? request torrent ?????? ??????????????? ??????, peer??? ??? ????????? ?????? ????????????.
		continue;
	}
        int random_idx =  rand() % torrent->num_peers;//?????? ???????????? ?????? ?????? ?????? ????????? ?????? ?????? ??????.
	
                        int result=request_peers_from_peer(torrent->peer_ip[random_idx],torrent->peer_port[random_idx],torrent->hash);
                        //int result=request_peers_from_peer_ans(torrent->peer_ip[random_idx],torrent->peer_port[random_idx],torrent->hash);
			torrent->peer_req_num [get_peer_idx (torrent, torrent->peer_ip[random_idx], torrent->peer_port[random_idx])] ++;
                        //??????! req block info??? ???????????????, peer_req_num??? ????????? ????????? ??????, ?????? ????????? ??????????????? ????????????.
                        if(result<0 && torrent->peer_req_num[get_peer_idx (torrent, torrent->peer_ip[random_idx], torrent->peer_port[random_idx])] > PEER_EVICTION_REQ_NUM)
                        {//?????? num_peers ????????? ?????? ???????????? ???????????????...
                        remove_peer_from_torrent(torrent,torrent->peer_ip[random_idx],torrent->peer_port[random_idx]);
                        }
        }
        // TODO:    Implement peer list request on selected torrent for a random peer on the peer list. (10 Points)
        // Hint:    Use request_peers_from_peer() to request the peer list. Increment peer_req_num for the peer.
        //          If request_block_from_peer() returns -1, the request failed.
	//		?????? ?????? request_peers_from_peer()??????... 
        //          If the request has failed AND peer_req_num is more than PEER_EVICTION_REQ_NUM, 
        //          evict the peer from the torrent using remove_peer_from_torrent().
        
    }
    return 0;
}

/////////////////////////// END OF YOUR IMPLEMENTATIONS ///////////////////////////

int is_ip_valid(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return (result != 0);
}

int main(int argc, char *argv[])
{
    // Input parsing
    // Enter IP of the seeder in the first argument. Select peer mode (1 or 2) in the second argument.
    if (argc != 3) 
    {
        printf ("Invalid number of arguments. Usage: ./peer <SEEDER IP> <MODE 1 or 2>\n");
        return 0;
    }    
    int mode = atoi(argv[2]);
    char *seeder_ip = argv[1];
    if (mode != 1 && mode != 2)
    {
        printf ("Invalid mode. Usage: ./peer <SEEDER IP> <MODE 1 or 2>\n");
        return 0;
    }
    if (is_ip_valid(seeder_ip) == 0)
    {
        printf ("Invalid IP address. Usage: ./peer <SEEDER IP> <MODE 1 or 2>\n");
        return 0;
    }
    if (mode == 1)
    {
        printf ("INFO - Running in peer mode 1. Will connect to seeder %s:%d\n", seeder_ip, DEFAULT_PORT + 1);
    }
    else
    {
        printf ("INFO - Running in peer mode 2. Will connect to seeder %s:%d\n", seeder_ip, DEFAULT_PORT);
    }


    silent_mode = 0; // Set to 0 to enable debug messages.
    unsigned int start_time = 0, counter = 0;

    unsigned int hash_1 = 0x279cf7a5; // Hash for snu_logo_torrent.png
    unsigned int hash_2 = 0x9b7a2926; // Hash for music_torrent.mp3
    unsigned int hash_3 = 0x3dfd2916; // Hash for text_file_torrent.txt
    unsigned int hash_4 = 0x1f77b213; // Hash for NXC_Lab_intro_torrent.pdf

    // Peer 1 - (Port 12781)
    if (mode == 1)
    {
        // Make some files into torrents
        make_file_into_torrent("text_file_torrent.txt", "text_file.txt");           // HASH: 0x3dfd2916
        make_file_into_torrent("NXC_Lab_intro_torrent.pdf", "NXC_Lab_intro.pdf");   // HASH: 0x1f77b213
        // Initialize listening socket
        int sockfd = listen_socket(DEFAULT_PORT);
        if (sockfd < 0) 
        {
            return -1;
        }
        // Wait 5 seconds before starting
        for (int countdown = 5; countdown > 0; countdown--) 
        {
            printf("Starting in %d seconds...\r", countdown);
            fflush(stdout);
            sleep_ms(1000);
        }
        while (1) 
        {
            // Run server & client routines concurrently
            server_routine(sockfd);
		     // Your implementation of server_routine() should be able to replace server_routine_ans() in this line.
            client_routine();           // Your implementation of client_routine() should be able to replace client_routine_ans() in this line.
            
	server_routine(sockfd);     // Your implementation of server_routine() should be able to replace server_routine_ans() in this line.
		
            // Take some action every "SLEEP_TIME_MSEC" milliseconds
            if (start_time == 0 || start_time + SLEEP_TIME_MSEC < get_time_msec())
            {
                if (1 == counter) 
                {
                    // Request torrents from seeder (peer 2), using their hash values
			//??????, ??? ????????????  request_torrent_from_peer ??? ???????????? ??? ??????.
			//?????? ?????? ??????????????? request_torrent_from_peer??? ???????????? ?????????.
                    request_from_hash (hash_1, seeder_ip, DEFAULT_PORT + 1); // Hash for snu_logo_torrent.png
                    request_from_hash (hash_2, seeder_ip, DEFAULT_PORT + 1); // Hash for music_torrent.mp3
                }
                start_time = get_time_msec();
                // print_all_torrents();
                print_torrent_status ();
                counter++;
            }
        }
        close_socket(sockfd);
    }
    // Peer 2 - (Port 12782)
    else if (mode == 2)
    {
        // Make some files into torrents
        make_file_into_torrent("snu_logo_torrent.png", "snu_logo.png");         // HASH: 0x279cf7a5
        make_file_into_torrent("music_torrent.mp3", "music.mp3");               // HASH: 0x9b7a2926 (source: https://www.youtube.com/watch?v=9PRnPdgNhMI)
        // Initialize listening socket
        int sockfd = listen_socket(DEFAULT_PORT + 1);
        if (sockfd < 0) 
        {
            return -1;
        }
        // Wait 5 seconds before starting
        for (int countdown = 5; countdown > 0; countdown--) 
        {
            printf("Starting in %d seconds...\r", countdown);
            fflush(stdout);
            sleep_ms(1000);
        }
        while (1) 
        {
            // Run server & client routines concurrently
            server_routine(sockfd);
                     // Your implementation of server_routine() s$            
		client_routine();           // Your implementation of client_routine() should be abl$                
		
                server_routine(sockfd);     // Your implementation of server_routine() should be abl$                
	
            // Take some action every "SLEEP_TIME_MSEC" milliseconds
            if (start_time == 0 || start_time + SLEEP_TIME_MSEC < get_time_msec())
            {
                if (1 == counter) 
                {
                    // Request torrents from seeder (peer 1), using their hash values
                    request_from_hash (hash_3, seeder_ip, DEFAULT_PORT); // Hash for text_file_torrent.txt
                    request_from_hash (hash_4, seeder_ip, DEFAULT_PORT); // Hash for NXC_Lab_intro_torrent.pdf
                }
                start_time = get_time_msec();
                // print_all_torrents();
                print_torrent_status ();
                counter++;
            }
        }
        close_socket(sockfd);
    }
    return 0;
}
