/* 
** NetXMS - Network Management System
** Performance Data Storage Driver for RRDTools
** Copyright (C) 2014 Raden Solutions
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** File: rrd.cpp
**/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <time.h>

#include <nms_core.h>
#include <pdsdrv.h>

/**
 * Driver class definition
 */
class RRDToolStorageDriver : public PerfDataStorageDriver
{
private:
   static const int PORTNO = 2003;
   static char const *HOSTNAME;
   static int sockfd;

   virtual void error(const char *msg);

public:
   virtual const TCHAR *getName();

   virtual bool saveDCItemValue(DCItem *dcObject, time_t timestamp, const TCHAR *value);
   virtual bool saveDCTableValue(DCTable *dcObject, time_t timestamp, Table *value);
   virtual bool init();
   virtual void shutdown();
};

int RRDToolStorageDriver::sockfd = 0;
const char *RRDToolStorageDriver::HOSTNAME = "graphite.ctco.lv";

/**
 * Driver name
 */
static TCHAR *s_driverName = _T("RRDTOOL");

/**
 * Get name
 */
const TCHAR *RRDToolStorageDriver::getName()
{
   return s_driverName;
}

bool RRDToolStorageDriver::init()
{
   struct sockaddr_in serv_addr;
   struct hostent *server;
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   if (sockfd < 0) 
        error("ERROR opening socket");
   server = gethostbyname(HOSTNAME);
   if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
   }
   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(PORTNO);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    return true;
}

void RRDToolStorageDriver::shutdown()
{
    close(sockfd);
}

/**
 * Save DCI value
 */
bool RRDToolStorageDriver::saveDCItemValue(DCItem *dcObject, time_t timestamp, const TCHAR *value)
{
   char buffer[256];
   char timestr[32];
   int n;
   time_t timer;

   bzero(buffer,256);
   bzero(timestr, 32);

   timer = time(NULL);
   strcpy(buffer, dcObject->getDescription());
   strcat(buffer, "@");
   strcat(buffer, "netxms.ctco.lv");
   strcat(buffer, " ");
   strcat(buffer, value);
   strcat(buffer, " ");
   sprintf(timestr, "%u", timestamp);
   strcat(buffer, timestr);
   perror(buffer);
   strcat(buffer, "\n");
   n = write(sockfd,buffer,strlen(buffer));
   if (n < 0) 
        error("ERROR writing to socket");
//   _tprintf(_T("SAVE: %s %s\n\n"), dcObject->getName(), value);
   return true;
}

/**
 * Save table DCI value
 */
bool RRDToolStorageDriver::saveDCTableValue(DCTable *dcObject, time_t timestamp, Table *value)
{
   _tprintf(_T("SAVE TABLE: %s %d\n\n"), dcObject->getName(), value->getNumRows());
   return true;
}

/**
 * Driver entry point
 */
DECLARE_PDSDRV_ENTRY_POINT(s_driverName, RRDToolStorageDriver);




void RRDToolStorageDriver::error(const char *msg)
{
    perror(msg);
    exit(0);
}
