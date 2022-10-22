#include <fcntl.h> // 추가.
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>//나는 이 헤더도 추가했다.
#include <unistd.h>


#include "base64.c"

//#define PORT 25000 // You can change port number here
#define PORT 80 // 80번 포트로 그냥 한다.
#define MAX 10000
int respond (int sock);

char usrname[] = "username";
char password[] = "password";
#include <stdint.h>
#include <stdlib.h>
//Problem 1 of project 1:simple webserver with authentification
//Both Problem 1 and 2 was tested on WSL enviroments, Linux, and M1 mac
//But If you still have problems on running codes please mail us
//Most importantly please comment your code

//If you are using mac 
//You can install homebrew here :https://brew.sh
//And open terminal and type 
//sudo brew install gcc
//sudo brew install make
//Type make command to build server
//And server binary will be created
//Use ifconfig command to figure out your ip(usually start with 192. or 172.)
//run server with ./server and open browser and type 192.x.x.x:25000



//If you are using Linux or WSL
//You just need to run "make"(If you are using WSL you may need to install gcc and make with apt)
//And server binary will be created
//Use ifconfig command to figure out your ip(usually start with 192. or 172.)
//run server with ./server and open browser and type 192.x.x.x:25000


//It will be better if you run virtual machine or other device to run server
//But you can also test server with opening terminal and run it on local IP 


int main( int argc, char *argv[] ) {
  int sockfd, newsockfd, portno = PORT;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;
  clilen = sizeof(cli_addr);

  printf("encoding start \n");// We have implemented base64 encoding you just need to use this function
  char *token = base64_encode("2018-10339:mypassword", strlen("2018-10339:mypassword"));//you can change your userid
  printf("encoding end \n");

  //browser will repond with base64 encoded "userid:password" string 
  //You should parse authentification information from http 401 responese and compare it


  /* First call to socket() function */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) {
    perror("ERROR opening socket");
    exit(1);
  }

  // port reusable
  int tr = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &tr, sizeof(int)) == -1) {
    perror("setsockopt");
    exit(1);
  }
  
  /* Initialize socket structure */
  memset(&serv_addr,0x00,sizeof(serv_addr));
  serv_addr.sin_family=AF_INET;
  serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
  serv_addr.sin_port=htons(portno);


  /* TODO : Now bind the host address using bind() call. 10% of score*/
    //it was mostly same as tutorial
  if(bind(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))==-1)
  {
	  perror("ERROR binding socket");
	  exit(1);
  }
  /* TODO : listen on socket you created  10% of score*/
  if(listen(sockfd,10)==-1){
  	perror("ERROR listening");
	exit(1);
  }


  printf("Server is running on port %d\n", portno);
    
    //it was mostly same as tutorial
    //in the while loop every time request comes we respond with respond function if valid

    //TODO: authentication loop 40 % of score
  char buf[MAX];  
  while(1){
      newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
      
      memset(buf,0x00,MAX);
      //TODO: accept connection
      if(newsockfd ==-1){
      	perror("accept error");
	exit(1);
      }
      
      if(read(newsockfd,buf,1024)<=0)
      {
	      return -1;
      }

      printf("\n----$$----\nthis is recevied client header : \n");
      printf("%s\n",buf);

      char *temp=strtok(buf,"\n");
      char *temp2=strtok(NULL,"\n");
      char *temp3=strtok(NULL,"\n");
      char *temp4=strtok(NULL,"\n");
      //char *target_line=strtok(NULL,"\n");
      char *AUTHORIZATION=strtok(NULL," \t\n\r");
      char *BASIC=strtok(NULL," \t\n\r");
      char *TOKEN=strtok(NULL," \t\n\r");

      printf("\n and here is parsed header : \n");
      //printf("first line : %s\n",temp);
      //printf("second line : %s\n",temp2);
      //printf("third line : %s\n",temp3);
      //printf("forth line : %s\n",temp4);
      //printf("target line : %s\n",target_line);
      printf("and then word => %s ( it should be 'Authorization:'.) \n",AUTHORIZATION);
      printf("and then word => %s ( it should be 'Basic'.) \n",BASIC);
      printf("and then word => %s ( it should be the token.) \n",TOKEN);
      printf("parsing ends.\n");
      
      if(strcmp(AUTHORIZATION,"Authorization:")!=0){
      	char message[]="HTTP/1.1 401 Unauthorized\nWWW-Authenticate: Basic realm=\"Access to the staging site\"";
      	int length=strlen(message);
      	int bytes=0;
      	while(length>0){
        	printf("send_bytes : %d\n",bytes);
        	bytes=send(newsockfd,message,length,0);
        	length=length-bytes;
      	}
	shutdown(newsockfd,SHUT_RDWR);
        close(newsockfd);
	//여기선 닫아주면 안될 지도???
	continue;
      }

      
      if (strcmp(TOKEN,token)==0){
      	printf("authenticate success. file transfer.\n");
	/*char message[]="HTTP/1.1 200 OK";
        int length=strlen(message);
        int bytes=0;
        while(length>0){
                printf("send_bytes : %d\n",bytes);
                bytes=send(newsockfd,message,length,0);
                length=length-bytes;
        }
	*/
	int readn=0;
        int fd=open("./index.html",O_RDONLY);
        memset(buf,0x00,MAX);
        while((readn=read(fd,buf,1024))>0)
                {
                        write(newsockfd,buf,readn);
                }
        close(fd);
      	shutdown(newsockfd,SHUT_RDWR);
      	close(newsockfd);
	//continue;
	break; // 다음 루프로 가려면 이렇게 break해 줘야 하지만, 이러면 코드가 말이 안된다.
      }
      else{
	printf("authenticate fail. close\n");
	char message[]="HTTP/1.1 401 Unauthorized\nWWW-Authenticate: Basic realm=\"Access to the staging site\"";
        int length=strlen(message);
        int bytes=0;
        while(length>0){
                printf("send_bytes : %d\n",bytes);
                bytes=send(newsockfd,message,length,0);
                length=length-bytes;
        }
       shutdown(newsockfd,SHUT_RDWR);
       close(newsockfd);
       //닫아주면 안될 지도?
        continue;
      }

      //TODO: send 401 message(more information about 401 message : https://developer.mozilla.org/en-US/docs/Web/HTTP/Authentication) and authentificate user
      //close connection


    }
    //Respond loop
    //사실 이렇게 두개의 loop로 나누는 방법은 의미가 없다. chrome을 비롯한 대부분의 브라우져는 쿠키에 authenticate 정보를 저장한다.
	//따라서 처음 auth success를 하면 그 뒤부터는 auth를 거치지 않고 해당 클라이언트는 respond 함수로 갈 수 있다.
	
	while (1) {
	
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if ( newsockfd == -1 ){
          perror("accept error");
          exit(1);
        }
	
	
	respond(newsockfd);
	
	
	printf("get out of respond func.\n");
	//auth_flag=0;//이후로는 auth flag가 0이기 때문에 이 loop에서 새로운 연결을 기다린다.
      }
	

  return 0;
}
//TODO: complete respond function 40% of score
int respond(int sock) {
  char filename[MAX];
  char buf[MAX];
  memset(buf,0x00,MAX);
  memset(filename,0x00,MAX);
  printf("enter respond func\n");
  
  if(read(sock,buf,MAX)<=0)
  {
	      perror("read error");
              return -1;
  }
  //printf("\n this is received client.(inside respond func)\n");
  //printf("%s\n",buf);

  char *temp=strtok(buf," \t\n\r");
  char *name=strtok(NULL," \t\n\r");
  printf("this is method : %s\n",temp);
  printf("this is url(filename) : %s\n",name);
  char add_dot[1024]=".";
  strcat(add_dot,name);
  strcpy(filename,add_dot);
  printf("real filename : %s\n",filename);
  // 파일명은 request header에서 method 다음에 두번째로 온다.
  // 근데 이 파일명은 ./~~이렇게 되있어야 제대로 인식한다. 혹은, /이게 없거나.

  char message[MAX];
  memset(message,0x00,MAX);
  printf("is file exists ? : %d\n",access(filename,R_OK));
  if(access(filename,R_OK)!=0 && strcmp(filename,"/")!=0)
  {
	sprintf(message,"HTTP/1.1 404 Not Found");
	int length=strlen(message);
  	int bytes=0;
  	while(length>0){
      		printf("send_bytes : %d\n",bytes);
      		bytes=send(sock,message,length,0);
      		length=length-bytes;
  	}
  }
  else if(strcmp(filename,"./")==0)
  {
	int readn=0;
        int fd=open("./index.html",O_RDONLY);
        memset(buf,0x00,MAX);
        while((readn=read(fd,buf,1024))>0)
        {
                        write(sock,buf,readn);
        }
        close(fd);
  }
  else{
	char* extension=strrchr(filename,'.');
	printf("extension is : %s\n",extension);
	if(strcmp(extension,".html")==0 || strcmp(extension,".js")==0 || strcmp(extension,".css")==0)
	{
		int readn=0;
  		int fd=open(filename,O_RDONLY);
		memset(buf,0x00,MAX);
		while((readn=read(fd,buf,1024))>0)
		{
			write(sock,buf,readn);
		}
		close(fd);
	}
	else if(strcmp(extension,".jpg")==0 || strcmp(extension,".png")==0)
	{
		int read=0;
		FILE * picture=fopen(filename,"r");
		if (picture == NULL) {
    			perror("Picture open error");
    			exit(1);
  		}
		memset(buf,0x00,MAX);
		while(!feof(picture))
		{
			read = fread(buf, 1, sizeof(buf)-1, picture);
          		write(sock, buf, read);
		}
		fclose(picture);
	}

  }
  
 
  printf("close\n");
  shutdown(sock,SHUT_RDWR);
  close(sock);

    
  return 0;
}

