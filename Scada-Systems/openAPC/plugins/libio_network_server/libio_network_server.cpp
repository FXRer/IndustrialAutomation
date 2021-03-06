/********************************************************************************************

These sources are distributed in the hope that they will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. All
the information given here, within the interface descriptions and within the specification
are subject to change without notice. Errors and omissions excepted.

These sources demonstrate the usage of the OpenAPC Plug-In programming interface. They can be
used freely according to the OpenAPC Dual License: As long as the sources and the resulting
applications/libraries/Plug-Ins are used together with the OpenAPC software, they are
licensed as freeware. When you use them outside the OpenAPC software they are licensed under
the terms of the GNU General Public License.

For more information please refer to http://www.openapc.com/oapc_license.php

*********************************************************************************************/



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifndef ENV_WINDOWSCE
#include <sys/types.h>
#endif

#ifndef ENV_WINDOWS
 #include <arpa/inet.h>
 #include <errno.h>
 #include <unistd.h>
 #include <fcntl.h>
 #define closesocket close
#else
 #include <windows.h>
 typedef int socklen_t;
 #define snprintf _snprintf
#endif

#define MAX_XML_SIZE         4000

#include "oapc_libio.h"
#include "network_common.h"
#include "liboapc.h"



/** the xml data that define the behaviour and user interface of the plugin within the main application */
static char xmltempl[]="<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n\
<oapc-config>\n\
 <flowimage>%s</flowimage>\n\
 <dialogue>\n\
  <general>\n\
   <param>\n\
    <name>ip</name>\n\
    <text>Local IP</text>\n\
    <type>string</type>\n\
    <default>%s</default>\n\
    <min>3</min>\n\
    <max>%d</max>\n\
   </param>\n\
   <param>\n\
    <name>port</name>\n\
    <text>Port</text>\n\
    <type>integer</type>\n\
    <default>%d</default>\n\
    <min>1</min>\n\
    <max>65535</max>\n\
   </param>\n\
   <param>\n\
    <name>incoming</name>\n\
    <text>Incoming Connections</text>\n\
    <type>integer</type>\n\
    <default>%d</default>\n\
    <min>1</min>\n\
    <max>%d</max>\n\
   </param>\n\
   <param>\n\
    <name>uname</name>\n\
    <text>Username</text>\n\
    <type>string</type>\n\
    <default>%s</default>\n\
    <min>0</min>\n\
    <max>%d</max>\n\
   </param>\n\
   <param>\n\
    <name>pwd</name>\n\
    <text>Password</text>\n\
    <type>string</type>\n\
    <default>%s</default>\n\
    <min>0</min>\n\
    <max>%d</max>\n\
   </param>\n\
   <param>\n\
    <name>mode</name>\n\
    <text>Mode</text>\n\
    <type>option</type>\n\
    <value>Use mode of transmitter</value>\n\
    <state>disabled</state>\n\
    <default>%d</default>\n\
   </param>\n\
  </general>\n\
  <helppanel>\n\
   <in0>CMD - command value</in0>\n\
   <in1>DATA - payload data</in1>\n\
   <in2>DATA - payload data</in2>\n\
   <in3>DATA - payload data</in3>\n\
   <in4>DATA - payload data</in4>\n\
   <out0>CMD - command value</out0>\n\
   <out1>DATA - payload data</out1>\n\
   <out2>DATA - payload data</out2>\n\
   <out3>DATA - payload data</out3>\n\
   <out4>DATA - payload data</out4>\n\
  </helppanel>\n\
 </dialogue>\n\
</oapc-config>\n";
/** Base64-encoded PNG data of an image that is used as icon for the program flow diagram within the main application*/
static char                 flowImage[]="iVBORw0KGgoAAAANSUhEUgAAAGoAAAAyCAMAAACDIr4/AAADAFBMVEUEAgT8/gSEgoQE/vzEwsSkoqTk4uT8AvxkYmSUkpTU1NS0tLT09PR0dHSMjYzMzMysrKwcGhwkKiTs7OxsbWycnJzc3Ny8vLz8/vx8e3yEhoTExsSkpqTk5uRkZmSUlpQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAD1NiQsAAABuElEQVR42u2Y0XKDIBBFjaKJgEKCgCla//8vu5AaddoY1JD2wX1wvMp49q4rKlH0xqjrOoqPx2Ns95yAzSCSJBnEZNjhcOiFT9xQKIJrROh2CDn4XQBqEJNhgOqFP8rmOyQItsYiGYvJsEWmehTkOyQItsYiGYvJsEWmehSkOEpwjfBHoXGCa4Q/qp4kuEb4o8LHjnoSlGnSvgFFDSE4g60IjaKlZrilNFPkIexFKJEihlUGKGZSQoOiuPl2xYzmOQqIahuDVSuowgxpnnYBURmB+uFMWBThaT473W5DKqJ0o5Ww9SNclnPvq8Uz2g+UygQV2KKaoCisof9ELZir3zwq2lZBrF2vC1e/VH4EbAtmUS1tDfRfI7s4oCumsXuCkTOVX2c7cKMrhqHVawUkMDWP2sgyxrKgEcFTl5dFQFcGgS9FMX+O2nqvNDI3VAN3qvyoAnag1hYljPMUXz8DPsKEWJZAsgNQUYVEcc4JYooAqDoVv7+wXjTdNnknOTGyqE6lCvxtgU0jY3mtwn9b+NR/I+pyj/P58iBe6erid2ZH7X8i/wE1LJasFAtQ+2rMvhrzd6sxb4svnYCqcdfVpisAAAAASUVORK5CYII=";
static char                 libname[]="Network Server";
static char                 xmldescr[MAX_XML_SIZE+1],sendbuf[MAX_SENDBUF_SIZE];
static lib_oapc_io_callback m_oapc_io_callback; // callback function that is used to inform the main function about changes at the IO ports
static struct libio_config  save_config;



/**
This function checks if there are new incoming clients. If the number of the client does not
exceed the limit it is accepted, elsewhere an error message is sent and the connection is
closed. Additionally the successfully connected clients are polled for new data.
this function is not part of the external library interface.
*/
static int check_all_clients(struct instData *data)
{
   int                c,i;
   unsigned long      ip;

   c=oapc_tcp_accept_connection(data->sock,&ip);
   if (c>0)
   {
      for (i=0; i<data->config.incoming; i++) if (data->client[i].sock==-1)
      {
         data->client[i].sock=c;
         data->client[i].val_char_set=0;
         data->client[i].val_cmd_set=0;
         data->client[i].val_num_set=0;
         data->client[i].val_digi_set=0;
         data->client[i].val_bin_set=0;
         data->client[i].val_bin=NULL;
         data->client[i].mode=0;
         // set socket to non-blocking in order to avoid deadlocks when sending fails
         oapc_tcp_set_blocking(c,false);
         c=-1;
         break;
      }
      if (c>0)
      {
         oapc_tcp_send(c,"FAIL too much connections\n",28,100);
         oapc_tcp_closesocket(c);
      }
      for (i=0; i<data->config.incoming; i++) if (data->client[i].sock!=-1)
      {
         check_clients(data,&data->client[i],true); // uname
         check_clients(data,&data->client[i],true); // pwd
         check_clients(data,&data->client[i],true); // mode
      }
   }
   return OAPC_OK;
}



/**
This function has to return the name that is used to display this library within the main application.
The name returned here has to be short and descriptive
@param[out] data pointer to the beginning of an char-array that contains the name
@return the length of the name structure or 0 in case of an error
*/
OAPC_EXT_API char *oapc_get_name(void)
{
   return libname;
}



/**
This function returns a set of OAPC_HAS_... flags that describe the general capabilities of this
library. These flags can be taken from oapc_libio.h
@return or-concatenated flags that describe the libraries capabilities
*/
OAPC_EXT_API unsigned long oapc_get_capabilities(void)
{
   return OAPC_HAS_OUTPUTS|
          OAPC_HAS_INPUTS|
          OAPC_HAS_XML_CONFIGURATION|
          OAPC_ACCEPTS_PLAIN_CONFIGURATION|
          OAPC_ACCEPTS_IO_CALLBACK|
          OAPC_FLOWCAT_IO;
}



/**
When the OAPC_HAS_INPUTS flag is set, the application calls this function to get the configuration
for the inputs. Here "input" is defined from the libraries point of view, means data that are sent from
the application to the library are input data
@return or-concatenated OAPC_..._IO...-flags that describe which inputs and data types are used or 0 in
        case of an error
*/
OAPC_EXT_API unsigned long oapc_get_input_flags(void)
{
   return OAPC_CHAR_IO0|
          OAPC_DIGI_IO1|
          OAPC_NUM_IO2|
          OAPC_CHAR_IO3|
          OAPC_BIN_IO4;
   // avoid it to submit the same IO number for different data types, that would cause displaying problems for the flow symbol
}



/**
When the OAPC_HAS_OUTPUTS flag is set, the application calls this function to get the configuration
for the outputs. Here "output" is defined from the libraries point of view, means data that are sent from
the library to the application are output data
@return or-concatenated OAPC_..._IO...-flags that describe which outputs and data types are used or 0 in case
        of an error
*/
OAPC_EXT_API unsigned long oapc_get_output_flags(void)
{
   return OAPC_CHAR_IO0|
          OAPC_DIGI_IO1|
          OAPC_NUM_IO2|
          OAPC_CHAR_IO3|
          OAPC_BIN_IO4;
   // avoid it to submit the same IO number for different data types, that would cause displaying problems for the flow symbol - and it would not make sense
}



/**
When the OAPC_HAS_XML_CONFIGURATION capability flag was set this function is called to retrieve an XML
structure that describes which configuration information have to be displayed within the main application.
@param[out] data pointer to the beginning of an char-array that contains the XML data
@return the length of the XML structure or 0 in case of an error
*/
OAPC_EXT_API char *oapc_get_config_data(void* instanceData)
{
   struct instData *data;

   data=(struct instData*)instanceData;

   snprintf(xmldescr,MAX_XML_SIZE,xmltempl,flowImage,data->config.ip,MAX_IP_SIZE,
                                       data->config.port,
                                       data->config.incoming,MAX_INCOMING_CONNECTIONS,
                                       data->config.uname,MAX_AUTH_SIZE,data->config.pwd,MAX_AUTH_SIZE,
                                       data->config.mode);
   return xmldescr;
}



/**
When the OAPC_ACCEPTS_PLAIN_CONFIGURATION capability flag was set this function is called for every configurable parameter
to return the value that was set within the application.
@param[in] name the name of the parameter according to the unique identifier that was set within the <name></name> field of the
           XML config file
@param[in] value the value that was configured for this parameter; in case it is not a string but a number it has to be converted,
           the representation sent by the application is always a string
*/
OAPC_EXT_API void oapc_set_config_data(void* instanceData,const char *name,const char *value)
{
   struct instData *data;

   data=(struct instData*)instanceData;

   set_config_data(data,name,value);
}



/**
This function delivers the data that are stored within the project file by the main application. It is
recommended to put two fields "version" and "length" into the data structure that is handed over here.
So when the data to be saved need to be changed it is easy to recognize which version of the data structure
is used, possible conversions can be done easier in oapc_set_loaded_data().<BR>
PLEASE NOTE: In order to keep the resulting project files compatible with all possible platforms the
             application is running at you have to store all values in network byteorder
@param[out] length the total effective length of the data that are returned by this function
@return the data that have to be saved
*/
OAPC_EXT_API char *oapc_get_save_data(void* instanceData,unsigned long *length)
{
   struct instData *data;

   data=(struct instData*)instanceData;

   *length=sizeof(struct libio_config);
   convert_to_save_byteorder(&save_config,&data->config);
   return (char*)&save_config;
}



/**
This function receives the data that have been stored within the project file by the main application. It is
recommended to check if the returned data are really what the library expects. To do that two fields
"version" and "length" within the saved data structure should be checked.<BR>
PLEASE NOTE: In order to keep the resulting project files compatible with all possible platforms the
             application is running at you have to convert all the values that have been stored in network
             byteorder back to the local byteorder
@param[in] length the total effective length of the data that are handed over to this function
@param[in] data the configuration data that are loaded for this library
*/
OAPC_EXT_API void oapc_set_loaded_data(void* instanceData,unsigned long length,char *loadedData)
{
   struct instData *data;

   data=(struct instData*)instanceData;

   if (length>sizeof(struct libio_config)) length=sizeof(struct libio_config);
   memcpy(&save_config,loadedData,length);
   convert_from_save_byteorder(&data->config,&save_config);
   data->client[0].mode=data->config.mode;
}



/**
When the capability flag OAPC_ACCEPTS_IO_CALLBACK is set, the main application no longer cyclically polls
the outputs of a Plug-In and the related parameter within the flow configuration dialogue is turned off.
Instead of this the main application hands over a function pointer to a callback and an ID. Whenever something
changes within the scope of this Plug-In that influences the output state of it, the Plug-In jumps into
that callback function to notify the main application about the new output state. The callback function 
"oapc_io_callback" expects two parameters. The first one "outputs" expects the Or-concatenated flags of
the outputs that have changed and the second one "callbackID" expects the ID that is handed over here to
identify the Plug-In. For a typedef of the callback function oapc_io_callback() that is called by the Plug-In
please refer to oapc_libio.h.<BR><BR>
Here the main application hands over a pointer to the callback function and a unique callback ID. Both have
to be stored for later use
@param[in] oapc_io_callback the callback function that has to be called whenever something changes at the
           outputs of this Plug-In
@param[in] callbackID a unique ID that identifies this Plug-In and that has to be used when the function
           oapc_io_callback is called
*/
OAPC_EXT_API void oapc_set_io_callback(void* instanceData,lib_oapc_io_callback oapc_io_callback,unsigned long callbackID)
{
   struct instData *data;

   data=(struct instData*)instanceData;

   m_oapc_io_callback=oapc_io_callback;
   data->m_callbackID=callbackID;
}



/**
This function handles all internal data initialisation and has to allocate a memory area where all
data are stored into that are required to operate this Plug-In. This memory area can be used by the
Plug-In freely, it is handed over with every function call so that the Plug-In cann access its
values. The memory area itself is released by the main application, therefore it has to be allocated
using malloc().
@return pointer where the allocated and pre-initialized memory area starts
*/
OAPC_EXT_API void* oapc_create_instance2(unsigned long flags)
{
   struct instData *data;

   flags=flags; // removing "unused" warning
   data=(struct instData*)malloc(sizeof(struct instData));
   memset(data,0,sizeof(struct instData));
   init_instancedata(data,1810);
   strcpy(data->config.ip,"0.0.0.0");
   return data;
}



/**
This function is called finally, it has to be used to release the instance data structure that was created
during the call of oapc_create_instance()
*/
OAPC_EXT_API void oapc_delete_instance(void* instanceData)
{
   if (instanceData) free(instanceData);
}



static void *recvLoop(void *arg)
{
   struct instData *data;
   int              ret,i;
   bool             doCmd;

   data=(struct instData*)arg;
   while (data->running)
   {
      oapc_thread_sleep(0);
   	check_all_clients(data);
   	
   	for (i=0; i<MAX_INCOMING_CONNECTIONS; i++)  if (data->client[i].sock!=-1)
   	{
   		if (!data->running) break;
         ret=check_clients(data,&data->client[0],false);
         if (ret==OAPC_OK)
         {
            if ((data->client[0].mode==1) && (data->client[0].val_cmd_set))
            {
               doCmd=false;
               if (data->client[0].val_digi_set) 
               {
                  m_oapc_io_callback(OAPC_DIGI_IO1,data->m_callbackID);
                  doCmd=true;
               }
               else if (data->client[0].val_num_set) 
               {
                  m_oapc_io_callback(OAPC_NUM_IO2,data->m_callbackID);
                  doCmd=true;
               }
               else if (data->client[0].val_char_set) 
               {
                  m_oapc_io_callback(OAPC_CHAR_IO3,data->m_callbackID);
                  doCmd=true;
               }            
               if (doCmd)
               {
                  m_oapc_io_callback(OAPC_CHAR_IO0,data->m_callbackID); // send the command after the data
                  while ((data->client[0].val_cmd_set) ||
                         (data->client[0].val_digi_set) || (data->client[0].val_num_set) || 
                         (data->client[0].val_char_set) || (data->client[0].val_bin_set))
                   oapc_thread_sleep(10);
               }
            }
            else 
            {
               if (data->client[0].val_digi_set) 
                m_oapc_io_callback(OAPC_DIGI_IO1,data->m_callbackID);
               if (data->client[0].val_num_set) 
                m_oapc_io_callback(OAPC_NUM_IO2,data->m_callbackID);
               if (data->client[0].val_char_set) 
                m_oapc_io_callback(OAPC_CHAR_IO3,data->m_callbackID);
               if (data->client[0].val_cmd_set) 
                m_oapc_io_callback(OAPC_CHAR_IO0,data->m_callbackID); // send the command after the data
            }
            oapc_thread_sleep(0);
         }
         else if (ret==OAPC_ERROR_CONNECTION)
         {
         	oapc_tcp_closesocket(data->client[i].sock);
         	data->client[i].sock=-1;
         }
         else oapc_thread_sleep(20);
   	}
   }
   data->running=true;
   return NULL;
}


/**
When this function is called everything has to be initialized in order to perform the required operation
@return a return value/error code that informs the main application if the initialization was done successfully
        or not
*/
OAPC_EXT_API unsigned long oapc_init(void* instanceData)
{
   int       i;
   struct instData *data;
#ifdef ENV_WINDOWS
   WSADATA wsa;

   WSAStartup(MAKEWORD(2,0),&wsa);
#endif
   data=(struct instData*)instanceData;

   data->sock=oapc_tcp_listen_on_port(data->config.port,data->config.ip);
   if (data->sock<=0) return OAPC_ERROR_RESOURCE;
   oapc_tcp_set_blocking(data->sock,false);
   for (i=0; i<MAX_INCOMING_CONNECTIONS; i++)
   {
      memset(&data->client[i],0,sizeof(struct client_dataset));
      data->client[i].sock=-1;
   }
   data->running=true;
   if (!oapc_thread_create(recvLoop,data)) return OAPC_ERROR_NO_MEMORY;

   return OAPC_OK;
}



/**
This function is called before the application unloads everything, it has to be used to deinitialize
everything and to release used resources.
*/
OAPC_EXT_API unsigned long oapc_exit(void* instanceData)
{
   struct instData *data;

   data=(struct instData*)instanceData;

   data->running=false;
   for (int i=0; i<MAX_INCOMING_CONNECTIONS; i++)
   {
      if (data->client[i].sock>-1) closesocket(data->client[i].sock);
   }
   oapc_tcp_closesocket(data->sock);
#ifdef ENV_WINDOWS
   WSACleanup();
#endif
   return OAPC_OK;
}



/**
This function is called by the main application when the library provides an digital input (marked
using the digital input flags OAPC_DIGI_IO...), a connection was edited to this input and a data
flow reaches the input.
@param[in] input specifies the input where the data are send to, here not the OAPC_DIGI_IO...-flag is used
           but the plain, 0-based input number
@param[in] value specifies the value (0 or 1) that is set to that input
@return an error code OAPC_ERROR_... in case of an error or OAPC_OK in case the value could be set
*/
OAPC_EXT_API unsigned long  oapc_set_digi_value(void* instanceData,unsigned long input,unsigned char value)
{
   struct instData *data;

   data=(struct instData*)instanceData;

   if (input!=1) return OAPC_ERROR_NO_SUCH_IO; // check for valid IO
   if (data->client[0].mode==1)
   {
      data->cmdValuePair.val_num_set=0;
      data->cmdValuePair.val_char_set=0;
      data->cmdValuePair.val_digi_set=1;
      data->cmdValuePair.val_bin_set=0;
      data->cmdValuePair.val_digi=value;
      check_cmd_value_pair(data);
   }
   else
   {
   	int i;
   	
      sprintf(sendbuf,"DIGI %d\n",value);
      for (i=0; i<data->config.incoming; i++) if (data->client[i].sock!=-1)
       oapc_tcp_send(data->client[i].sock,sendbuf,(int)strlen(sendbuf),MAX_SEND_TIMEOUT);
   }
   return OAPC_OK;
}



/**
This function is called by the main application periodically in order to poll the state of the related
output. It has to return the data that are available for that input or - in case there are none available -
the error code OAPC_ERROR_NO_DATA_AVAILABLE to notify the main application, that there is nothing new.
@param[in] output specifies the output where the data are fetched from, here not the OAPC_DIGI_IO...-flag is used
           but the plain, 0-based output number
@param[out] value if there are new data available, they are stored within that variable, if there are no new
           data, the returned data are ignored by the main application
@return an error code OAPC_ERROR_... in case of an error, OAPC_ERROR_NO_DATA_AVAILABLE in case no new data are
           available or OAPC_OK in case the value could be set
*/
OAPC_EXT_API unsigned long  oapc_get_digi_value(void* instanceData,unsigned long output,unsigned char *value)
{
   struct instData *data;

   if (output!=1) return OAPC_ERROR_NO_SUCH_IO;
   data=(struct instData*)instanceData;

   if (data->client[0].val_digi_set)
   {
      *value=data->client[0].val_digi;
      data->client[0].val_digi_set=0;
      return OAPC_OK;
   }
   return OAPC_ERROR_NO_DATA_AVAILABLE; // this library does not support inputs
}



/**
This function is called by the main application when the library provides an numerical input (marked
using the digital input flags OAPC_NUM_IO...), a connection was edited to this input and a data
flow reaches the input.
@param[in] input specifies the input where the data are send to, here not the OAPC_NUM_IO...-flag is used
           but the plain, 0-based input number
@param[in] value specifies the numerical floating-point value that is set to that input
@return an error code OAPC_ERROR_... in case of an error or OAPC_OK in case the value could be set
*/
OAPC_EXT_API unsigned long  oapc_set_num_value(void* instanceData,unsigned long input,double value)
{
   struct instData *data;

   data=(struct instData*)instanceData;

   if (input!=2) return OAPC_ERROR_NO_SUCH_IO; // check for valid IO
   if (data->client[0].mode==1)
   {
      data->cmdValuePair.val_num_set=1;
      data->cmdValuePair.val_char_set=0;
      data->cmdValuePair.val_digi_set=0;
      data->cmdValuePair.val_bin_set=0;
      data->cmdValuePair.val_num=value;
      check_cmd_value_pair(data);
   }
   else
   {
   	int i;
   	
      sprintf(sendbuf,"NUM %f\n",value);
      for (i=0; i<data->config.incoming; i++) if (data->client[i].sock!=-1)
       oapc_tcp_send(data->client[i].sock,sendbuf,(int)strlen(sendbuf),MAX_SEND_TIMEOUT);
   }
   return OAPC_OK;
}



/**
This function is called by the main application periodically in order to poll the state of the related
output. It has to return the data that are available for that input or - in case there are none available -
the error code OAPC_ERROR_NO_DATA_AVAILABLE to notify the main application, that there is nothing new.
@param[in] output specifies the output where the data are fetched from, here not the OAPC_NUM_IO...-flag is used
           but the plain, 0-based output number
@param[out] value if there are new data available, they are stored within that variable, if there are no new
           data, the returned data are ignored by the main application
@return an error code OAPC_ERROR_... in case of an error, OAPC_ERROR_NO_DATA_AVAILABLE in case no new data are
           available or OAPC_OK in case the value could be set
*/
OAPC_EXT_API unsigned long  oapc_get_num_value(void* instanceData,unsigned long output,double *value)
{
   struct instData *data;

   if (output!=2) return OAPC_ERROR_NO_SUCH_IO;
   data=(struct instData*)instanceData;

   if (data->client[0].val_num_set)
   {
      *value=data->client[0].val_num;
      data->client[0].val_num_set=0;
      return OAPC_OK;
   }
   return OAPC_ERROR_NO_DATA_AVAILABLE; // this library does not support inputs
}



/**
This function is called by the main application when the library provides an character input (marked
using the digital input flags OAPC_CHAR_IO...), a connection was edited to this input and a data
flow reaches the input.
@param[in] input specifies the input where the data are send to, here not the OAPC_CHAR_IO...-flag is used
           but a plain, 0-based input number
@param[in] value specifies the string that is set to that input
@return an error code OAPC_ERROR_... in case of an error or OAPC_OK in case the value could be set
*/
OAPC_EXT_API unsigned long  oapc_set_char_value(void* instanceData,unsigned long input,char *value)
{
   // here we allocate the send buffer dynamically because we do not know how big the string is
   char            *buffer;
   struct instData *data;
   int              i;

   data=(struct instData*)instanceData;

   if ((input!=0) && (input!=3)) return OAPC_ERROR_NO_SUCH_IO; // check for valid IO
   if (data->client[0].mode==1)
   {
      if (input==0) // command
      {
         data->cmdValuePair.cmd_set=1;
         strncpy(data->cmdValuePair.cmd,value,MAX_CMDVAL_SIZE);
         check_cmd_value_pair(data);
      }
      else // char value
      {
         data->cmdValuePair.val_num_set=0;
         data->cmdValuePair.val_char_set=1;
         data->cmdValuePair.val_digi_set=0;
         data->cmdValuePair.val_bin_set=0;
         strncpy(data->cmdValuePair.val_char,value,MAX_CMDVAL_SIZE);
         check_cmd_value_pair(data);
      }
   }
   else if (data->client[0].mode==3)
   {
      buffer=(char*)malloc(strlen(value)+10);
      if (!buffer) return OAPC_ERROR_RESOURCE;
      sprintf(buffer,"%s\n",value);
      for (i=0; i<data->config.incoming; i++) if (data->client[i].sock!=-1)
       oapc_tcp_send(data->client[i].sock,buffer,(int)strlen(buffer),MAX_SEND_TIMEOUT);
      free((void*)buffer);
   }
   else
   {
      buffer=(char*)malloc(strlen(value)+10);
      if (!buffer) return OAPC_ERROR_RESOURCE;
      if (input==3) sprintf(buffer,"CHAR %s\n",value);
      else sprintf(buffer,"CMD %s\n",value);
      for (i=0; i<data->config.incoming; i++) if (data->client[i].sock!=-1)
       oapc_tcp_send(data->client[i].sock,buffer,(int)strlen(buffer),MAX_SEND_TIMEOUT);
      free((void*)buffer);
   }
   return OAPC_OK;
}



/**
This function is called by the main application periodically in order to poll the state of the related
output. It has to return the data that are available for that input or - in case there are none available -
the error code OAPC_ERROR_NO_DATA_AVAILABLE to notify the main application, that there is nothing new.
@param[in] output specifies the output where the data are fetched from, here not the OAPC_CHAR_IO...-flag is used
           but the plain, 0-based output number
@param[out] value if there are new data available, they are stored within that variable, if there are no new
           data, the returned data are ignored by the main application
@return an error code OAPC_ERROR_... in case of an error, OAPC_ERROR_NO_DATA_AVAILABLE in case no new data are
           available or OAPC_OK in case the value could be set
*/
OAPC_EXT_API unsigned long  oapc_get_char_value(void* instanceData,unsigned long output,unsigned long length,char *value)
{
   struct instData *data;

   data=(struct instData*)instanceData;

   if ((output!=0) && (output!=3)) return OAPC_ERROR_NO_SUCH_IO; // check for valid IO
   if ((output==3) && (data->client[0].val_char_set))
   {
      if (length>MAX_CMDVAL_SIZE) length=MAX_CMDVAL_SIZE;
      strncpy(value,data->client[0].val_char,length);
      data->client[0].val_char_set=0;
      return OAPC_OK;
   }
   else if ((output==0) && (data->client[0].val_cmd_set))
   {
      if (length>MAX_CMDVAL_SIZE) length=MAX_CMDVAL_SIZE;
      strncpy(value,data->client[0].val_cmd,length);
      data->client[0].val_cmd_set=0;
      return OAPC_OK;
   }
   return OAPC_ERROR_NO_DATA_AVAILABLE; // this library does not support inputs
}



/**
This function is called by the main application when the library provides a binary data input (marked
using the digital input flags OAPC_BIN_IO...), a connection was edited to this input and a data
flow reaches the input.
@param[in] input specifies the input where the data are send to, here not the OAPC_BIN_IO...-flag is used
           but a plain, 0-based input number
@param[in] value pointer to the binary data; PLEASE NOTE: it is not allowed to change these data, they
           are a reference only that has to be copied before modification
@return an error code OAPC_ERROR_... in case of an error or OAPC_OK in case the value could be set
*/
OAPC_EXT_API unsigned long  oapc_set_bin_value(void* instanceData,unsigned long input,struct oapc_bin_head *value)
{
   struct instData *data;

   data=(struct instData*)instanceData;

   if (input!=4) return OAPC_ERROR_NO_SUCH_IO; // check for valid IO
   if (data->client[0].mode==1)
   {
      if (data->client[0].val_bin) free(data->client[0].val_bin); // thats an erroneous condition: new binary data have been sent before the preceding ones could be submitted
      data->cmdValuePair.val_bin=(struct oapc_bin_head*)malloc(sizeof(struct oapc_bin_head)+value->sizeData);
      if (!data->cmdValuePair.val_bin) return OAPC_ERROR_NO_MEMORY;
      memcpy(data->client[0].val_bin,value,sizeof(struct oapc_bin_head)+value->sizeData);
      data->cmdValuePair.val_num_set=0;
      data->cmdValuePair.val_char_set=0;
      data->cmdValuePair.val_digi_set=0;
      data->cmdValuePair.val_bin_set=1;
      check_cmd_value_pair(data);
   }
   else
   {
      struct oapc_bin_head binSendHead;
      int                  i;

      for (i=0; i<data->config.incoming; i++) if (data->client[i].sock!=-1)
      {
         sprintf(sendbuf,"BIN\n");
         if (oapc_tcp_send(data->client[i].sock,sendbuf,(int)strlen(sendbuf),MAX_SEND_TIMEOUT)<=0) return OAPC_ERROR_SEND_DATA;
      }
      for (i=0; i<data->config.incoming; i++) if (data->client[i].sock!=-1)
      {
         convert_bin_to_network_byteorder(&binSendHead,value);
         if (oapc_tcp_send(data->client[i].sock,(char*)&binSendHead,sizeof(struct oapc_bin_head)-1,MAX_SEND_TIMEOUT)<=0) return OAPC_ERROR_SEND_DATA;
         if (oapc_tcp_send(data->client[i].sock,(char*)&value->data,value->sizeData,MAX_SEND_TIMEOUT*100)<=0) return OAPC_ERROR_SEND_DATA;
      }
   }
   return OAPC_OK;
}



/**
This function is called by the main application periodically in order to poll the state of the related
output. It has to return the data that are available for that input or - in case there are none available -
the error code OAPC_ERROR_NO_DATA_AVAILABLE to notify the main application, that there is nothing new.
@param[in] output specifies the output where the data are fetched from, here not the OAPC_BIN_IO...-flag is used
           but the plain, 0-based output number
@param[out] value if there are new data available, they have to be stored within that variable, if there are
           no new data, the returned data are ignored by the main application
@return an error code OAPC_ERROR_... in case of an error, OAPC_ERROR_NO_DATA_AVAILABLE in case no new data are
           available or OAPC_OK in case the value could be set
*/
OAPC_EXT_API unsigned long  oapc_get_bin_value(void *instanceData,unsigned long output,struct oapc_bin_head **value)
{
   struct instData *data;

   if (output!=4) return OAPC_ERROR_NO_SUCH_IO;
   data=(struct instData*)instanceData;

   if ((data->client[0].val_bin_set) && (data->client[0].val_bin))
   {
      *value=data->client[0].val_bin;
      data->client[0].val_bin_set=0;
      return OAPC_OK;
   }
   else return data->client[0].lastBinError;
}



OAPC_EXT_API void oapc_release_bin_data(void *instanceData,unsigned long /*output*/)
{
   struct instData *data;

   data=(struct instData*)instanceData;

   free(data->client[0].val_bin);
   data->client[0].val_bin=NULL;
}
