#include "proxy_parse.h"
  /*struct ParsedRequest* parse=ParsedRequest_create();

  char buf[]="GET http://www.cse.iitm.ac.in:6666/ HTTP/1.0\r\n\r\nHEY: HELLO\r\n\0";
  ParsedRequest_parse(parse,buf,buflen);
  //ParsedHeader_parse()

  printf("%s\n%s\n%s\n%s\n%s\n",parse->method,parse->protocol,parse->host,parse->port,parse->version);

  printf("%lu\n%lu\n",parse->headersused,parse->headerslen);

  return 0;
  */

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
  # include<inttypes.h>
  # define BACKLOG 10
  # define MAXDATASIZE 1000000


  void* get_in_addr(struct sockaddr* sa)
  {
    if(sa->sa_family==AF_INET)
    {
      return(&(((struct sockaddr_in*)sa)->sin_addr));
    }

      return(&(((struct sockaddr_in6*)sa)->sin6_addr));
  }


  int main(int argc,char*argv[])
  {
    char buf[MAXDATASIZE];
    int x=strlen(argv[1]);
    char* PORT=(char*)malloc(x*sizeof(char));
    for(int i=0;i<x;i++)
    {
      PORT[i]=(argv[1])[i];
    }

    for(int i=0;i<x;i++)
    {
      printf("%c",PORT[i]);
    }

    //printf("Initial port: %s\n",PORT);
    printf("\n");


    struct addrinfo hints,*servinfo,*p;
    int rv,yes=1;
    int sockfd,newfd;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];

    memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_UNSPEC;
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_flags=AI_PASSIVE;

    if((rv=getaddrinfo(NULL,PORT,&hints,&servinfo)!=0))
    {
      fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(rv));
      return 1;
    }

    for(p=servinfo;p!=NULL;p=p->ai_next)
    {
      if((sockfd=socket(p->ai_family,p->ai_socktype,p->ai_protocol))==-1)
      {
        perror("server:socket");
        continue;
      }

      if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int))==-1)
      {
        perror("setsockopt");
        continue;
      }

      if(bind(sockfd,p->ai_addr,p->ai_addrlen)==-1)
      {
        close(sockfd);
        perror("server:bind");
        continue;
      }

      break;      //so if we come here, socket(), setsockopt(), bind() work fine.
    }

    freeaddrinfo(servinfo);

    if(p==NULL)
    {
      fprintf(stderr,"server: failed to bind\n");
      exit(1);
    }

    if(listen(sockfd,BACKLOG)==-1)
    {
      perror("listen");
      exit(1);
    }


    printf("server: waiting for connections...\n");

    int count=0; int fd1[2]; //int fd2[2];

    if (pipe(fd1)==-1)
   {
       fprintf(stderr, "Pipe Failed" );
       return 1;
   }

    while(1)
    {
       while(count<3)
       {

         //printf("CLient fork above\n");
        if(!fork())                    //this fork is such that, child updates count; parent takes it by pipe and while loop continues
        {
          count++;                     //inside child of this fork means u r in a client
          close(fd1[0]);
          write(fd1[1],&count,sizeof(count));
          close(fd1[1]);
          sin_size=sizeof(struct sockaddr_storage);
          printf("proxy waiting to accept connection no: %d....\n",count);

          newfd=accept(sockfd,(struct sockaddr*)&(their_addr),&sin_size);

          if(newfd==-1)
          {
            perror("accept");
            continue;
          }


          inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr*)&their_addr),s,sizeof(s));
          printf("Server got connection from: %s\n",s);


          int numbytes;
          char* request; char* headers;
          memset(buf,0,MAXDATASIZE);

          if((numbytes=recv(newfd,buf,MAXDATASIZE-1,0))==-1)   //SO IN CASE OF SERVER; IRRESPECTIVE OF SEND() OR RECV(); USE NEWFD AND NOT SOCKFD
          {
            perror("recvVV ");
            exit(1);
          }

          buf[numbytes]='\0';
          //printf("proxy received: %s\n",buf);

          char* first_3;
          first_3=(char*)malloc(4);
          memcpy(first_3,buf,3);
          first_3[3]='\0';
          //printf("first_3 got: %s\n",first_3);
          if(strcmp(first_3,"GET")!=0)
          {
            send(newfd,"(Error 501) Not implemented\0",28,0);
            exit(0);
          }

          struct ParsedRequest* parse_request=ParsedRequest_create();
          int y=ParsedRequest_parse(parse_request,buf,strlen(buf));
          if(y==-1)
          {
            send(newfd,"(Error 400) Bad Request\0",24,0);
            exit(0);
          }
          //printf("ParsedHeader_parse returning: %d\n",y);

          request=(char*)malloc(sizeof(buf));
          headers=(char*)malloc(sizeof(buf));
          // if(parse_request->port==NULL)
          // {
          //   parse_request->port=(char*)malloc(2);
          //   memcpy(parse_request->port,"80",2);
          // }
          ParsedRequest_unparse(parse_request, request, MAXDATASIZE);

          //## char*bef_hst=strstr(request,parse_request->host);


          //printf("REQUEST GOT: %s\n",request);
          //printf("%s\n%s\n%s\n%s\n%s\n",parse_request->method,parse_request->protocol,parse_request->host,parse_request->port,parse_request->version);

          //printf("%lu\n%lu\n",parse_request->headersused,parse_request->headerslen);

          ParsedRequest_unparse_headers(parse_request, headers,
          				  (size_t)strlen(buf));

          //printf("Headers: %s",headers);

          ///////////
          char string_to_be_sent[MAXDATASIZE];
          char* host_start; char* rel_host_start,*rel_host_start_arr; char*temp,*current;
          host_start=strstr(request,parse_request->host);
          //printf("\nhost_start: %s\n",host_start);

          rel_host_start=host_start+strlen(parse_request->host);

          if(parse_request->port!=NULL)
            rel_host_start+=1+strlen(parse_request->port);

          temp=strstr(rel_host_start," ");
          rel_host_start_arr=(char*)malloc(MAXDATASIZE);
          memcpy(rel_host_start_arr,rel_host_start,temp-rel_host_start);
          rel_host_start_arr[temp-rel_host_start]='\0';

          //printf("rel_host_start_arr: %s\n",rel_host_start_arr);


          memcpy(string_to_be_sent,"GET ",4);
          memcpy(string_to_be_sent+4,rel_host_start_arr,strlen(rel_host_start_arr));
          memcpy(string_to_be_sent+4+strlen(rel_host_start_arr)," ",1);
          memcpy(string_to_be_sent+5+strlen(rel_host_start_arr),parse_request->version,strlen(parse_request->version));
          current=string_to_be_sent+5+strlen(rel_host_start_arr)+strlen(parse_request->version);
          current[0]='\r'; current[1]='\n'; current+=2; //current[0]='\0';
          memcpy(current,"Host: ",6);
          current+=6;
          memcpy(current,parse_request->host,strlen(parse_request->host));
          current+=strlen(parse_request->host);
          current[0]='\r'; current[1]='\n'; current+=2;

          int yy=ParsedHeader_remove(parse_request,"Connection");
          if(yy==0)
          {
            ParsedHeader_set(parse_request,"Connection","close");
          }
          ParsedRequest_unparse_headers(parse_request,current,MAXDATASIZE-(current-string_to_be_sent));
          current+=ParsedHeader_headersLen(parse_request);
          current[0]='\0';
          printf("string_to_be_sent:\n %s\n",string_to_be_sent);

          ///////////        CLIENT FOR FOREIGN SERVER.

          struct addrinfo hints,*servinfo,*p;
          int rv;
          char recvv[MAXDATASIZE];
          int sockfd2;
          //char buf[MAXDATASIZE];
          //char s[INET6_ADDRSTRLEN];

          memset(&hints,0,sizeof(hints));

          hints.ai_family=AF_UNSPEC;
          hints.ai_socktype=SOCK_STREAM;

          char* PORT_serve;
          if(parse_request->port==NULL)
          {
            PORT_serve=(char*)malloc(3);
            memcpy(PORT_serve,"80\0",3);
          }
          else
          {
            PORT_serve=(char*)malloc(strlen(parse_request->port)+1);
            memcpy(PORT_serve,parse_request->port,strlen(parse_request->port));
            PORT_serve[strlen(parse_request->port)]='\0';
          }
          // printf("PORT_serve: %s\n",PORT_serve);
          // printf("HEYYY\n");

          if( (rv=getaddrinfo(parse_request->host,PORT_serve,&hints,&servinfo)) != 0 )
          {
            fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(rv));
            return 1;
          }

          for(p=servinfo;p!=NULL;p=p->ai_next)
          {
            if((sockfd2=socket(p->ai_family,p->ai_socktype,p->ai_protocol))==-1)
            {
              perror("proxy_as_client:socket");
              continue;
            }

            if(connect(sockfd2,p->ai_addr,p->ai_addrlen)==-1)
            {
              perror("proxy_as_client:connect");
              continue;
            }
            break;
          }

          if(p==NULL)
          {
            fprintf(stderr,"proxy_as_client: failed to connect\n");
            return 2;
          }

          //inet_ntop(p->ai_family,get_in_addr(p->ai_addr),s,sizeof(s));
          inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),s, sizeof s);
          printf("Proxy_as_Client connecting to: %s\n",s);

          freeaddrinfo(servinfo);

          if(send(sockfd2,string_to_be_sent,strlen(string_to_be_sent),0)==-1)
          {
            perror("Proxy_as_client send");
          }


          //}
          char* new_current;
          new_current=recvv;
          int loc_count=0;

           while(true)
           {
               numbytes=recv(sockfd2,new_current,MAXDATASIZE-loc_count,0);
              if(numbytes==0)
              {
                //printf("breaking :(((((((((((((((()))))))))))))))\n");
                break;
              }
              new_current=new_current+numbytes;
              //printf("\n\n\n\n@@@@@@@@@@GOT FROM FOREIGN SERVER: %s\n",recvv);
              loc_count+=numbytes;
            }
          recvv[loc_count]='\0';
          ////////////SEND TO CLient
          send(newfd,recvv,loc_count+1,0);

          close(sockfd);

      //}
          exit(0);
          }

        //wait(NULL);
        //close(fd1[1]);
        int xx;

        read(fd1[0],&xx,sizeof(xx));

        count=xx;
        //printf("\nCount got via pipe is: %d\n",xx);

      }
    }
    return(0);

  }
