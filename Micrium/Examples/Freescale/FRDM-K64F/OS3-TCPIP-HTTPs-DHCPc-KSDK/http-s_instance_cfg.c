/*
*********************************************************************************************************
*                                              uC/HTTPs
*                                 Hypertext Transfer Protocol (server)
*
*                          (c) Copyright 2004-2014; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*
*               uC/HTTPs is provided in source form to registered licensees ONLY.  It is
*               illegal to distribute this source code to any third party unless you receive
*               written permission by an authorized Micrium representative.  Knowledge of
*               the source code may NOT be used to develop a similar product.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                               HTTP INSTANCE SERVER CONFIGURATION FILE
*
*                                              TEMPLATE
*
* Filename      : http-s_instance_cfg.c
* Version       : V2.10.00
* Programmer(s) : AA
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                             INCLUDE FILES
*
* Note(s) : (1) All values that are used in this file and are defined in other header files should be
*               included in this file. Some values could be located in the same file such as task priority
*               and stack size. This template file assume that the following values are defined in app_cfg.h:
*
*                   HTTPs_OS_CFG_INSTANCE_TASK_PRIO
*                   HTTPs_OS_CFG_INSTANCE_TASK_STK_SIZE
*
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <app_cfg.h>                                           /* See Note #1.                                         */
#include  <Source/http-s.h>
#include  <Source/os.h>
#include  "http-s_instance_cfg.h"
#include  <board.h>

/*
*********************************************************************************************************
*********************************************************************************************************
*                                   FILES & FOLDERS STRING DEFINES
*********************************************************************************************************
*********************************************************************************************************
*/

#define  HTTPs_CFG_INSTANCE_STR_FOLDER_ROOT               "\\"

#define  HTTPs_CFG_INSTANCE_STR_FILE_DEFAULT              "index.html"
#define  HTTPs_CFG_INSTANCE_STR_FILE_ERR_404              "404.html"

#define  HTTPs_CFG_INSTANCE_STR_FOLDER_UPLOAD             "\\"


/*
*********************************************************************************************************
*********************************************************************************************************
*                                        SECURE CONFIGURATION
*********************************************************************************************************
*********************************************************************************************************
*/



/*
*********************************************************************************************************
*********************************************************************************************************
*                                    CALLBACK FUNCTIONS PROTOTYPES
*********************************************************************************************************
*********************************************************************************************************
*/

static  void         HTTPs_InstanceConnObjsInit(const  HTTPs_INSTANCE     *p_instance);

static  CPU_BOOLEAN  HTTPs_InstanceConnReq     (const  HTTPs_INSTANCE     *p_instance,
                                                       HTTPs_CONN         *p_conn);

static  void         HTTPs_InstanceConnErr     (const  HTTPs_INSTANCE     *p_instance,
                                                       HTTPs_CONN         *p_conn,
                                                       HTTPs_ERR           err);

static  void         HTTPs_InstanceConnClose   (const  HTTPs_INSTANCE     *p_instance,
                                                       HTTPs_CONN         *p_conn);

static  void         HTTPs_InstanceErrFileGet  (       HTTPs_STATUS_CODE   status_code,
                                                       CPU_CHAR           *p_file_str,
                                                       CPU_INT32U          file_len_max,
                                                       HTTPs_FILE_TYPE    *p_file_type,
                                                       void              **p_data,
                                                       CPU_INT32U         *p_data_len);

static  CPU_BOOLEAN  HTTPs_InstanceReqHdrRx    (const  HTTPs_INSTANCE     *p_instance,
                                                const  HTTPs_CONN         *p_conn,
                                                       HTTPs_HDR_FIELD     hdr_field);

static  CPU_BOOLEAN  HTTPs_InstanceRespHdrTx   (       HTTPs_INSTANCE     *p_instance,
                                                       HTTPs_CONN         *p_conn);

static  CPU_BOOLEAN  HTTPs_InstanceCGI_Post    (const  HTTPs_INSTANCE     *p_instance,
                                                       HTTPs_CONN         *p_conn,
                                                const  HTTPs_CGI_DATA     *p_data);

static  CPU_BOOLEAN  HTTPs_InstanceCGI_Poll    (const  HTTPs_INSTANCE     *p_instance,
                                                       HTTPs_CONN         *p_conn);


static  CPU_BOOLEAN  HTTPs_InstanceTokenValGet (const  HTTPs_INSTANCE     *p_instance,
                                                const  HTTPs_CONN         *p_conn,
                                                const  CPU_CHAR           *p_token,
                                                       CPU_INT16U          token_len,
                                                       CPU_CHAR           *p_val,
                                                       CPU_INT16U          val_len_max);


/*
*********************************************************************************************************
*********************************************************************************************************
*                            HTTP INSTANCE SERVER CONFIGURATION STRUCTURE
*********************************************************************************************************
*********************************************************************************************************
*/

const  HTTPs_CFG  HTTPs_CfgInstance_0 = {


/*
*--------------------------------------------------------------------------------------------------------
*                                      INSTANCE OS CONFIGURATION
*
* Note(s) : (1) We recommend you configure the Network Protocol Stack task priorities & HTTP server Instances'
*               task priorities as follows:
*
*               NET_OS_CFG_IF_TX_DEALLOC_TASK_PRIO  (highest priority)
*
*               HTTPs_OS_CFG_INSTANCE_TASK_PRIO
*
*               NET_OS_CFG_TMR_TASK_PRIO
*               NET_OS_CFG_IF_RX_TASK_PRIO          (lowest  priority)
*
*               We recommend that the uC/TCP-IP Timer task and network interface Receive task be lower
*               priority than almost all other application tasks; but we recommend that the network
*               interface Transmit De-allocation task be higher  priority than all application tasks that use
*               uC/TCP-IP network services.
*
*               However better performance can be observed when the web server instance is set with the lowest priority.
*               Some experimentation could be required to identify the best task priority configuration.
*
*           (2) If stop/restart feature is disabled (see 'HTTP INSTANCE CONFIGURATION' note #2 in http-s_cfg.h),
*               HTTPs_OS_CFG_OBJ_CLOSE_REQ_NAME & HTTPs_OS_CFG_OBJ_CLOSE_COMP_NAME should be configured as DEF_NULL.
*
*           (3) The web server can delay this task periodically to allow other tasks with lower priority to run.
*--------------------------------------------------------------------------------------------------------
*/

                                                                /* Configure Instance Task priority (See Note #1):      */
        HTTPs_OS_CFG_INSTANCE_TASK_PRIO,                        /* MUST be >= HTTPs_OS_PRIO_MIN                         */

                                                                /* Configure instance task size:                        */
        HTTPs_OS_CFG_INSTANCE_TASK_STK_SIZE,                    /* MUST be >= HTTPs_OS_STACK_SIZE_MIN                   */

                                                                /* Configure instance task delay ...                    */
        1,                                                      /* ... in integer milliseconds (see Note #3).           */


/*
*--------------------------------------------------------------------------------------------------------
*                                INSTANCE LISTEN SOCKET CONFIGURATION
*
* Note(s) : (1) TODO
*
*           (2) (a) 'Secure' field is used to enabled or disable the Secure Sockets Layer (SSL):
*
*                           DEF_NULL,                                  the web server instance is not secure and doesn't
*                                                                      use SSL.
*
*                           Point to a secure configuration structure, the web server is secure and uses SSL.
*
*               (b) The secure web server can be enabled ONLY if the application project contains a secure module
*                   supported by uC/TCPIP-V2 such as:
*
*                   (i)  NanoSSL provided by Mocana.
*                   (ii) CyaSSL  provided by YaSSL.
*
*           (3) (a) Default HTTP port used by all web browsers is 80. The default port number is defined by the following
*                   value:
*
*                        HTTPs_CFG_DFLT_PORT
*
*
*                   When default port is used the web server instance can be accessed using the ip address of the target
*                   from any web browser:
*
*                       http://<target ip address>
*
*                   If the web server instance is configured with the non default port, the instance server should be accessed
*                   via this kind of address:
*
*                       http://<target ip address>:<port number>
*
*                       Where
*                           <target ip address> must be replaced by the ip address of the target.
*                           <port number>       must be replaced by the configured port number.
*
*               (b) Default secure port used by all browsers is 443. The default secure port number is defined by the
*                   following value:
*
*                       HTTPs_CFG_DFLT_PORT_SECURE
*
*                   When default port is used the web server instance can be accessed using the ip address of the target
*                   from any web browser:
*
*                       https://<target ip address>
*
*                   If the web server instance is configured with the non default port, the instance server should be accessed
*                   via this kind of address:
*
*                       https://<target ip address>:<port number>
*
*                       Where
*                           <target ip address> must be replaced by the ip address of the target.
*                           <port number>       must be replaced by the configured port number.
*
*               (c) Port number must be unique, i.e. it's not possible to start two instances with the same
*                   port number.
*--------------------------------------------------------------------------------------------------------
*/

                                                                /* Configure socket type (see note #1) :                */
    HTTPs_SOCK_SEL_IPv4,
                                                                /* HTTPs_SOCK_SEL_IPv4       Accept Only IPv4.          */
                                                                /* HTTPs_SOCK_SEL_IPv6       Accept Only IPv6.          */
                                                                /* HTTPs_SOCK_SEL_IPv4_IPv6  Accept Only Ipv4 & IPv6.   */


                                                                /* Configure instance secure configuration (SSL) ...    */
                                                                /* structure (see note #2):                             */
    DEF_NULL,
                                                                /*   DEF_NULL for a non-secure web server.              */
                                                                /*   Pointer to the secure configuration to be used.    */


                                                                /* Configure instance server port (See note #3) :       */
    HTTPs_CFG_DFLT_PORT,
                                                                /* HTTPs_CFG_DFLT_PORT         Default HTTP     port.   */
                                                                /* HTTPs_CFG_DFLT_PORT_SECURE  Default HTTP SSL port.   */

/*
*--------------------------------------------------------------------------------------------------------
*                                  INSTANCE CONNECTION CONFIGURATION
*
* Note(s) : (1) (a) 'ConnNbrMax' is used to configure maximum number of connections that the web server will be able to serve
*                   simultaneously.
*
*               (b) Maximum number of connections must be configured following your requirements about the memory usage and
*                   the number of connections:
*
*                   (1) Each connection requires memory space which is reserved at the instance start up. The memory
*                       required by the web server is greatly affected by the number of connections configured.
*
*                   (2) When a client downloads an items such as an html document, image, css file, javascript file, it
*                       should open a new connection for each of these items. Also, most common web servers can open up to
*                       15 simultaneous connections. For example, 1 html document which includes 2 images + 1 css file,
*                       should have 4 connections simultaneously opened.
*
*               (c) The number of connections and uC/TCPIP-V2 configurations must be set accordingly. Each connection
*                   requires 1 socket and 1 TCP connection, so the following configuration values located in 'net_cfg.h'
*                   MUST be correctly configured:
*
*                       NET_SOCK_NBR_SOCK
*                       NET_TCP_NBR_CONN
*
*           (2) For each connection, when the inactivity timeout occurs, the connection is automatically closed with
*               whatever the last connection state was.
*
*           (3) Each connection has a buffer to receive or transmit data and to read files. If the memory is limited the buffer
*               size can be reduced, but the performance might be impacted.
*
*           (4) When the instance is created, an hook function can be called to initialize connection objects used by the instance.
*
*               If the hook is not required by the upper application, it can be set as DEF_NULL and no function will be called.
*
*               See HTTPs_InstanceConnObjsInit() function for further details.
*--------------------------------------------------------------------------------------------------------
*/
                                                                /* Configure maximum number of simultaneous ...         */
                                                                /* connections (see Note #1):                           */
     5,                                                         /* MUST be >= 1                                         */


                                                                /* Configure connection maximum inactivity timeout ...  */
                                                                /* ... in integer seconds (see Note #2).                */
    1,                                                          /* SHOULD be >= 1                                       */

                                                                /* Configure connection buffer length (see note #3):    */
    1460,                                                       /* MUST be >= 512                                       */

                                                                /* Configure instance connection objects init...        */
                                                                /* callback hook function (see note #4):                */
   &HTTPs_InstanceConnObjsInit,                                 /* SHOULD be a callback function pointer.               */


/*
*--------------------------------------------------------------------------------------------------------
*                                  INSTANCE FILE SYSTEM CONFIGURATION
*
* Note(s) : (1) Web server instance uses a working folder where files and subfolders are located.
*
*               (a) It can be set as a null pointer (DEF_NULL), if the file system doesn't support 'set working folder'
*                   functionality but HTML documents and files must be located in the default path used by the file system.
*
*               (b) If the static file system is used, it SHOULD be set to DEF_NULL.
*
*           (2) The default HTML document is returned when no file is specified in the request of the client, i.e. accessing
*               with only the web server address. Most of the time this file should be "index.html".
*
*           (3) Not Found error (404) is a HTTP standard response code which is returned when the server could not find what
*               was requested. In this case the server sends the 404 error HTML document.
*--------------------------------------------------------------------------------------------------------
*/

                                                                /* Configure instance working folder (see note #1):     */
    HTTPs_CFG_INSTANCE_STR_FOLDER_ROOT,                         /* SHOULD be a string pointer                           */

                                                                /* Configure instance default HTML document ...         */
                                                                /* ... (see note #2):                                   */
    HTTPs_CFG_INSTANCE_STR_FILE_DEFAULT,                        /* MUST be a string pointer                             */


/*
*--------------------------------------------------------------------------------------------------------
*                                CONNECTION CALLBACK HOOK CONFIGURATION
*
* Note(s) : (1) For each new incoming connection request a hook function can be called by the web server to authenticate
*               the remote connection to accept or reject it. This function can have access to allow stored request header
*               field.
*
*               If the hook is not required by the upper application, it can be set as DEF_NULL and no function will be called.
*
*               See HTTPs_InstanceConnReq() function for further details.
*
*           (2) When an internal error occurs during the processing of a connection a hook function can be called to
*               change the behavior such as the status code and the page returned.
*
*               If the hook is not required by the upper application, it can be set as DEF_NULL and no function will be called.
*
*               See HTTPs_InstanceConnErr() function for further details.
*
*           (3) Once a connection is closed a hook function can be called to notify the upper application that a connection
*               is not yet active. This hook function could be used to free some previously allocated memory.
*
*               If the hook is not required by the upper application, it can be set as DEF_NULL and no function will be called.
*
*               See HTTPs_InstanceConnClose() function for further details.
*
*           (4) Get error file hook can be called every time an error has occurred when processing a connection, i.e.
*               status code is not equal to 'OK'. This function can set the web page that should be transmit instead
*               of the default error page defined in http-s_cfg.h.
*
*               If set to DEF_NULL the default error page will be used for every error.
*
*               See HTTPs_InstanceErrFileGet() function for further details.*
*--------------------------------------------------------------------------------------------------------
*/

                                                                /* Configure connection request callback hook ...       */
                                                                /* function (see note #1):                              */
   &HTTPs_InstanceConnReq,                                      /* SHOULD be a callback function pointer.               */

                                                                /* Configure Connection error callback hook  ...        */
                                                                /* function (see note #2) :                             */
   &HTTPs_InstanceConnErr,                                      /* SHOULD be a callback function pointer.               */

                                                                /* Configure Connection  close callback hook ...        */
                                                                /* function (see note #3) :                             */
   &HTTPs_InstanceConnClose,                                    /* SHOULD be a callback function pointer.               */


/*
*--------------------------------------------------------------------------------------------------------
*                                   STATUS CODE FILE CONFIGURATION
*
* Note(s) : (1) Get error file hook can be called every time an error has occurred when processing a connection, i.e.
*               status code is not equal to 'OK'. This function can set the web page that should be transmit instead
*               of the default error page defined in http-s_cfg.h.
*
*               If set to DEF_NULL the default error page will be used for every error.
*
*               See HTTPs_InstanceErrFileGet() function for further details.
*--------------------------------------------------------------------------------------------------------
*/

                                                                /* Configure Get error file callback hook ...           */
                                                                /* (see note #4):                                       */

  &HTTPs_InstanceErrFileGet,                                    /* SHOULD be a callback function pointer.               */


/*
*--------------------------------------------------------------------------------------------------------
*                                     HEADER FIELD CONFIGURATION
*
* Note(s) : (1) (a) The uC-HTTP Server core process is, by default, the following header fields :
*                       Content-Type
*                       Content-Length
*                       Transfer-Encoding
*                       Location
*                       Connection
*
*               (b) Enabling the header feature allows the processing of additional header fields other than the
*                   the default ones.
*
*                   By configuring the right callback functions, the upper application can :
*
*                       (1) choose which header field(s) received in an http request message to keep in memory blocks for
*                           later processing in hook function 'HTTPs_InstanceConnReq()'.
*
*                       (2) add header field(s) to memory blocks that will be included in http response message by the uC-HTTP
*                           server core.
*
*               (c) To allow the processing of additional header fields, 'HTTPs_CFG_HDR_EN' must be set as 'DEF_ENABLED'.
*                   See http-s_cfg.h section 'HTTP HEADER FIELD FEATURE' for further informations.
*
*           (2) The total number of request header field blocks represents the memory blocks pool available for all the
*               connections.
*
*               If the upper application doesn't require the processing of additional request header fields, the total
*               number of request header fields can be set to 0.
*
*           (3) Each connection has a maximum number of request header field blocks it can used.
*
*               The maximum MUST be equal or less than the total number of request header field blocks (see Note #2).
*
*               If no more request header field blocks are available when a connection solicits one, the server will retry at
*               the next occasion. Therefore, the maximum request header field blocks per connection and the total number
*               of request header field blocks must be set carefully to optimize performance.
*
*           (4) Request header field data length MUST be configured to handle the longest data value the upper application
*               is expected to receive in an header field.
*
*               Only the maximum data length will be kept from a received header field with data longer than the maximum.
*
*               If the upper application doesn't require the processing of additional request header fields, the request header
*               field data length can be set to 0.
*
*           (5) Each time a header field other than the default one (see note #1a) is received, a hook function is called
*               allowing to choose which header field(s) to keep for further processing.
*
*               If the hook is not required by the upper application, it can be set as DEF_NULL and no function will be called.
*
*               See HTTPs_InstanceReqHdrRx() function for further details.
*
*           (6) The total number of response header field blocks represents the memory block pools available for all the
*               connections.
*
*               If the upper application doesn't require the addition of header fields in the response message, the total
*               number of response header fields can be set to 0.
*
*           (7) Each connection has a maximum of response header field blocks it can used.
*
*               The maximum MUST be equal or less than the total number of response header field blocks (see Note #6).
*
*               If no more response header field block is available when a connection solicit one, the server will retry at
*               the next occasion. Therefore, the maximum of response header field blocks per connection and the total number
*               of response header field blocks must be set careful to optimize performance.
*
*           (8) Boolean, Integer and Date data type are not yet supported by the server. Therefore their number must be set
*               to 0.
*
*               Only String data type is supported for now. Therefore its number SHOULD be set to the total number of response
*               header field blocks (see Note #6).
*
*           (9) Response header field data length MUST be configured to handle the longest data value the upper application
*               is ready to send in an header field.
*
*               If the upper application doesn't require the processing of additional request header fields, the request header
*               field data length can be set to 0.
*
*          (10) Before an HTTP response message is transmitted, a hook function is called to enable adding header field(s) to
*               the message before it is sent.
*
*               If the hook is not required by the upper application, it can be set as DEF_NULL and no function will be called.
*
*               See HTTPs_InstanceRespHdrTx() function for further details.
*
*--------------------------------------------------------------------------------------------------------
*/

                                                                /* Configure HTTP header field feature (see note #1):   */
   DEF_ENABLED,
                                                                /*   DEF_DISABLED     HTTP header field DISABLED        */
                                                                /*   DEF_ENABLED      HTTP header field ENABLED         */

                                                                /* Configure number of request header field blocks ...  */
   15,                                                          /* ... (see note #2):                                   */
                                                                /* SHOULD be >= 0                                       */

                                                                /* Configure maximum of request header field blocks ... */
   3,                                                           /* ... per connection (see note #3):                    */
                                                                /* SHOULD be >= 0                                       */

                                                                /* Configure maximum of request header field data  ...  */
   32,                                                          /* length (see note #4):                                */
                                                                /* SHOULD be >= 0                                       */

                                                                /* Configure request header field receive callback ...  */
  &HTTPs_InstanceReqHdrRx,                                      /* ... function (see note #5):                          */
                                                                /* SHOULD be a callback function pointer.               */




                                                                /* Configure number of response header field blocks ... */
   15,                                                          /* ...  (see note #6):                                  */
                                                                /* SHOULD be >= 0                                       */

                                                                /* Configure maximum response header field blocks ...   */
   3,                                                           /* ... per connection (see note #7):                    */
                                                                /* SHOULD be >= 0                                       */

                                                                /* Configure number of response header fields with ...  */
   0,                                                           /* ... boolean data type (see note #8):                 */
                                                                /* SHOULD be = 0                                        */

                                                                /* Configure number of response header fields with ...  */
   0,                                                           /* ... integer data type (see note #8):                 */
                                                                /* SHOULD be = 0                                        */

                                                                /* Configure number of response header fields with ...  */
   0,                                                           /* ... date time data type (see note #8):               */
                                                                /* SHOULD be = 0                                        */

                                                                /* Configure number of response header fields with ...  */
   15,                                                          /* ... string data (see note #8):                       */
                                                                /* SHOULD be >= 0                                       */

                                                               /* Configure maximum string length (see note #9):        */
   128,                                                        /* SHOULD be >= 0                                        */

                                                               /* Configure response header field transmit ...          */
  &HTTPs_InstanceRespHdrTx,                                    /* ... callback function (see note #10):                 */
                                                               /* SHOULD be a callback function pointer.                */


/*
*--------------------------------------------------------------------------------------------------------
*                         INSTANCE COMMON GATEWAY INTERFACE (CGI) FORM CONFIGURATION
*
* Note(s) : (1) (a) Forms in HTML documents are allowed. When the form is posted, the web server will process the POST
*                   action and will invoke the callback with a list of key-value pairs transmitted.
*
*                   Assuming we have an HTML page that look like this:
*
*                       <html>
*                           <body>
*                               <form action="form_return_page.htm" method="post">
*                                   Text Box 1: <input type="text" name="textbox1" /><br>
*                                   Text Box 2: <input type="text" name="textbox1" /><br>
*                                   <input type="submit" name="submit" value="Submit"></input>
*                               </form>
*                           </body>
*                       </html>
*
*
*                   When the client sends the request, the web server should call the callback function with the following
*                   key pair value items:
*
*                       Key-Name: "textbox1",  Key-Value: "Text Box 1 value"
*                       Key-Name: "textbox2",  Key-Value: "Text Box 2 value"
*                       Key-Name: "submit",    Key-Value: "Submit"*
*
*
*                   For more information see User Manual section 'CGI Form Submission'
*
*
*               (b) Only form method 'POST' action is supported.
*
*               (c) 'HTTPs_CFG_CGI_EN' must be set as 'DEF_ENABLED' to enable the web server instance token replacement.
*                   See http-s_cfg.h section 'HTTP CGI FORM CONFIGURATION' for further information.
*
*
*           (2) (a) Number of control key value pairs must be greater than or equal to the maximum number of inputs
*                   which can be transmitted by one of your forms contained in your HTML documents.
*
*               (b) If the feature is not enabled, this value is not used.
*
*
*           (3) (a) Control name length MUST be configured to handle the longest Key-Name contained in your html documents.
*
*               (b) Control value length MUST be configured to handle the longest Key-Value which can be entered by the user
*                   in your html documents.
*
*
*           (4) (a) CGI multipart forms MUST be used to transmit large messages such as a file or e-mails message body. HTML pages
*                   that contain mutlipart forms look like this:
*
*                       <html>
*                           <body>
*                               <form action="form_return_page.htm" method="post" enctype="multipart/form-data">
*                                   Browse file: <input type="file" name="fileUploaded"/><br>
*                                   <input type="submit" name="submit" value="Submit"></input>
*                               </form>
*                           </body>
*                       </html>
*
*               (b) If CGI file upload feature is enabled, the web server will store the file received. If the feature is
*                   not enabled and a file is received the file will be simply dropped.
*
*                   (1) File upload is not yet possible with the Static File System.
*
*                   (2) File overwrite must be enabled to allow a file to be received if the file already exists in the folder.
*
*                   (3) A folder name need to be specified to indicate where the uploaded files will be saved.
*
*                       If you wish to save uploaded files directly in the root web directory, the name folder needs
*                       to be set as "\\".
*
*                       If uploaded files need to be saved inside a subfolder of the root web directory, the folder MUST
*                       already exist when the HTTP server tries to access it.
*
*           (5) (a) The Post callback function occurs after all the key-value pairs of a POST request message have been
*                   recovered. This allows the processing of the recovered key-value pairs.
*
*                   The callback function SHOULD NOT be blocking and SHOULD return quickly. A time consuming function will
*                   block the processing of the other connections and reduce the HTTP server performance.
*
*                   In case the CGI data processing is time consuming, the Poll callback function SHOULD be enabled to
*                   allow the server to periodically verify if the upper application has finished the CGI data processing.
*
*                   If the CGI form feature is not enabled, this field is not used and can be set as DEF_NULL.
*
*                   See HTTPs_InstanceCGI_Post() function for further details.
*
*               (b) The Poll callback function SHOULD be enable in case the CGI data processing require lots of time. It
*                   allows the HTTP server to periodically poll the upper application and verify if the CGI data processing
*                   has finished.
*
*                   If the Poll feature is not required, this field SHOULD be set as DEF_NULL.
*
*                   See HTTPs_InstanceCGI_Poll() function for further details.
*
*--------------------------------------------------------------------------------------------------------
*/

                                                                /* Configure instance CGI form feature (see note #1):   */
    DEF_ENABLED,
                                                                /*   DEF_DISABLED     CGI form DISABLED                 */
                                                                /*   DEF_ENABLED      CGI form ENABLED                  */


                                                                /* Configure number of control key value pairs ...      */
                                                                /* ...  (see note #2):                                  */
    15,                                                         /* SHOULD be >= 1                                       */


                                                                /* Configure maximum control name length ...            */
                                                                /* ... (see note #3a):                                  */
    10,                                                         /* SHOULD be > 1                                        */


                                                                /* Configure maximum control value length ...           */
                                                                /* ... (see note #3b):                                  */
    48,                                                         /* SHOULD be > 1                                        */


                                                                /* Configure instance CGI multipart form feature ...    */
                                                                /* ... (see note #4a):                                  */
    DEF_ENABLED,
                                                                /*   DEF_DISABLED     CGI multipart form DISABLED       */
                                                                /*   DEF_ENABLED      CGI multipart form ENABLED        */


                                                                /* Configure instance CGI file upload feature ...       */
                                                                /* ... (see note #4b):                                  */
    DEF_DISABLED,
                                                                /*   DEF_DISABLED     File upload DISABLED              */
                                                                /*   DEF_ENABLED      File upload ENABLED               */


                                                                /* Configure instance file overwrite feature ...        */
                                                                /* ... (see note #4b2):                                 */
    DEF_DISABLED,
                                                                /*   DEF_DISABLED     File overwrite DISABLED           */
                                                                /*   DEF_ENABLED      File overwrite ENABLED            */


                                                                /* Configure instance upload folder (see note #4b3):    */
    HTTPs_CFG_INSTANCE_STR_FOLDER_UPLOAD,                       /* SHOULD be a string pointer.                          */

                                                                /* Configure CGI post callback (see note #5a):          */
   &HTTPs_InstanceCGI_Post,                                     /* SHOULD be a callback function pointer.               */

                                                                /* Configure CGI poll callback (see note #5b):          */
   &HTTPs_InstanceCGI_Poll,                                     /* SHOULD be a callback function pointer.               */


/*
*--------------------------------------------------------------------------------------------------------
*                               DYNAMIC TOKEN REPLACEMENT CONFIGURATION
*
* Note(s) : (1) (a) Dynamic content can be inserted in HTML web pages (files having the htm or html suffix) by using
*                   special tokens being substituted when the page is actually sent to the web browser. Those tokens
*                   are represented in an HTML document as:
*
*                       ${TOKEN_NAME}
*
*                   Assuming we have an HTML page that look like this:
*
*                       <html>
*                           <body>
*                               This system's IP address is ${My_IP_Address}
*                           </body>
*                       </html>
*
*
*                   When a web client requests this file, the web server will parse the file, find the ${My_IP_Address}
*                   token, and pass the string " My_IP_Address " into the callback function. That function will then
*                   substitute the token for its value, sending the following HTML file to the client:
*
*                       <html>
*                           <body>
*                               This system's IP address is 135.17.115.215
*                           </body>
*                       </html>
*
*                   For more information see User manual section TODO
*
*               (b) 'HTTPs_CFG_TOKEN_PARSE_EN' must be set as 'DEF_ENABLED' to enable the web server instance token
*                   replacement. See http-s_cfg.h section 'HTTP DYNAMIC TOKEN REPLACEMENT CONFIGURATION' for further
*                   information.
*
*           (2) (a) Each connection that transmits an HTML document requires only one token. So to optimize performance, numbers
*                   of tokens SHOULD be equally configured to the maximum number of HTML documents that can be transmitted
*                   simultaneously.
*
*               (b) (1) If dynamic token replacement feature is enabled, number of tokens must be greater than or equal to
*                       one.
*
*                   (2) If the feature is not enabled, this value is not used.
*
*
*           (3) (a) The web server reserves a value buffer for each token which is passed to the callback function to be
*                   filled with the replacement value. Therefore, the length of the token value must be configured to handle the
*                   longest value.
*
*               (b) (1) If the dynamic token replacement feature is enabled, token value length must be greater than or equal
*                       to one.
*
*                   (2) If the feature is not enabled, this value is not used.
*
*           (4) (a) The callback function is called by the web server when a token is found. This means the callback
*                   function must fill a buffer with the value of the instance token to be sent.
*                   See 'HTTPs_InstanceTokenValGet' for further information.
*
*               (b) If the feature is not enabled, this field is not used and can be set as DEF_NULL.
*--------------------------------------------------------------------------------------------------------
*/
                                                                   /* Configure instance dynamic token replacement ...     */
                                                                   /* ... feature (see note #1):                           */
       DEF_ENABLED,
                                                                   /*   DEF_DISABLED     Token replacement DISABLED        */
                                                                   /*   DEF_ENABLED      Token replacement ENABLED         */

                                                                   /* Configure instance number of token (see note #2):    */
        5,                                                         /* SHOULD be >= 1                                       */

                                                                   /* Configure instance token value length (see note #3): */
       12,                                                         /* SHOULD be >= 1                                       */

                                                                   /* Configure dynamic token replacement callback ...     */
                                                                   /* ... function (see note #4):                          */
      &HTTPs_InstanceTokenValGet,                                  /* SHOULD be a callback function pointer.               */


/*
*--------------------------------------------------------------------------------------------------------
*                                    INSTANCE PROXY CONFIGURATION
*
* Note(s) : (1) (a) When an HTTP Server is behind an HTTP Proxy, the HTTP client must send its requests with an
*                    absolute Uniform Resource Identifier (URI).
*                   For example,
*                       GET http://example.com/index.html HTTP/1.1
*
*                   When the absolute URI feature is enabled, the HTTP server will support absolute URI in the first line
*                   of the http request messages (see example just above).
*
*                   The server will also look for the 'Host' header field in the received request messages and save it in
*                   the 'HostPtr' field of the HTTPs_CONN structure.
*
*               (b) 'HTTPs_CFG_ABSOLUTE_URI_EN' must be set as 'DEF_ENABLED' to enable the web server support of
*                    absolute URI.
*                    See the http-s_cfg.h section of 'HTTP PROXY CONFIGURATION' for further information.
*
*               (c) The maximum host name length is the maximum length the server will allow for the received host name
*                   in a request message.
*--------------------------------------------------------------------------------------------------------
*/

                                                                /* Configure maximum host name length (see note #1c):   */
    128,                                                        /* SHOULD be > 1                                        */


};                                                              /* End of configuration structure.                      */




/*
*********************************************************************************************************
*********************************************************************************************************
*                                         CALLBACK FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        HTTPs_InstanceConnObjsInit()
*
* Description : Called to initialized the instance connection objects;
*               Examples of behaviors that could be implemented :
*
*               (a) Session connections handling initialization:
*
*                   (1) Initialize the memory pool and chained list for session connection objects.
*                   (2) Initialize a periodic timer which check for expired session and release them if
*                       it is the case.
*
*               (b) Back-end CGI task initialization.
*
* Argument(s) : p_instance  Pointer to the HTTP instance structure
*
* Return(s)   : none.
*
* Caller(s)   : HTTPs_InstanceInit() via 'p_cfg->InstanceConnObjsInit
*
* Note(s)     : none.
*
*********************************************************************************************************
*/

static void HTTPs_InstanceConnObjsInit (const  HTTPs_INSTANCE  *p_instance)
{
     (void)&p_instance;                                         /* Prevent 'variable unused' compiler warning.          */
}


/*
*********************************************************************************************************
*                                        HTTPs_InstanceConnReq()
*
* Description : Called after the parsing of an HTTP request message's first line and header(s).
*               Allows the application to process the information received in the request message.
*               Examples of behaviors that could be implemented :
*
*               (a) Analyze the Request-URI and validate that the client has the permission to access
*                   the resource. If not, change the Response Status Code to 403 (Forbidden) or 401
*                   (Unauthorized) if an Authentication technique is implemented. In case of a 401
*                   Status, a "WWW-Authenticate" header needs to be added to the response message
*                   (See HTTPs_InstanceRespHdrTx() function)
*
*               (b) Depending on whether the header feature is enabled and which header fields have been
*                   chosen for use (see HTTPs_InstanceReqHdrRx() function), different behaviors
*                   are possible. Here are some examples :
*
*                       (1) A "Cookie" header is received. The default html page is modified to include
*                           personalized features for the client.
*
*                       (2) An "Authorization" header is received. This validates that the client login is good and
*                           changes permanently its' access to the folder/file.
*
*                       (3) An "If-Modified-Since" header is received. It then validates whether or not the resource
*                           has been modified since the 'HTTP-date' received with the header. If it was, continue
*                           with the request processing normally, else change the Status Code to 304 (Not Modified).
*
* Argument(s) : p_instance  Pointer to the HTTP instance structure
*
*               p_conn      Pointer to the connection control structure
*
* Return(s)   : DEF_YES,    if the client is authorized to access the requested resource.
*
*               DEF_NO,     otherwise.
*                               Status code will be set automatically to HTTPs_STATUS_UNAUTHORIZED
*
* Caller(s)   : HTTPs_Req() via 'p_cfg->ConnReqFnctPtr'.
*
* Note(s)     : (1) The instance structure is for read-only. It must not be modified at any point in this hook function.
*
*               (2) The following connection attributes can be accessed to analyze the connection:
*
*                   (a) 'ClientAddr'
*
*                           This connection parameter contains the IP address and port used by the remote client to access the
*                           server instance.
*
*                   (b) 'Method'
*
*                           HTTPs_METHOD_GET        Get  request
*                           HTTPs_METHOD_POST       Post request
*                           HTTPs_METHOD_HEAD       Head request
*
*                   (c) 'FileNamePtr'
*
*                           This is a pointer to the string that contains the name of the file requested.
*
*                   (d) 'ReqHdrCtr'
*
*                           This parameter is a counter of the number of header field that has been stored.
*
*                   (e) 'ReqHdrFirstPtr'
*
*                           This parameter is a pointer to the first header field stored. A linked list is created with
*                           all header field stored.
*
*               (3) In this hook function, only the under-mentioned connection parameters are allowed
*                   to be modified :
*
*                   (a) 'StatusCode'
*
*                           HTTPs_STATUS_OK,
*                           HTTPs_STATUS_CREATED,
*                           HTTPs_STATUS_ACCEPTED,
*                           HTTPs_STATUS_NO_CONTENT,
*                           HTTPs_STATUS_RESET_CONTENT,
*                           HTTPs_STATUS_MOVED_PERMANENTLY,
*                           HTTPs_STATUS_FOUND,
*                           HTTPs_STATUS_SEE_OTHER,
*                           HTTPs_STATUS_NOT_MODIFIED,
*                           HTTPs_STATUS_USE_PROXY,
*                           HTTPs_STATUS_TEMPORARY_REDIRECT,
*                           HTTPs_STATUS_BAD_REQUEST,
*                           HTTPs_STATUS_UNAUTHORIZED,
*                           HTTPs_STATUS_FORBIDDEN,
*                           HTTPs_STATUS_NOT_FOUND,
*                           HTTPs_STATUS_METHOD_NOT_ALLOWED,
*                           HTTPs_STATUS_NOT_ACCEPTABLE,
*                           HTTPs_STATUS_REQUEST_TIMEOUT,
*                           HTTPs_STATUS_CONFLIT,
*                           HTTPs_STATUS_GONE,
*                           HTTPs_STATUS_LENGTH_REQUIRED,
*                           HTTPs_STATUS_PRECONDITION_FAILED,
*                           HTTPs_STATUS_REQUEST_ENTITY_TOO_LARGE,
*                           HTTPs_STATUS_REQUEST_URI_TOO_LONG,
*                           HTTPs_STATUS_UNSUPPORTED_MEDIA_TYPE,
*                           HTTPs_STATUS_REQUESTED_RANGE_NOT_SATISFIABLE,
*                           HTTPs_STATUS_EXPECTATION_FAILED,
*                           HTTPs_STATUS_INTERNAL_SERVER_ERR,
*                           HTTPs_STATUS_NOT_IMPLEMENTED,
*                           HTTPs_STATUS_SERVICE_UNAVAILABLE,
*                           HTTPs_STATUS_HTTP_VERSION_NOT_SUPPORTED,
*                           HTTPs_STATUS_UNKOWN
*
*                   (b) 'FileNamePtr'
*
*                           This is a pointer to the string that contains the name of the file requested. You can change
*                           the name of the requested file to send another file instead without error.
*
*                   (c) 'FilePtr'
*
*                           This is a pointer to the data file or data to transmit. This parameter should be null when calling
*                           this function. If data from memory has to be sent instead of a file, this pointer must be set
*                           to the location of the data.
*
*                   (d) 'FileType'
*
*                           HTTPs_FILE_TYPE_FS              Open and transmit a file. Value by default.*
*                           HTTPs_FILE_TYPE_STATIC_DATA     Transmit data from the memory. Must be set by the hook function.
*
*                   (e) 'FileLen'
*
*                           0,                              Default value, will be set when the file is opened.
*                           Data length,                    Must be set by the hook function when transmitting data from
*                                                           the memory
*
*                   (f) 'ConnDataPtr'
*
*                           This is a pointer available for the upper application when memory block must be allocated
*                           to process the connection request. If memory is allocated by the upper application, the memory
*                           space can be deallocated into another hook function.
*
*               (4) When the Location of the requested file has changed, besides the Status Code to change (3xx),
*                   the FileNamePtr parameter needs to be updated. A "Location" header will be added automatically in
*                   the response by uC/HTTPs core with the new location.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  HTTPs_InstanceConnReq(const  HTTPs_INSTANCE  *p_instance,
                                                  HTTPs_CONN      *p_conn)
{
#if (HTTPs_CFG_HDR_EN == DEF_ENABLED)
    HTTPs_HDR_BLK  *p_req_hdr_blk;
#endif


#if (HTTPs_CFG_HDR_EN == DEF_ENABLED)
                                                                /* ------- PROCESS THE REQ HDR FIELDS RECEIVED -------- */
    p_req_hdr_blk = p_conn->ReqHdrFirstPtr;
    while (p_req_hdr_blk != (HTTPs_HDR_BLK *)0) {
        switch (p_req_hdr_blk->HdrField) {
            case HTTPs_HDR_FIELD_COOKIE:
                 break;


            default:
                 break;
        }
        p_req_hdr_blk = p_req_hdr_blk->HdrBlkNextPtr;
    }
#endif

    (void)&p_instance;                                          /* Prevent 'variable unused' compiler warning.          */

    return (DEF_OK);
}


/*
*********************************************************************************************************
*                                      HTTPs_InstanceConnErr()
*
* Description : Called each time an internal error occurs.
*
* Argument(s) : p_instance  Pointer to the HTTP instance structure.
*
*               p_conn      Pointer to the connection control structure.
*
*               err         Internal error that occurred:
*                               HTTPs_ERR_REQ_METHOD_NOT_SUPPORTED
*                               HTTPs_ERR_REQ_FORMAT_INV
*                               HTTPs_ERR_REQ_URI_INV
*                               HTTPs_ERR_REQ_URI_LEN
*                               HTTPs_ERR_REQ_PROTOCOL_VER_NOT_SUPPORTED
*                               HTTPs_ERR_REQ_MORE_DATA_REQUIRED
*                               HTTPs_ERR_REQ_HDR_OVERFLOW
*                               HTTPs_ERR_METHOD_POST_CGI_NOT_EN
*                               HTTPs_ERR_METHOD_POST_MULTIPART_NOT_EN
*                               HTTPs_ERR_METHOD_POST_CONTENT_TYPE_UNKNOWN
*                               HTTPs_ERR_METHOD_POST_STATE_UNKNOWN
*                               HTTPs_ERR_CGI_FORMAT_INV
*                               HTTPs_ERR_CGI_FILE_UPLOAD_OPEN
*                               HTTPs_ERR_CGI_CFG_POOL_SIZE_INV
*                               HTTPs_ERR_STATE_UNKNOWN
*
* Return(s)   : none.
*
* Caller(s)   : HTTPs_ErrInternal() via 'p_cfg->ConnErrFnctPtr()'.
*
* Note(s)     : (1) The instance structure is for read-only. It must not be modified at any point in this hook function.
*
*               (2) The following connection attributes can be accessed to analyze the connection:
*
*                   (a) 'ClientAddr'
*
*                           This connection parameter contains the IP address and port used by the remote client to access the
*                           server instance.
*
*                   (b) 'Method'
*
*                           HTTPs_METHOD_GET        Get  request
*                           HTTPs_METHOD_POST       Post request
*                           HTTPs_METHOD_HEAD       Head request
*
*                   (c) 'FileNamePtr'
*
*                           This is a pointer to the string that contains the name of the file requested.
*
*                   (d) 'ReqHdrCtr'
*
*                           This parameter is a counter of the number of header field that has been stored.
*
*                   (e) 'ReqHdrFirstPtr'
*
*                           This parameter is a pointer to the first header field stored. A linked list is created with
*                           all header field stored.
*
*               (3) In this hook function, only the under-mentioned connection parameters are allowed
*                   to be modified :
*
*                   (a) 'StatusCode'
*
*                           HTTPs_STATUS_OK,
*                           HTTPs_STATUS_CREATED,
*                           HTTPs_STATUS_ACCEPTED,
*                           HTTPs_STATUS_NO_CONTENT,
*                           HTTPs_STATUS_RESET_CONTENT,
*                           HTTPs_STATUS_MOVED_PERMANENTLY,
*                           HTTPs_STATUS_FOUND,
*                           HTTPs_STATUS_SEE_OTHER,
*                           HTTPs_STATUS_NOT_MODIFIED,
*                           HTTPs_STATUS_USE_PROXY,
*                           HTTPs_STATUS_TEMPORARY_REDIRECT,
*                           HTTPs_STATUS_BAD_REQUEST,
*                           HTTPs_STATUS_UNAUTHORIZED,
*                           HTTPs_STATUS_FORBIDDEN,
*                           HTTPs_STATUS_NOT_FOUND,
*                           HTTPs_STATUS_METHOD_NOT_ALLOWED,
*                           HTTPs_STATUS_NOT_ACCEPTABLE,
*                           HTTPs_STATUS_REQUEST_TIMEOUT,
*                           HTTPs_STATUS_CONFLIT,
*                           HTTPs_STATUS_GONE,
*                           HTTPs_STATUS_LENGTH_REQUIRED,
*                           HTTPs_STATUS_PRECONDITION_FAILED,
*                           HTTPs_STATUS_REQUEST_ENTITY_TOO_LARGE,
*                           HTTPs_STATUS_REQUEST_URI_TOO_LONG,
*                           HTTPs_STATUS_UNSUPPORTED_MEDIA_TYPE,
*                           HTTPs_STATUS_REQUESTED_RANGE_NOT_SATISFIABLE,
*                           HTTPs_STATUS_EXPECTATION_FAILED,
*                           HTTPs_STATUS_INTERNAL_SERVER_ERR,
*                           HTTPs_STATUS_NOT_IMPLEMENTED,
*                           HTTPs_STATUS_SERVICE_UNAVAILABLE,
*                           HTTPs_STATUS_HTTP_VERSION_NOT_SUPPORTED,
*                           HTTPs_STATUS_UNKOWN
*
*                   (b) 'FileNamePtr'
*
*                           This is a pointer to the string that contains the name of the file requested. You can change
*                           the name of the requested file to send another file instead without error.
*
*                   (c) 'FilePtr'
*
*                           This is a pointer to the data file or data to transmit. This parameter should be null when calling
*                           this function. If data from memory has to be sent instead of a file, this pointer must be set
*                           to the location of the data.
*
*                   (d) 'FileType'
*
*                           HTTPs_FILE_TYPE_FS              Open and transmit a file. Value by default.*
*                           HTTPs_FILE_TYPE_STATIC_DATA     Transmit data from the memory. Must be set by the hook function.
*
*                   (e) 'FileLen'
*
*                           0,                              Default value, will be set when the file is opened.
*                           Data length,                    Must be set by the hook function when transmitting data from
*                                                           the memory
*
*                   (f) 'ConnDataPtr'
*
*                           This is a pointer available for the upper application when memory block must be allocated
*                           to process the connection request. If memory is allocated by the upper application, the memory
*                           space can be deallocated into another hook function.
*********************************************************************************************************
*/

static  void  HTTPs_InstanceConnErr (const  HTTPs_INSTANCE  *p_instance,
                                            HTTPs_CONN      *p_conn,
                                            HTTPs_ERR        err)
{
    (void)&p_instance;                                          /* Prevent 'variable unused' compiler warning.          */
    (void)&p_conn;

    switch (err) {
        case HTTPs_ERR_FILE_404_NOT_FOUND:
            return;


        default:
            break;
    }

}


/*
*********************************************************************************************************
*                                       HTTPs_InstanceConnClose()
*
* Description : Called each time a connection is being closed. Allows the upper application to free some
*               previously allocated memory.
*
*
* Argument(s) : p_instance  Pointer to the HTTP instance structure.
*
*               p_conn      Pointer to the connection control structure.
*
* Return(s)   : none.
*
* Caller(s)   : HTTPs_ConnClose() via 'p_cfg->ConnCloseFnctPtr()'
*
* Note(s)     : (1) The instance structure is for read-only. It MUST NOT be modified.
*
*               (2) The connection structure is for read-only since the connection will be freed after this call.
*
*                   (a) 'ConnDataPtr' parameter should be used to store the location of the data allocated.
*********************************************************************************************************
*/

static  void  HTTPs_InstanceConnClose (const  HTTPs_INSTANCE  *p_instance,
                                              HTTPs_CONN      *p_conn)
{
   (void)&p_instance;                                           /* Prevent 'variable unused' compiler warning.          */
   (void)&p_conn;
}


/*
*********************************************************************************************************
*                                      HTTPs_InstanceErrFileGet()
*
* Description : (1) Called when the response status code has been changed to a value other than OK (200). The
*                   change could be the result of the request processing in the HTTPs_InstanceConnReq()
*                   callback function or the result of an internal error in the uC/HTTPs core.
*
*               (2) This function is intended to set the name of the file which will be sent with the response message.
*                   If no file is set, a default status page will be sent including the status code number and the
*                   reason phrase.
*
* Argument(s) : status_code     Status code, number of the response message.
*
*               p_file_str      Pointer to the buffer where the filename string must be copied.
*
*               file_len_max    Maximum length of the filename.
*
*               p_file_type     Pointer to the buffer where the file type must be copied:
*                                   HTTPs_FILE_TYPE_FS,           when file is include in a File System.
*                                   HTTPs_FILE_TYPE_STATIC_DATA,  when file is a simple data stream inside a memory
*                                                                 block.
*
*               p_data          Pointer to the data memory block, if file type is HTTPs_FILE_TYPE_STATIC_DATA.
*
*                               DEF_NULL,                         otherwise
*
*               p_data_len      Pointer to variable holding
*                               the length of the data,           if file type is HTTPs_FILE_TYPE_STATIC_DATA.
*
*                               DEF_NULL,                         otherwise
*
* Return(s)   : none.
*
* Caller(s)   : HTTPs_PrepareStatusCode() via 'p_cfg->ErrFileGetFnctPtr().
*
* Note(s)     : (1) If the configured file doesn't exist the instance will transmit the default web page instead,
*                   defined by HTTPs_CFG_HTML_DFLT_ERR_PAGE in http-s_cfg.h
*********************************************************************************************************
*/

static  void  HTTPs_InstanceErrFileGet (HTTPs_STATUS_CODE    status_code,
                                        CPU_CHAR            *p_file_str,
                                        CPU_INT32U           file_len_max,
                                        HTTPs_FILE_TYPE     *p_file_type,
                                        void               **p_data,
                                        CPU_INT32U          *p_data_len)
{
    switch (status_code) {
        case HTTPs_STATUS_NOT_FOUND:
             Str_Copy_N(p_file_str, HTTPs_CFG_INSTANCE_STR_FILE_ERR_404, file_len_max);
            *p_file_type = HTTPs_FILE_TYPE_FS;
             return;


        default:
             Str_Copy_N(p_file_str, "error.html", file_len_max);
            *p_data      = HTTPs_CFG_HTML_DFLT_ERR_PAGE;
            *p_data_len  = HTTPs_HTML_DLFT_ERR_LEN;
            *p_file_type = HTTPs_FILE_TYPE_STATIC_DATA;
             return;
    }
}


/*
*********************************************************************************************************
*                                       HTTPs_InstanceReqHdrRx()
*
* Description : Called each time a header field is parsed in a request message. Allows to choose which
*               additional header field(s) need to be processed by the upper application.
*
* Argument(s) : p_instance  Pointer to the HTTP instance structure.
*
*               p_conn      Pointer to the connection control structure.
*
*               hdr_field   Type of the header field received.
*
*                                   HTTPs_HDR_FIELD_ACCEPT,
*                                   HTTPs_HDR_FIELD_ACCEPT_CHARSET,
*                                   HTTPs_HDR_FIELD_ACCEPT_ENCODING,
*                                   HTTPs_HDR_FIELD_ACCEPT_LANGUAGE,
*                                   HTTPs_HDR_FIELD_ACCEPT_RANGES,
*                                   HTTPs_HDR_FIELD_AGE,
*                                   HTTPs_HDR_FIELD_ALLOW,
*                                   HTTPs_HDR_FIELD_AUTHORIZATION,
*                                   HTTPs_HDR_FIELD_CLIENT_IP,
*                                   HTTPs_HDR_FIELD_CONTENT_BASE,
*                                   HTTPs_HDR_FIELD_CONTENT_ENCODING,
*                                   HTTPs_HDR_FIELD_CONTENT_LANGUAGE,
*                                   HTTPs_HDR_FIELD_CONTENT_LOCATION,
*                                   HTTPs_HDR_FIELD_CONTENT_MD5,
*                                   HTTPs_HDR_FIELD_CONTENT_RANGE,
*                                   HTTPs_HDR_FIELD_COOKIE,
*                                   HTTPs_HDR_FIELD_COOKIE2,
*                                   HTTPs_HDR_FIELD_DATE,
*                                   HTTPs_HDR_FIELD_ETAG,
*                                   HTTPs_HDR_FIELD_EXPECT,
*                                   HTTPs_HDR_FIELD_EXPIRES,
*                                   HTTPs_HDR_FIELD_FROM,
*                                   HTTPs_HDR_FIELD_IF_MODIFIED_SINCE,
*                                   HTTPs_HDR_FIELD_IF_MATCH,
*                                   HTTPs_HDR_FIELD_IF_NONE_MATCH,
*                                   HTTPs_HDR_FIELD_IF_RANGE,
*                                   HTTPs_HDR_FIELD_IF_UNMODIFIED_SINCE,
*                                   HTTPs_HDR_FIELD_LAST_MODIFIED,
*                                   HTTPs_HDR_FIELD_PUBLIC,
*                                   HTTPs_HDR_FIELD_RANGE,
*                                   HTTPs_HDR_FIELD_REFERER,
*                                   HTTPs_HDR_FIELD_RETRY_AFTER,
*                                   HTTPs_HDR_FIELD_SERVER,
*                                   HTTPs_HDR_FIELD_SET_COOKIE,
*                                   HTTPs_HDR_FIELD_SET_COOKIE2,
*                                   HTTPs_HDR_FIELD_TE,
*                                   HTTPs_HDR_FIELD_TRAILER,
*                                   HTTPs_HDR_FIELD_UPGRATE,
*                                   HTTPs_HDR_FIELD_USER_AGENT,
*                                   HTTPs_HDR_FIELD_VARY,
*                                   HTTPs_HDR_FIELD_VIA,
*                                   HTTPs_HDR_FIELD_WARNING,
*                                   HTTPs_HDR_FIELD_WWW_AUTHENTICATE,
*
* Return(s)   : DEF_YES,   If the header field needs to be process.
*
*               DEF_NO,    Otherwise.
*
* Caller(s)   : HTTPs_ReqHdrParse() via 'p_cfg->ReqHdrRxFnctPtr()'.
*
* Note(s)     : (1) The instance structure is for read-only. It MUST NOT be modified.
*
*               (2) The connection structure SHOULD NOT be modified. It should be only read to determine if the header
*                   type must be stored.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  HTTPs_InstanceReqHdrRx (const  HTTPs_INSTANCE   *p_instance,
                                             const  HTTPs_CONN       *p_conn,
                                                    HTTPs_HDR_FIELD   hdr_field)
{
#if (HTTPs_CFG_HDR_EN == DEF_ENABLED)
    switch (hdr_field) {
        case HTTPs_HDR_FIELD_COOKIE:
             return(DEF_YES);


        case HTTPs_HDR_FIELD_COOKIE2:
             return(DEF_YES);


        default:
             break;
    }
#endif

    (void)&p_instance;                                           /* Prevent 'variable unused' compiler warning.          */
    (void)&p_conn;

    return (DEF_NO);
}


/*
*********************************************************************************************************
*                                       HTTPs_InstanceRespHdrTx()
*
* Description : Called each time the HTTP server is building a response message. Allows for adding header
*               fields to the response message according to the application needs.
*
* Argument(s) : p_instance  Pointer to the HTTP instance structure.
*
*               p_conn      Pointer to the connection control structure.
*
* Return(s)   : DEF_YES,    if the header fields are added without running into a error.
*
*               DEF_NO,     otherwise.
*
* Caller(s)   : HTTPs_RespPrepareHdr() via 'p_cfg->RespHdrTxFnctPtr()'.
*
* Note(s)     : (1) The instance structure MUST NOT be modified.
*
*               (2) The connection structure MUST NOT be modified manually since the response is about to be
*                   transmitted at this point. The only change to the connection structure should be the
*                   addition of header fields for the response message through the function HTTPs_RespHdrGet().
*********************************************************************************************************
*/

static  CPU_BOOLEAN  HTTPs_InstanceRespHdrTx (HTTPs_INSTANCE   *p_instance,
                                              HTTPs_CONN       *p_conn)
{
#if (HTTPs_CFG_HDR_EN == DEF_ENABLED)

           HTTPs_HDR_BLK  *p_resp_hdr_blk;
    const  HTTPs_CFG      *p_cfg;
           CPU_CHAR       *str_data;
           CPU_SIZE_T      str_len;
           HTTPs_ERR       http_err;


    p_cfg = p_instance->CfgPtr;

    switch (p_conn->StatusCode) {
           case HTTPs_STATUS_OK:

                if (p_conn->FileContentType == HTTPs_CONTENT_TYPE_HTML) {
                                                                /* --------------- ADD COOKIE HDR FIELD --------------- */
                    p_resp_hdr_blk = HTTPs_RespHdrGet(p_instance,   /* Get and add header block to the connection.      */
                                                      p_conn,
                                                      HTTPs_HDR_FIELD_SET_COOKIE,
                                                      HTTPs_HDR_VAL_TYPE_STR_DYN,
                                                     &http_err);
                    if (p_resp_hdr_blk == (HTTPs_HDR_BLK *)0) {
                        return(DEF_FAIL);
                    }

                    str_data = "user=micrium";                  /* Build Cookie string value.                           */

                    str_len = Str_Len_N(str_data, p_cfg->RespHdrStrLenMax);

                                                                /* update hdr blk parameter.                            */
                    Str_Copy_N(p_resp_hdr_blk->ValPtr,
                               str_data,
                               str_len);

                    p_resp_hdr_blk->ValLen = str_len;

                                                                /* --------------- ADD SERVER HDR FIELD --------------- */
                    p_resp_hdr_blk = HTTPs_RespHdrGet(p_instance,   /* Get and add header block to the connection.      */
                                                      p_conn,
                                                      HTTPs_HDR_FIELD_SERVER,
                                                      HTTPs_HDR_VAL_TYPE_STR_DYN,
                                                     &http_err);
                    if (p_resp_hdr_blk == (HTTPs_HDR_BLK *)0) {
                        return(DEF_FAIL);
                    }

                    str_data = "uC-HTTPs V2.00.00";             /* Build Server string value.                           */

                    str_len = Str_Len_N(str_data, p_cfg->RespHdrStrLenMax);

                                                                /* update hdr blk parameter.                            */
                    Str_Copy_N(p_resp_hdr_blk->ValPtr,
                               str_data,
                               str_len);

                    p_resp_hdr_blk->ValLen = str_len;
                }
                break;


           default:
                break;
    }
#endif

    return (DEF_YES);
}


/*
*********************************************************************************************************
*                                       HTTPs_InstanceCGI_Post()
*
* Description : Called each time a form has been posted and all control key value pairs have been received.
*
* Argument(s) : p_instance  Pointer to the HTTP instance structure.
*
*               p_conn      Pointer to the connection control structure.
*
*               p_data      Pointer to the first control key value pair.
*
* Return(s)   : DEF_YES, if the response can be sent.
*
*               DEF_NO,  if the response cannot be sent after this call and CGI Poll function MUST be called before
*                        sending the response (see note #3).
*
* Caller(s)   : HTTPs_MethodPost() via 'p_cfg->CGI_PostFnctPtr()'.
*
* Note(s)     : (1) This callback function SHOULD NOT be blocking and SHOULD return quickly. A time consuming
*                   function will block the processing of other connections and reduce the HTTP server performance.
*
*               (2) If CGI data take a while to be processed:
*
*                   (a) the processing SHOULD be done in a separate task and not in this callback function to avoid
*                       blocking other connections.
*
*                   (b) the poll callback function SHOULD be used to allow the connection to poll periodically the
*                       upper application and verify if the CGI data processing has been completed.
*
*                       The 'ConnDataPtr' attribute inside HTTP_CONN structure can be used to store a
*                       semaphore pointer related to the completion of the CGI processing.
*
*                       See 'HTTPs_InstanceCGI_Poll()' for more details on poll function.
*
*               (3) The following connection attributes can be accessed to analyze the connection:
*
*                   (a) 'ClientAddr'
*
*                           This connection parameter contains the IP address and port used by the remote client to access the
*                           server instance.
*
*                   (b) 'Method'
*
*                           HTTPs_METHOD_GET        Get  request
*                           HTTPs_METHOD_POST       Post request
*                           HTTPs_METHOD_HEAD       Head request
*
*                   (c) 'FileNamePtr'
*
*                           This is a pointer to the string that contains the name of the file requested.
*
*                   (d) 'ReqHdrCtr'
*
*                           This parameter is a counter of the number of header field that has been stored.
*
*                   (e) 'ReqHdrFirstPtr'
*
*                           This parameter is a pointer to the first header field stored. A linked list is created with
*                           all header field stored.
*
*                   (f) 'ConnDataPtr'
*
*                           This is a pointer available for the upper application when memory block must be allocated
*                           to process the connection request. If memory is allocated by the upper application, the memory
*                           space can be deallocated into another hook function.
*
*               (4) In this hook function, only the under-mentioned connection parameters are allowed
*                   to be modified :
*
*                   (a) 'StatusCode'
*
*                           HTTPs_STATUS_OK,
*                           HTTPs_STATUS_CREATED,
*                           HTTPs_STATUS_ACCEPTED,
*                           HTTPs_STATUS_NO_CONTENT,
*                           HTTPs_STATUS_RESET_CONTENT,
*                           HTTPs_STATUS_MOVED_PERMANENTLY,
*                           HTTPs_STATUS_FOUND,
*                           HTTPs_STATUS_SEE_OTHER,
*                           HTTPs_STATUS_NOT_MODIFIED,
*                           HTTPs_STATUS_USE_PROXY,
*                           HTTPs_STATUS_TEMPORARY_REDIRECT,
*                           HTTPs_STATUS_BAD_REQUEST,
*                           HTTPs_STATUS_UNAUTHORIZED,
*                           HTTPs_STATUS_FORBIDDEN,
*                           HTTPs_STATUS_NOT_FOUND,
*                           HTTPs_STATUS_METHOD_NOT_ALLOWED,
*                           HTTPs_STATUS_NOT_ACCEPTABLE,
*                           HTTPs_STATUS_REQUEST_TIMEOUT,
*                           HTTPs_STATUS_CONFLIT,
*                           HTTPs_STATUS_GONE,
*                           HTTPs_STATUS_LENGTH_REQUIRED,
*                           HTTPs_STATUS_PRECONDITION_FAILED,
*                           HTTPs_STATUS_REQUEST_ENTITY_TOO_LARGE,
*                           HTTPs_STATUS_REQUEST_URI_TOO_LONG,
*                           HTTPs_STATUS_UNSUPPORTED_MEDIA_TYPE,
*                           HTTPs_STATUS_REQUESTED_RANGE_NOT_SATISFIABLE,
*                           HTTPs_STATUS_EXPECTATION_FAILED,
*                           HTTPs_STATUS_INTERNAL_SERVER_ERR,
*                           HTTPs_STATUS_NOT_IMPLEMENTED,
*                           HTTPs_STATUS_SERVICE_UNAVAILABLE,
*                           HTTPs_STATUS_HTTP_VERSION_NOT_SUPPORTED,
*                           HTTPs_STATUS_UNKOWN
*
*                   (b) 'FileNamePtr'
*
*                           This is a pointer to the string that contains the name of the file requested. You can change
*                           the name of the requested file to send another file instead without error.
*
*                   (c) 'FilePtr'
*
*                           This is a pointer to the data file or data to transmit. This parameter should be null when calling
*                           this function. If data from memory has to be sent instead of a file, this pointer must be set
*                           to the location of the data.
*
*                   (d) 'FileType'
*
*                           HTTPs_FILE_TYPE_FS              Open and transmit a file. Value by default.*
*                           HTTPs_FILE_TYPE_STATIC_DATA     Transmit data from the memory. Must be set by the hook function.
*
*                   (e) 'FileLen'
*
*                           0,                              Default value, will be set when the file is opened.
*                           Data length,                    Must be set by the hook function when transmitting data from
*                                                           the memory
*
*               (5) When the Location of the requested file has change, besides the Status Code to change (3xx),
*                   the FileNamePtr parameter needs to be update. A "Location" header will be added automatically in
*                   the response by uC/HTTPs core with the new location.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  HTTPs_InstanceCGI_Post (const  HTTPs_INSTANCE  *p_instance,
                                                    HTTPs_CONN      *p_conn,
                                             const  HTTPs_CGI_DATA  *p_data)
{
            HTTPs_CGI_DATA  *p_ctrl_var;
    static  CPU_CHAR         buf[4];
            CPU_INT16S       str_cmp;
            CPU_SIZE_T       str_len;
    static  CPU_BOOLEAN      led1_val;
    static  CPU_BOOLEAN      led2_val;
            CPU_BOOLEAN      led_val;


    p_ctrl_var = (HTTPs_CGI_DATA *)p_data;
    while (p_ctrl_var != DEF_NULL) {

                                                                /* **************** RECEIVED CTRL TYPE **************** */
        if (p_ctrl_var->DataType == HTTPs_CGI_DATA_TYPE_CTRL_VAL) {

            str_cmp = Str_Cmp_N(p_ctrl_var->CtrlNamePtr, "LED", p_ctrl_var->CtrlNameLen);
            if (str_cmp == 0) {

                str_cmp = Str_Cmp_N(p_ctrl_var->ValPtr, "LED1", p_ctrl_var->ValLen);
                if (str_cmp == 0) {
                    led1_val = !led1_val;                       /* Toggle led1 value.                                   */
                    led_val  = led1_val;
                                                                /* Add board specific code to toggle led1.              */
                }

                str_cmp = Str_Cmp_N(p_ctrl_var->ValPtr, "LED2", p_ctrl_var->ValLen);
                if (str_cmp == 0) {
                    led2_val = !led2_val;                       /* Toggle led2 value.                                   */
                    led_val  =  led2_val;
                                                                /* Add board specific code to toggle led2.              */
                }

                Str_Copy(p_conn->FileNamePtr, "led.txt");       /* Set filename of data to send.                        */
                                                                /* The extension of the filename will be use to set ... */
                                                                /* ... the content-type in http response.               */

                p_conn->FileType = HTTPs_FILE_TYPE_STATIC_DATA;

                if (led_val == DEF_ON) {
                    str_len          = Str_Len("ON");
                    Str_Copy(&buf[0], "ON");
                } else {
                    str_len          = Str_Len("OFF");
                    Str_Copy(&buf[0], "OFF");
                }
                p_conn->FilePtr       = &buf[0];                /* Set FilePtr to point towards data to send.           */
                p_conn->FileLen       = str_len;
                p_conn->FileFixPosCur = 0;                      /* Reset position cursor to start of data.              */
            }

        } else if (p_ctrl_var->DataType == HTTPs_CGI_DATA_TYPE_FILE) {
                                                                /* Send back in response last file received in post.    */
            Str_Copy(p_conn->FileNamePtr, p_ctrl_var->ValPtr);
            p_conn->FileType = HTTPs_FILE_TYPE_FS;
        }

        p_ctrl_var = p_ctrl_var->DataNextPtr;
    }

    (void)&p_instance;                                          /* Prevent 'variable unused' compiler warning.          */


    return (DEF_YES);
}


/*
*********************************************************************************************************
*                                       HTTPs_InstanceCGI_Poll()
*
* Description : Called periodically by a connection waiting for the upper application to complete the CGI
*               data processing.
*
* Argument(s) : p_instance  Pointer to the HTTP instance structure.
*
*               p_conn      Pointer to the connection control structure.
*
* Return(s)   : DEF_YES, if the response can be sent (CGI processing complete).
*
*               DEF_NO,  if the response cannot be sent after this call and CGI Poll function MUST be called again
*                        before sending the response (see note #2).
*
* Caller(s)   : HTTPs_MethodPost() via 'p_cfg->CGI_PollFnctPtr()'.
*
* Note(s)     : (1) This callback function SHOULD NOT be blocking and SHOULD return quickly. A time consuming
*                   function will block the processing of other connections and reduce the HTTP server performance.
*
*                   This function will be called periodically by the connection until DEF_YES is returned.
*
*               (2) The poll callback function SHOULD be used when the CGI data processing takes a while to
*                   be completed. It will allow the server to periodically poll the upper application to verify
*                   if the CGI data processing has finished.
*
*                   The 'ConnUDataPtr' attribute inside the HTTP_CONN structure can be used to store a
*                   semaphore pointer related to the completion of the CGI procesing.
*
*                   See 'HTTPs_InstanceCGI_Post()' for more details on post/poll functionality.
*
*               (3) The following connection attributes can be accessed to analyze the connection:
*
*                   (a) 'ClientAddr'
*
*                           This connection parameter contains the IP address and port used by the remote client to access the
*                           server instance.
*
*                   (b) 'Method'
*
*                           HTTPs_METHOD_GET        Get  request
*                           HTTPs_METHOD_POST       Post request
*                           HTTPs_METHOD_HEAD       Head request
*
*                   (c) 'FileNamePtr'
*
*                           This is a pointer to the string that contains the name of the file requested.
*
*                   (d) 'ReqHdrCtr'
*
*                           This parameter is a counter of the number of header field that has been stored.
*
*                   (e) 'ReqHdrFirstPtr'
*
*                           This parameter is a pointer to the first header field stored. A linked list is created with
*                           all header field stored.
*
*                   (f) 'ConnDataPtr'
*
*                           This is a pointer available for the upper application when memory block must be allocated
*                           to process the connection request. If memory is allocated by the upper application, the memory
*                           space can be deallocated into another hook function.
*
*               (4) In this hook function, only the under-mentioned connection parameters are allowed
*                   to be modified :
*
*                   (a) 'StatusCode'
*
*                           HTTPs_STATUS_OK,
*                           HTTPs_STATUS_CREATED,
*                           HTTPs_STATUS_ACCEPTED,
*                           HTTPs_STATUS_NO_CONTENT,
*                           HTTPs_STATUS_RESET_CONTENT,
*                           HTTPs_STATUS_MOVED_PERMANENTLY,
*                           HTTPs_STATUS_FOUND,
*                           HTTPs_STATUS_SEE_OTHER,
*                           HTTPs_STATUS_NOT_MODIFIED,
*                           HTTPs_STATUS_USE_PROXY,
*                           HTTPs_STATUS_TEMPORARY_REDIRECT,
*                           HTTPs_STATUS_BAD_REQUEST,
*                           HTTPs_STATUS_UNAUTHORIZED,
*                           HTTPs_STATUS_FORBIDDEN,
*                           HTTPs_STATUS_NOT_FOUND,
*                           HTTPs_STATUS_METHOD_NOT_ALLOWED,
*                           HTTPs_STATUS_NOT_ACCEPTABLE,
*                           HTTPs_STATUS_REQUEST_TIMEOUT,
*                           HTTPs_STATUS_CONFLIT,
*                           HTTPs_STATUS_GONE,
*                           HTTPs_STATUS_LENGTH_REQUIRED,
*                           HTTPs_STATUS_PRECONDITION_FAILED,
*                           HTTPs_STATUS_REQUEST_ENTITY_TOO_LARGE,
*                           HTTPs_STATUS_REQUEST_URI_TOO_LONG,
*                           HTTPs_STATUS_UNSUPPORTED_MEDIA_TYPE,
*                           HTTPs_STATUS_REQUESTED_RANGE_NOT_SATISFIABLE,
*                           HTTPs_STATUS_EXPECTATION_FAILED,
*                           HTTPs_STATUS_INTERNAL_SERVER_ERR,
*                           HTTPs_STATUS_NOT_IMPLEMENTED,
*                           HTTPs_STATUS_SERVICE_UNAVAILABLE,
*                           HTTPs_STATUS_HTTP_VERSION_NOT_SUPPORTED,
*                           HTTPs_STATUS_UNKOWN
*
*                   (b) 'FileNamePtr'
*
*                           This is a pointer to the string that contains the name of the file requested. You can change
*                           the name of the requested file to send another file instead without error.
*
*                   (c) 'FilePtr'
*
*                           This is a pointer to the data file or data to transmit. This parameter should be null when calling
*                           this function. If data from memory has to be sent instead of a file, this pointer must be set
*                           to the location of the data.
*
*                   (d) 'FileType'
*
*                           HTTPs_FILE_TYPE_FS              Open and transmit a file. Value by default.*
*                           HTTPs_FILE_TYPE_STATIC_DATA     Transmit data from the memory. Must be set by the hook function.
*
*                   (e) 'FileLen'
*
*                           0,                              Default value, will be set when the file is opened.
*                           Data length,                    Must be set by the hook function when transmitting data from
*                                                           the memory
*
*               (5) When the Location of the requested file has change, besides the Status Code to change (3xx),
*                   the FileNamePtr parameter needs to be update. A "Location" header will be added automatically in
*                   the response by uC/HTTPs core with the new location.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  HTTPs_InstanceCGI_Poll (const  HTTPs_INSTANCE  *p_instance,
                                                    HTTPs_CONN      *p_conn)
{

    (void)&p_instance;                                          /* Prevent 'variable unused' compiler warning.          */
    (void)&p_conn;

    return (DEF_YES);
}


/*
*********************************************************************************************************
*                                      HTTPs_InstanceTokenValGet()
*
* Description : Called for each ${TEXT_STRING} embedded variable found in a HTML document.
*
* Argument(s) : p_instance   Pointer to the HTTP instance structure (see note #1).
*
*               p_conn       Pointer to the connection control structure (see note #2).
*
*               p_token      Pointer to the string that contains the value of the HTML embedded token.
*
*               token_len    Length of the embedded token.
*
*               p_val        Pointer to which buffer token value is copied to.
*
*               val_len_max  Maximum buffer length.
*
* Return(s)   : DEF_OK,   if token value copied successfully.
*
*               DEF_FAIL, otherwise (see Note #3).
*
* Caller(s)   : HTTPs_TokenValGet() via 'p_cfg->TokenValGetFnctPtr()'.
*
* Note(s)     : (1) The instance structure MUST NOT be modified.
*
*               (2) The connection structure MUST NOT be modified manually since the response is about to be
*                   transmitted at this point. The only change to the connection structure should be the
*                   addition of header fields for the response message through the function HTTPs_RespHdrGet().
*
*               (3) If the token replacement failed, the token will be replaced by a line of tilde (~) of
*                   length equal to val_len_max.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  HTTPs_InstanceTokenValGet (const  HTTPs_INSTANCE  *p_instance,
                                                const  HTTPs_CONN      *p_conn,
                                                const  CPU_CHAR        *p_token,
                                                       CPU_INT16U       token_len,
                                                       CPU_CHAR        *p_val,
                                                       CPU_INT16U       val_len_max)
{
    static  CPU_CHAR    buf[20];
            CPU_INT32U  ver;


    if (Str_Cmp_N(p_token, "TEXT_STRING", 11) == 0) {
        Str_Copy_N(p_val, "Text", val_len_max);
        return (DEF_OK);
    }

    if (Str_Cmp_N(p_token, "OS_VERSION", 10) == 0) {

        ver =  OS_VERSION / 10000;
       (void)Str_FmtNbr_Int32U(ver,   2, DEF_NBR_BASE_DEC, ' ', DEF_NO, DEF_NO,  &buf[0]);
        buf[2] = '.';

        ver = (OS_VERSION /   100) % 100;
       (void)Str_FmtNbr_Int32U(ver,   2, DEF_NBR_BASE_DEC, '0', DEF_NO, DEF_NO,  &buf[3]);
        buf[5] = '.';

        ver = (OS_VERSION /    1) %  100;
       (void)Str_FmtNbr_Int32U(ver,   2, DEF_NBR_BASE_DEC, '0', DEF_NO, DEF_YES, &buf[6]);
        buf[8] = '\0';

    } else if(Str_Cmp_N(p_token, "NET_VERSION", 11) == 0) {
#if (NET_VERSION >  205u)
        ver =  NET_VERSION / 10000;
       (void)Str_FmtNbr_Int32U(ver,  2, DEF_NBR_BASE_DEC, ' ', DEF_NO, DEF_NO,  &buf[0]);
        buf[2] = '.';

        ver = (NET_VERSION /   100) % 100;
       (void)Str_FmtNbr_Int32U(ver,  2, DEF_NBR_BASE_DEC, '0', DEF_NO, DEF_NO,  &buf[3]);
        buf[5] = '.';

        ver = (NET_VERSION /     1) % 100;
       (void)Str_FmtNbr_Int32U(ver,  2, DEF_NBR_BASE_DEC, '0', DEF_NO, DEF_YES, &buf[6]);
        buf[8] = '\0';

#else
        ver =  NET_VERSION /   100;
       (void)Str_FmtNbr_Int32U(ver,  2, DEF_NBR_BASE_DEC, ' ', DEF_NO, DEF_NO,  &buf[0]);
        buf[2] = '.';

        ver = (NET_VERSION /     1) % 100;
       (void)Str_FmtNbr_Int32U(ver,  2, DEF_NBR_BASE_DEC, '0', DEF_NO, DEF_YES, &buf[3]);
        buf[5] = '\0';
#endif

    } else if(Str_Cmp_N(p_token, "HTTPs_VERSION", 13) == 0) {
        ver =  HTTPs_VERSION / 10000;
       (void)Str_FmtNbr_Int32U(ver,  2, DEF_NBR_BASE_DEC, ' ', DEF_NO, DEF_NO,  &buf[0]);
        buf[2] = '.';

        ver = (HTTPs_VERSION /   100) % 100;
       (void)Str_FmtNbr_Int32U(ver,  2, DEF_NBR_BASE_DEC, '0', DEF_NO, DEF_NO,  &buf[3]);
        buf[5] = '.';

        ver = (HTTPs_VERSION /     1) % 100;
       (void)Str_FmtNbr_Int32U(ver,  2, DEF_NBR_BASE_DEC, '0', DEF_NO, DEF_YES, &buf[6]);
        buf[8] = '\0';
    }

    Str_Copy_N(p_val, &buf[0], val_len_max);


    (void)&p_instance;                                          /* Prevent 'variable unused' compiler warning.          */
    (void)&p_conn;
    (void)&token_len;

    return (DEF_OK);
}

