//
//  ResponseParser.h
//  dnsResolver
//
//  Created by Nicolas A Perez on 3/18/15.
//  Copyright (c) 2015 Nicolas A Perez. All rights reserved.
//



#ifndef dnsResolver_ResponseParser_h
#define dnsResolver_ResponseParser_h

#include "Others.h"

typedef struct _Response
{
	struct DNS_HEADER *header;
	
	struct RES_RECORD Answers[50];
	
} RESPONSE;


RESPONSE * ReadResponseFromStream(char * stream);


#endif
