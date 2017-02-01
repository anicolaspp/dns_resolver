//
//  dnsResolver
//
//  Created by Nicolas A Perez on 3/18/15.
//  Copyright (c) 2015 Nicolas A Perez. All rights reserved.
//


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "Others.h"
#include "ResponseParser.h"


unsigned char *serverToQuery;
unsigned char *destServer;
unsigned char buf[65536], *qname;

void SetServerToQuery(const char *server)
{
	long len = strlen(server);
	
	if (serverToQuery != 0)
	{
		free(serverToQuery);
	}
	
	serverToQuery = malloc(len * sizeof(char));
	
	strcpy(serverToQuery, server);
}

void SetDestination(const char *dest)
{
	long len = strlen(dest);
	
	if (destServer != 0)
	{
		free(destServer);
	}
	
	destServer = malloc(len * sizeof(char));
	
	strcpy(destServer, dest);
}

struct DNS_HEADER *GetMessageHeader()
{
	struct DNS_HEADER *header = NULL;
	
	header = (struct DNS_HEADER*) & buf;
	
	header->id = (unsigned short) htons(getpid());
	header->qr = 0;
	header->opcode = 0;
	header->aa = 0;
	header->tc = 0;
	header->rd = 1;
	header->ra = 0;
	header->z = 0;
	header->ad = 0;
	header->cd = 0;
	header->rcode = 0;
	header->q_count = htons(1);
	header->ans_count = 0;
	header->auth_count = 0;
	header->add_count = 0;
	
	return header;
}

int GetUpdSocketDecriptor()
{
	int result = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	
	return result;
}

void ChangetoDnsNameFormat(unsigned char* dns,unsigned char* host)
{
	int lock = 0 , i;
	strcat((char*)host,".");
	
	for(i = 0 ; i < strlen((char*)host) ; i++)
	{
		if(host[i]=='.')
		{
			*dns++ = i-lock;
			for(;lock<i;lock++)
			{
				*dns++=host[lock];
			}
			lock++; //or lock=i+1;
		}
	}
	*dns++='\0';
}

void prepareMessage(void)
{
	struct QUESTION *qinfo = NULL;
	
	qname =(unsigned char*)&buf[sizeof(struct DNS_HEADER)];
	
	ChangetoDnsNameFormat(qname, destServer);
	
	qinfo =(struct QUESTION*)&buf[sizeof(struct DNS_HEADER) + (strlen((const char*)qname) + 1)];
	
	qinfo->qtype = htons( T_A );
	qinfo->qclass = htons(1);
}

int SendMessageToServer(struct DNS_HEADER * header, int updSocket)
{
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons(53);
	address.sin_addr.s_addr = inet_addr(serverToQuery);
	
	prepareMessage();
	
	long dataLen = sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION);
	
	int result = 0;
	int sendCounter = 0;
	
	while (result <= 0 && sendCounter++ < 3)
	{
		result = sendto(updSocket, buf, dataLen, 0, (struct sockaddr*)&address,sizeof(address));
	}
	
	return result;
	
}

char * ReadResponseFromServer(int updSocket)
{
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons(53);
	address.sin_addr.s_addr = inet_addr(serverToQuery);
	
	long i = sizeof(address);
	
	int bytesRead = 0;
	int receiveCounter = 0;
	
		//memset(buf, 0, 65536 * sizeof(unsigned char*));
	
	char buf[65536];
	
	memset(buf, 0, sizeof(unsigned char) * 65536);
	
	while (bytesRead <= 0 && receiveCounter++ < 3)
	{
		bytesRead = recvfrom (updSocket, (char*)buf , 65536 , 0 , (struct sockaddr*)&address , (socklen_t*)&i );
	}
	
	
	return buf;
}

u_char* ReadName(unsigned char* reader,unsigned char* buffer,int* count)
	{
		unsigned char *name;
		unsigned int p=0,jumped=0,offset;
		int i , j;
	
		*count = 1;
		name = (unsigned char*)malloc(256);
	
		name[0]='\0';
	
			//read the names in 3www6google3com format
		while(*reader!=0)
		{
			if(*reader>=192)
			{
				offset = (*reader)*256 + *(reader+1) - 49152; //49152 = 11000000 00000000 ;)
				reader = buffer + offset - 1;
				jumped = 1; //we have jumped to another location so counting wont go up!
			}
			else
			{
				name[p++]=*reader;
			}
	
			reader = reader+1;
	
			if(jumped==0)
			{
				*count = *count + 1; //if we havent jumped to another location then we can count up
			}
		}
	
		name[p]='\0'; //string complete
		if(jumped==1)
		{
			*count = *count + 1; //number of steps we actually moved forward in the packet
		}
	
			//now convert 3www6google3com0 to www.google.com
		for(i=0;i<(int)strlen((const char*)name);i++)
		{
			p=name[i];
			for(j=0;j<(int)p;j++)
			{
				name[i]=name[i+1];
				i=i+1;
			}
			name[i]='.';
		}
		name[i-1]='\0'; //remove the last dot
		return name;
	}


int main( int argc , char *argv[])
{
	argc = 3;
	argv = malloc(3 * sizeof(char*));
	argv[0] = "someString";
	argv[1] = "131.94.68.228";
	argv[2] = "cs.fiu.edu";
	
	
	if (argc < 3)
	{
		return 1;
	}
	
	SetServerToQuery(argv[1]);
	SetDestination(argv[2]);
	
	while (1)
	{
		struct DNS_HEADER * header = GetMessageHeader();
		int updSocket = GetUpdSocketDecriptor();
		
		
		int result = SendMessageToServer(header, updSocket);
		
		if (result <= 0)
		{
			printf("Impossible to send data after 3 times. Application will end now.\n");
			return 1;
		}
		
		char * stream = ReadResponseFromServer(updSocket);
		
		if (strlen(stream) <= 0)
		{
			printf("Impossible to read data after 3 times. Application will end now.\n");
			return 1;
		}
		
		header = (struct DNS_HEADER*) stream;
		
		
		printf("\nThe response contains : ");
		printf("\n %d Questions.",ntohs(header->q_count));
		printf("\n %d Answers.",ntohs(header->ans_count));
		printf("\n %d Authoritative Servers.",ntohs(header->auth_count));
		printf("\n %d Additional records.\n\n",ntohs(header->add_count));
		
		
		RESPONSE *response = malloc(sizeof(RESPONSE));
		response->header = header;
		
		unsigned char *reader= &stream[sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION)];
		int stop;
		
		
		for(int i=0;i<ntohs(response->header->ans_count);i++)
					{
						response->Answers[i].name=ReadName(reader,buf,&stop);
						reader = reader + stop;
				
						response->Answers[i].resource = (struct R_DATA*)(reader);
						reader = reader + sizeof(struct R_DATA);
				
						if(ntohs(response->Answers[i].resource->type) == 1) //if its an ipv4 address
						{
							response->Answers[i].rdata = (unsigned char*)malloc(ntohs(response->Answers[i].resource->data_len));
				
							for(int j=0 ; j<ntohs(response->Answers[i].resource->data_len) ; j++)
							{
								response->Answers[i].rdata[j]=reader[j];
							}
				
							response->Answers[i].rdata[ntohs(response->Answers[i].resource->data_len)] = '\0';
				
							reader = reader + ntohs(response->Answers[i].resource->data_len);
						}
						else
						{
							response->Answers[i].rdata = ReadName(reader,buf,&stop);
							reader = reader + stop;
						}
					}
		
	}
	
	
	printf("%s\n", serverToQuery);
	printf("%s\n", destServer);
	
	
 
	return 0;
}