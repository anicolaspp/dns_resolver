//
//  ResponseParser.c
//  dnsResolver
//
//  Created by Nicolas A Perez on 3/18/15.
//  Copyright (c) 2015 Nicolas A Perez. All rights reserved.
//

#include "ResponseParser.h"
#include "Others.h"

RESPONSE * ReadResponseFromStream(char * stream)
{
	RESPONSE * response = malloc(sizeof(RESPONSE));
	
	response->header = (struct DNS_HEADER*) stream;
	
	return response;
}
