# include<stdio.h>
# include<stdlib.h>
# include<unistd.h>
# include<errno.h>
# include<string.h>
# include<netdb.h>
# include<sys/types.h>
# include<netinet/in.h>
# include<sys/socket.h>
# include<arpa/inet.h>

# define PORT "6666"
# define MAXDATASIZE 1000000

void* get_in_addr(struct sockaddr* sa)
{
  if(sa->sa_family==AF_INET)
  {
    return( &(((struct sockaddr_in*)sa)->sin_addr));
  }

  return( &(((struct sockaddr_in6*)sa)->sin6_addr));
}

int main(int argc,char* argv[])
{
  struct addrinfo hints,*servinfo,*p;
  int rv;
  int sockfd,numbytes;
  char buf[MAXDATASIZE];
  char s[INET6_ADDRSTRLEN];

  if(argc!=2)
  {
    fprintf(stderr,"usage:client hostname\n");
    exit(1);
  }

  memset(&hints,0,sizeof(hints));

  hints.ai_family=AF_UNSPEC;
  hints.ai_socktype=SOCK_STREAM;

  if((rv=getaddrinfo(argv[1],PORT,&hints,&servinfo)!=0))
  {
    fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(rv));
    return 1;
  }

  for(p=servinfo;p!=NULL;p=p->ai_next)
  {
    if((sockfd=socket(p->ai_family,p->ai_socktype,p->ai_protocol))==-1)
    {
      perror("client:socket");
      continue;
    }

    if(connect(sockfd,p->ai_addr,p->ai_addrlen)==-1)
    {
      perror("client:connect");
      continue;
    }
    break;
  }

  if(p==NULL)
  {
    fprintf(stderr,"client: failed to connect\n");
    return 2;
  }

  //inet_ntop(p->ai_family,get_in_addr(p->ai_addr),s,sizeof(s));
  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),s, sizeof s);
  printf("Client connecting to: %s\n",s);

  freeaddrinfo(servinfo);




  //while(1)
  //{
    char ss[MAXDATASIZE]="GET http://www.google.com/maps/ HTTP/1.0\r\nAccept-Language: en-us\r\nConnection: Continue\r\n\r\n\0";

    char ch;
    int i=0;
    //gets(s);
    printf("\n length of message sent: %lu",strlen(ss));
    if(send(sockfd,ss,strlen(ss),0)==-1)
    {
      perror("send");
    }
  //}

    char recvv[MAXDATASIZE];
    recv(sockfd,recvv,MAXDATASIZE,0);
    printf("\nClient got: %s",recvv);

  close(sockfd);
  return(0);
}

// cse.iitm.ac.in, google.com, yahoo.com, google maps
