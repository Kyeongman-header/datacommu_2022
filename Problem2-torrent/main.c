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
    
	
//printf("push torrent에서, send한 info 정보. name %s, hash %d, size %d, block_num %d, block_size %d, block info %s\n",info.name,info.hash,info.size,info.block_num,info.block_size,info.block_info);
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
    char temp_peer_ip[MAX_PEER_NUM][STRING_LEN]; // IP address of each peer, receiving peer를 제외하고 임시 배열을 전송할 것이다.
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
	// block_ptrs(block_idx)는 이미 그 자체로 char * 다.
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
        
	//printf("전체 buffer : %s\n",buf);
	
	char peer[INET_ADDRSTRLEN];
	//printf("서버 루틴 진입.\n");                                             // Buffer for saving peer IP address
	inet_ntop( AF_INET, &client_addr.sin_addr, peer, INET_ADDRSTRLEN );     // Convert IP address to string
        if (inet_ntop( AF_INET, &client_addr.sin_addr, peer, INET_ADDRSTRLEN ) == NULL)
      	//printf("이넷 inet addr: %s\n", peer);
	{
      	perror("inet_ntop");
      	exit(EXIT_FAILURE);
   	}
	
	if (silent_mode == 0)
            printf ("INFO - SERVER: Received command %s ", buf);
	
	char * End;
	char *name=strtok(buf," \t\n\r");
  	char *_port=strtok(NULL," \t\n\r");//int number임
	char *_id_hash=strtok(NULL," \t\n\r");//역시 16진수 num
	//printf("com : %s\n",name);
	//printf("port : %s\n",_port);
	//printf("hash : %s\n",_id_hash);
	//char *_torrent_hash=strtok(NULL," \t\n\r"); // 16진수 num
	
	unsigned int port=(unsigned int) strtoul(_port,&End,10);
	unsigned int peer_id_hash=(unsigned int) strtoul(_id_hash,&End,16);
	
	//unsigned int torrent_hash=(unsigned int) strtoul(_torrent_hash,%End, 16);
	//unsigned int torrent_num_peers; //int num
	//unsigned int block_index; //int num
	
	/*
	if(strcmp(name,"PUSH_PEERS")==0){//밑에서 다 처리해준다...
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
	strcpy(cmd,name);//cmd가 name
        int peer_port=port;
	
        
        if (silent_mode == 0)
            printf ("from peer %s:%d\n", peer, peer_port);

        // TODO: Check if command is sent from myself, and if it is, ignore the message. (HINT: use id_hash) (5 Points)
	if (peer_id_hash==id_hash) //전송된 피어의 해쉬가 내 해쉬랑 같으면 무시하면 된다.
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
        else if (strcmp(cmd, "PUSH_TORRENT") == 0) //얘는 예외인게, 이미 존재하는 토렌트에 대해서는 아무것도 업데이트 해줄 게 없다.
	//여기선 새로운 토렌트 info가 들어왔을때, 그것을 global torrent list에 추가해주고,
        {//peer가 토렌트에 이미 있는지 없는지 확인할 필요도 없이(그 토렌트는 처음 오는거니깐 무조건 없음), 무조건 torrent에 피어를 추가해준다.
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
        else if (strcmp(cmd, "REQUEST_PEERS") == 0) // 매번, 알려지지 않은 피어에게서 온 수신은 피어를 추가한다. 
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
		printf("피어가 보낸 num of peers : %d\n",torrent_num_peers); // 우리 실험에서는 이것은 무조건 0이다(자기 자신 외에 다른 피어가 없어서)
		char temp_peer_ip[MAX_PEER_NUM][STRING_LEN]; // IP address of each peer
    		int temp_peer_port[MAX_PEER_NUM];
                recv_socket(newsockfd, (char *)temp_peer_ip, sizeof(temp_peer_ip));
		recv_socket(newsockfd, (char *)temp_peer_port,sizeof(temp_peer_port));
		for(int i=0;i<torrent_num_peers;i++)
		{
			//보내준  peer list에 대해 토렌트에 없으면, 토렌트에 피어를 추가한다.
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
                //피어  block info를 받은 info로 업데이트해준다.
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
		    //데이터를 받을 그릇 temp_data
                
		
		recv_socket(newsockfd, torrent->block_ptrs[block_index], (torrent->block_size));
		
		//먼저, 해당 block이 다운로드 되었음을 표시하기 위해 block info를 수정하고,
		//그 다음 실제로 block에다가 다운로드된 데이터를 넣는다.
		//downloaded block num도 증가시킨다.
		//그리고 그 block까지가 완성된 토렌트를 파일로 만든다.
		//printf("푸시 블락에서 블록 인덱스 : %d\n",block_index);
		torrent->block_info[block_index]=1;
		torrent->downloaded_block_num++;
		torrent->data=torrent->block_ptrs[0];
		
		
		//data가 block ptrs 처음 가리키는건  설마 구현되어 있지 않을까...? 확실치 않으니 추가한다.
		
		
		save_torrent_into_file(torrent,torrent->name);
		
		//for(int j=0;j<torrent->block_num;j++)
                //{
                //              printf("%d ",torrent->block_info[j]);
                //}
		//?? update peer block info는 왜 필요한지 잘 모르겠다.
                //peer 가 자신의 block을 준거지 block info를 준게 아닌데?
		//수정된 바가 없는데 왜 block info를 업데이트 하는걸까?
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
    //저 peer reqs는 어떻게 활용하라는 건지 모르겠다...
    char *peer_list_ip[MAX_PEER_NUM]={0};
    int peer_list_port[MAX_PEER_NUM]={0};
    // 현재 클라이언트가 하나 이상 전송한 피어의 ip, port의 리스트.
    int count=0;
    for (int i = 0; i < num_torrents; i++)
    {
        torrent_file *torrent = global_torrent_list[i];
	//print_torrent_info(torrent);
	if(torrent->downloaded_block_num==torrent->block_num)
	{
		//이것은 이미 나한테 있는 파일의 토렌트이다.(100퍼센트 다운된 녀석이다.)
		continue;
	}
	for(int j=0; j<torrent->block_num;j++)
	{
		if(torrent->block_info[j]!=1)
		{//내 토렌트에는 그 블록이 없어
			for(int k=0; k<torrent->num_peers;k++)
			{//내 토렌트의 모든 피어에 대해서 검색해봐
				
				if(torrent->peer_block_info[k][j]==1)
				{//해당 피어가 그 블록을 가지고 있다면
					int not_included_flag=1;
					for(int l=0;l<count;l++)
					{//현재 request 된 피어 목록에 그 피어가 있는지 확인하고, 모든 목록에 없으면 req 한 뒤 그 req peer 목록에 추가하고 break,
					// 있으면 break;
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
						//block request를 보낸다.
						int result=request_block_from_peer(torrent->peer_ip[k],torrent->peer_port[k],torrent,j);
                                         	//int result=request_block_from_peer_ans(torrent->peer_ip[k],torrent->peer_port[k],torrent,j);
						//이때 torrent의 peer_req_num 증가
                                        	
						torrent->peer_req_num [get_peer_idx (torrent, torrent->peer_ip[k], torrent->peer_port[k])] = torrent->peer_req_num [get_peer_idx (torrent, torrent->peer_ip[k], torrent->peer_port[k])]+1;
			
		
                                                //만약! req block이 실패했는데, peer_req_num이 한계값 이상일 경우, 해당 피어는 토렌트에서 사라진다.
                         if(result<0 && torrent->peer_req_num[get_peer_idx(torrent,torrent->peer_ip[k],torrent->peer_port[k])] > PEER_EVICTION_REQ_NUM)
						{//설마 num_peers 같은건 해당 함수에서 줄여주겠지...
                                                        remove_peer_from_torrent(torrent,torrent->peer_ip[k],torrent->peer_port[k]);
                                          	
						}
						
						peer_list_ip[count]=(char *) malloc(STRING_LEN);
						//그리고 req peer list에 해당 peer의 ip와 port를 저장한다.
						
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
                //이것은 이미 나한테 있는 파일의 토렌트이다.(100퍼센트 다운된 녀석이다.)
                continue;
        	}
        	for(int k=0; k<torrent->num_peers;k++)
                {//내 토렌트의 모든 피어에 대해서
			int result=request_block_info_from_peer(torrent->peer_ip[k],torrent->peer_port[k],torrent->hash);
			
			//int result=request_block_info_from_peer_ans(torrent->peer_ip[k],torrent->peer_port[k],torrent->hash);//
			torrent->peer_req_num [get_peer_idx (torrent, torrent->peer_ip[k], torrent->peer_port[k])] ++;
                        //만약! req block info가 실패했는데, peer_req_num이 한계값 이상일 경우, 해당 피어는 토렌트에서 사라진다.
                        if(result<0 && torrent->peer_req_num[get_peer_idx (torrent, torrent->peer_ip[k], torrent->peer_port[k])] > PEER_EVICTION_REQ_NUM)
                        {//설마 num_peers 같은건 해당 함수에서 줄여주겠지...
                        remove_peer_from_torrent(torrent,torrent->peer_ip[k],torrent->peer_port[k]);
                        }
		}
	}
	
        // TODO:    Implement block info update on selected torrent for all peers on the peer list. (10 Points)
        // Hint:    Use request_block_info_from_peer() to request the block info. Increment peer_req_num for the peer.
        //          If request_block_from_peer() returns -1, the request failed. 
	//		아마 여기 block_info일듯
        //          If the request has failed AND peer_req_num is more than PEER_EVICTION_REQ_NUM, 
        //          evict the peer from the torrent using remove_peer_from_torrent().
	for (int i = 0; i < num_torrents; i++)
        {
        torrent_file *torrent = global_torrent_list[i];
	if(torrent->num_peers==0)
	{//아직 request torrent 조차 이루어지지 않아, peer가 단 하나도 없는 경우이다.
		continue;
	}
        int random_idx =  rand() % torrent->num_peers;//해당 토렌트가 가진 피어 범위 안에서 랜덤 난수 생성.
	
                        int result=request_peers_from_peer(torrent->peer_ip[random_idx],torrent->peer_port[random_idx],torrent->hash);
                        //int result=request_peers_from_peer_ans(torrent->peer_ip[random_idx],torrent->peer_port[random_idx],torrent->hash);
			torrent->peer_req_num [get_peer_idx (torrent, torrent->peer_ip[random_idx], torrent->peer_port[random_idx])] ++;
                        //만약! req block info가 실패했는데, peer_req_num이 한계값 이상일 경우, 해당 피어는 토렌트에서 사라진다.
                        if(result<0 && torrent->peer_req_num[get_peer_idx (torrent, torrent->peer_ip[random_idx], torrent->peer_port[random_idx])] > PEER_EVICTION_REQ_NUM)
                        {//설마 num_peers 같은건 해당 함수에서 줄여주겠지...
                        remove_peer_from_torrent(torrent,torrent->peer_ip[random_idx],torrent->peer_port[random_idx]);
                        }
        }
        // TODO:    Implement peer list request on selected torrent for a random peer on the peer list. (10 Points)
        // Hint:    Use request_peers_from_peer() to request the peer list. Increment peer_req_num for the peer.
        //          If request_block_from_peer() returns -1, the request failed.
	//		아마 여기 request_peers_from_peer()일듯... 
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
			//아마, 이 함수에서  request_torrent_from_peer 를 호출하는 것 같다.
			//여기 말고 아무데서도 request_torrent_from_peer를 호출하지 않는다.
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
