/*
*********************************************************************************************************
*                                              uC/HTTPs
*                                 Hypertext Transfer Protocol (server)
*
*                          (c) Copyright 2004-2013; Micrium, Inc.; Weston, FL
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
*               You can find our product's user manual, API reference, release notes and
*               more information at: https://doc.micrium.com
*
*               You can contact us at: http://www.micrium.com
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                             HTTP SERVER
*
* Filename      : http-s.h
* Version       : V2.10.01
* Programmer(s) : AA
*                 MM
*********************************************************************************************************
* Note(s)       : (1) Assumes the following versions (or more recent) of software modules are included in
*                     the project build :
*
*                         (a) uC/CPU    V1.27
*                         (b) uC/LIB    V1.37.00
*                         (a) uC/TCP-IP V2.13.02
*
*                         See also 'HTTPs INCLUDE FILES  Notes #1, #2 & #3' in http-s.h.
*
*                 (2) Assume that at least one of the following file system is included in the project build
*
*                     (a) Real Dynamic file system
*
*                         Require a port for the file system  which should be located under FS folder of uC/TCP-IP
*
*                         See also 'HTTPs INCLUDE FILES  Notes #1b2' in http-s.h.
*
*                     (b) Static file system
*
*                         File system port is located FS folder of uC/HTTPs.
*
*                         See also 'HTTPs INCLUDE FILES  Notes #1c3' in http-s.h.
*
*                 (3) (a) This server support multiple instances.
*
*                         (1) One task is created per instance.
*                         (2) It's possible to start and stop an instance once an instance has been initialized.
*                         (3) Each instance must use different TCP port. Not possible to have many started instance
*                             at same time using the same port.
*
*                 (5) Multiple connection is supported. Most of known browser can open up to 15 simultaneous connections.
*                     This server is designed to server all connections in parallel in an instance single task.
*
*                 (4) (a) These following HTTP version should be compatible with this server:
*
*                         (1) HTTP 0.9
*                         (2) HTTP 1.0
*                         (3) HTTP 1.1
*
*                     (b) The following HTTP method options are supported/implemented :
*
*                         (1) GET       The GET method means retrieve whatever information (in the form of an
*                                       entity) is identified by the Request-URI.
*
*                             See also Note #3.
*
*                         (2) HEAD      The HEAD method is identical to GET except that the server MUST NOT
*                                       return a message-body (file) in the response.
*
*                         (3) POST      The POST method is used to request that the origin server accept the
*                                       entity enclosed in the request as a new subordinate of the resource
*                                       identified by the Request-URI in the Request-Line.
*
*                             See also Notes #4.
*
*                     (c) The following HTTPs method options are intentionally NOT supported/implemented :
*
*                         (a) PUT       The PUT method requests that the enclosed entity be stored under the
*                                       supplied Request-URI.
*
*                         (b) DELETE    The DELETE method requests that the origin server delete the resource
*                                       identified by the Request-URI.
*
*                         (c) TRACE     The TRACE method is used to invoke a remote, application-layer loop-
*                                       back of the request message.
*
*                         (d) CONNECT   This specification reserves the method name CONNECT for use with a
*                                       proxy that can dynamically switch to being a tunnel.
*
*                     (d) This server is designed to work with an embedded system and it should not be able to
*                         use any advanced "niceties" such as PERL and PHP, that are available on larger
*                         UNIX or NT environments.
*
*                 (5) For each method type a request is sent to the server and the following features are
*                     supported:
*
*                     (a) Two type of data can be sent back to the client either from a file located in a file system
*                         and from a memory location.
*
*                     (b) Prior to send any response a callback function can be automatically called to let the upper
*                         application authenticate the remote user and allow or not the access to the requested resource.
*                         The upper application can also in from this callback function change the file to send
*                         back or even tell the web server to send static data.
*
*                     (c) Each type of error are reported to the upper application via a callback function. Following
*                         the error type the upper application set the web page and change the status code.
*
*                     (d) Dynamic content replacement is supported using token :
*
*                         (1) uC/HTTPs supports external dynamic contents in text-based files (HTML, plain text, JS).
*
*                             When a request is received asking for a text-based file, the server will first parse
*                             the file prior to sending it to the client. Any occurrence of a token of the form :
*                             ${TEXT_STRING}, will trigger a callback function defined at the application level. This
*                             function will return the specific value for TEXT_STRING with which the token will be
*                             replace.
*
*                             Now a simple example of the above discussion.
*                             Assume we have a file called myip.html on the target that looks like this:
*
*                             <html>
*                             <body>
*                               This system's IP address is ${My_IP_Address}
*                             </body>
*                             </html>
*
*                             When a client attaches to this server and requests myip.html, the server will
*                             parse the file, find the ${My_IP_Address} token and pass the string "My_IP_Address"
*                             into an application specific function called HTTPs_InstanceTokenValReq(). That function
*                             will then build a replacement text string that the server will give to the client in
*                             place of the ${My_IP_Address} text. The file seen by the client would look something
*                             like this:
*
*                             <html>
*                             <body>
*                               This system's IP address is 135.17.115.215
*                             </body>
*                             </html>
*
*                             Note that this server does not define any syntax within the ${}.  This is 100%
*                             application-specific and can be used for simple variable name conversion or something
*                             more elaborate if necessary.
*
*                         (2) uC/HTTPs includes internal build-in token substitution. The string of form #{} is reserved
*                             to the web server. It will replace automatically these token type with value specific to
*                             the web server. It can be use in any text-based file to display web server value. The
*                             following tokens are supported:
*
*                                   #{STATUS_CODE}      Replaced by status code number of the current connection.
*                                   #{REASON_PHRASE}    Replaced by the reason phrase based on the status code of the
*                                                       current connection
*
*
*                         (3) External dynamic content replacement can be enabled or disabled using the
*                             following configuration value:
*
*                               HTTPs_CFG_TOKEN_PARSE_EN, in http-s_cfg.h, is used to disable the token
*                               replacement for all web server instance.
*
*                               TokenParseEn, in the instance configuration structure, is used to disable
*                               the token replacement of the specific web server instance.
*
*
*                 (6) (1) uC-HTTPs supports HTML form submissions through the POST method.
*
*                     (2) The following POST Internet media type are supported:
*
*                         (a) "application/x-www-form-urlencoded"
*
*                             This is the default format for encoding key-value pairs when a web browser sends
*                             a POST request from a web form element.
*
*                         (b) "multipart/form-data"
*
*                             This is the format that must be used when uploading file on the web server,
*                             using a web browser or not. It must be specified in the form tag as example:
*
*                               <form action="upload.html" ENCTYPE="multipart/form-data" method="post">
*
*                     (3) Each key pair value are stored internally into a list, when all key pair value have
*                         been received the list is posted to the upper application using a callback function
*                         called 'GGI post' function.
*
*                         The post function must not not be a blocking function since all other simultaneous
*                         connection will be stopped until the post function return.
*
*                         If the processing the form require a significant amount  of time, the server can call
*                         periodically another callback function to poll the status of the processing of the
*                         post data. This callback function is called 'CGI poll' function.
*
*                     (4) The file upload submitted using "multipart/form-data" POST Internet media type is
*                         supported when correctly enabled. All received file are stored in the default folder
*                         specified in the instance configuration. A key pair value is added to the POST data
*                         list to specify the location of the file. The file can be moved by the upper
*                         application when "CGI post" function called.
*
*                     (5) Dynamic content is also supported for POST request, see Note #3.
*
*                 (7) This web server support Header field as defined in HTTP V1.1 for request and response.
*
*                     (1) For each header field received with the request (request header) and not necessary
*                         for the web server a callback function is called to know if the field must be kept.
*                         The web server create a list of header field that contains all field that must be
*                         retained until the connection is completed and closed.
*
*                     (2) The upper application can add header field that will be send with the response.
*                         Response header field can be added from any callback function via a call to the
*                         function HTTPs_RespHdrGet().
*
*                 (8) Secure web server, known as https or ssl/tls, is supported if security stack with the secure
*                     port for uC/TCPIP is present in the project. To access the web server using the default
*                     address (https://...) the web server must be configured to use the default http secure port:
*                     443.
*
*                     See also 'HTTPs INCLUDE FILES  Notes #1b3' in http-s.h.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*********************************************************************************************************
*                                               MODULE
*
* Note(s) : (1) This main network protocol suite header file is protected from multiple pre-processor
*               inclusion through use of the HTTP module present pre-processor macro definition.
*
*               See also 'HTTPs INCLUDE FILES  Note #5'.
*********************************************************************************************************
*********************************************************************************************************
*/

#ifndef  HTTPs_PRESENT                                          /* See Note #1.                                         */
#define  HTTPs_PRESENT


/*
*********************************************************************************************************
*********************************************************************************************************
*                                        HTTPs VERSION NUMBER
*
* Note(s) : (1) (a) The HTTPs module software version is denoted as follows :
*
*                       Vx.yy.zz
*
*                           where
*                                   V               denotes 'Version' label
*                                   x               denotes     major software version revision number
*                                   yy              denotes     minor software version revision number
*                                   zz              denotes sub-minor software version revision number
*
*               (b) The HTTPs software version label #define is formatted as follows :
*
*                       ver = x.yyzz * 100 * 100
*
*                           where
*                                   ver             denotes software version number scaled as an integer value
*                                   x.yyzz          denotes software version number, where the unscaled integer
*                                                       portion denotes the major version number & the unscaled
*                                                       fractional portion denotes the (concatenated) minor
*                                                       version numbers
*********************************************************************************************************
*********************************************************************************************************
*/

#define  HTTPs_VERSION                                 21001u   /* See Note #1.                                         */


/*
*********************************************************************************************************
*********************************************************************************************************
*                                         HTTPs INCLUDE FILES
*
* Note(s) : (1) The HTTPs module files are located in the following directories :
*
*               (a) (1) \<Your Product Application>\http-s_cfg.h
*                   (2)                            \http-s_instance_cfg.*
*
*               (b) (1) \<Network Protocol Suite>\Source\net_*.*
*
*                   (2) If a dynamic file system is used:
*
*                       (A) \<Network Protocol Suite>\FS\<fs>\net_fs_<fs>.*
*
*                   (3) If network security manager is to be used:
*
*                       (A) \<Network Protocol Suite>\Secure\<Network Security Suite>\net_secure.*
*
*               (c) (1) \<HTTPs>\Source\http-s.*
*                                      \http-s_mem.*
*
*                   (2) \<HTTPs>\OS\<os>\http-s_os.*
*
*                   (3) If a dynamic file system is not used:
*
*                       (A) \<HTTPs>\FS\Static\http-s_fs_static.*
*
*
*               where
*                   <Your Product Application>      directory path for Your Product's Application
*                   <Network Protocol Suite>        directory path for network protocol suite
*                   <HTTPs>                         directory path for HTTPs module
*                   <os>                            directory name for specific operating system (OS)
*
*           (2) CPU-configuration software files are located in the following directories :
*
*               (a) \<CPU-Compiler Directory>\cpu_*.*
*               (b) \<CPU-Compiler Directory>\<cpu>\<compiler>\cpu*.*
*
*                       where
*                               <CPU-Compiler Directory>        directory path for common CPU-compiler software
*                               <cpu>                           directory name for specific processor (CPU)
*                               <compiler>                      directory name for specific compiler
*
*           (3) NO compiler-supplied standard library functions SHOULD be used.
*
*               (a) Standard library functions are implemented in the custom library module(s) :
*
*                       \<Custom Library Directory>\lib_*.*
*
*                           where
*                                   <Custom Library Directory>      directory path for custom library software
*
*           (4) Compiler MUST be configured to include as additional include path directories :
*
*               (a) '\<Your Product Application>\'                                          See Note #1a
*
*               (b) (1) '\<Network Protocol Suite>\                                         See Note #1b1
*
*                   (2) '\<Network Protocol Suite>\Secure\<Network Security Suite>\'        See Note #1b2A
*
*               (c) '\<HTTPs>\' directories                                                 See Note #1c
*
*               (d) (1) '\<CPU-Compiler Directory>\'                                        See Note #2a
*                   (2) '\<CPU-Compiler Directory>\<cpu>\<compiler>\'                       See Note #2b
*
*               (e) '\<Custom Library Directory>\'                                          See Note #3a
*
*           (5) An application pre-processor MUST include ONLY this main HTTPs server header file, 'net.h'.
*               All other network protocol suite files are included via this main HTTP server header file.
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <cpu.h>                                               /* CPU Configuration              (see Note #2b)        */
#include  <cpu_core.h>                                          /* CPU Core Library               (see Note #2a)        */

#include  <lib_def.h>                                           /* Standard        Defines        (see Note #3a)        */
#include  <lib_str.h>                                           /* Standard String Library        (see Note #3a)        */
#include  <lib_ascii.h>                                         /* Standard ASCII  Library        (see Note #3a)        */

#include  <http-s_cfg.h>
#include  <FS/net_fs.h>                                         /* File System Interface          (see Note #1c3)       */

#include  <Source/net.h>                                        /* Network Protocol Suite         (see Note #1b)        */
#include  <Source/net_cfg_net.h>
#include  <Source/net_app.h>
#include  <Source/net_sock.h>
#include  <Source/net_tcp.h>
#include  <Source/net_tmr.h>

/*
*********************************************************************************************************
*********************************************************************************************************
*                                               EXTERNS
*********************************************************************************************************
*********************************************************************************************************
*/

#if ((defined(HTTPs_MODULE)) && \
     (defined(HTTPs_GLOBALS_EXT)))
#define  HTTPs_EXT
#else
#define  HTTPs_EXT  extern
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                       PRE CONFIGURATION ERROR
*********************************************************************************************************
*********************************************************************************************************
*/

#ifndef  HTTPs_CFG_CTR_STAT_EN
    #error  "HTTPs_CFG_CTR_STAT_EN                         not #define'd in 'http-s_cfg.h'"
    #error  "                                     [MUST be  DEF_DISABLED]                 "
    #error  "                                     [     ||  DEF_ENABLED ]                 "
#elif  ((HTTPs_CFG_CTR_STAT_EN != DEF_ENABLED ) && \
        (HTTPs_CFG_CTR_STAT_EN != DEF_DISABLED))
    #error  "HTTPs_CFG_CTR_STAT_EN                   illegally #define'd in 'http-s_cfg.h'"
    #error  "                                     [MUST be  DEF_DISABLED]                 "
    #error  "                                     [     ||  DEF_ENABLED ]                 "
#endif



#ifndef  HTTPs_CFG_CTR_ERR_EN
    #error  "HTTPs_CFG_CTR_ERR_EN                          not #define'd in 'http-s_cfg.h'"
    #error  "                                     [MUST be  DEF_DISABLED]                 "
    #error  "                                     [     ||  DEF_ENABLED ]                 "
#elif  ((HTTPs_CFG_CTR_ERR_EN != DEF_ENABLED ) && \
        (HTTPs_CFG_CTR_ERR_EN != DEF_DISABLED))
    #error  "HTTPs_CFG_CTR_ERR_EN                    illegally #define'd in 'http-s_cfg.h'"
    #error  "                                     [MUST be  DEF_DISABLED]                 "
    #error  "                                     [     ||  DEF_ENABLED ]                 "
#endif



#ifndef  HTTPs_CFG_TOKEN_PARSE_EN
    #error  "HTTPs_CFG_TOKEN_PARSE_EN                 not #define'd in 'http-s_cfg.h'"
    #error  "                             [MUST be  DEF_DISABLED]                    "
    #error  "                             [     ||  DEF_ENABLED ]                    "
#elif  ((HTTPs_CFG_TOKEN_PARSE_EN != DEF_ENABLED ) && \
        (HTTPs_CFG_TOKEN_PARSE_EN != DEF_DISABLED))
    #error  "HTTPs_CFG_TOKEN_PARSE_EN           illegally #define'd in 'http-s_cfg.h'"
    #error  "                             [MUST be  DEF_DISABLED]                    "
    #error  "                             [     ||  DEF_ENABLED ]                    "
#endif



#ifndef  HTTPs_CFG_ABSOLUTE_URI_EN
    #error  "HTTPs_CFG_ABSOLUTE_URI_EN                not #define'd in 'http-s_cfg.h'"
    #error  "                             [MUST be  DEF_DISABLED]                    "
    #error  "                             [     ||  DEF_ENABLED ]                    "
#elif  ((HTTPs_CFG_ABSOLUTE_URI_EN != DEF_ENABLED ) && \
        (HTTPs_CFG_ABSOLUTE_URI_EN != DEF_DISABLED))
    #error  "HTTPs_CFG_ABSOLUTE_URI_EN          illegally #define'd in 'http-s_cfg.h'"
    #error  "                             [MUST be  DEF_DISABLED]                    "
    #error  "                             [     ||  DEF_ENABLED ]                    "
#endif



#ifndef  HTTPs_CFG_CGI_EN
    #error  "HTTPs_CFG_CGI_EN                         not #define'd in 'http-s_cfg.h'"
    #error  "                             [MUST be  DEF_DISABLED]                    "
    #error  "                             [     ||  DEF_ENABLED ]                    "
#elif ((HTTPs_CFG_CGI_EN != DEF_ENABLED ) && \
       (HTTPs_CFG_CGI_EN != DEF_DISABLED))
    #error  "HTTPs_CFG_CGI_EN                   illegally #define'd in 'http-s_cfg.h'"
    #error  "                             [MUST be  DEF_DISABLED]                    "
    #error  "                             [     ||  DEF_ENABLED ]                    "


#elif   (HTTPs_CFG_CGI_EN == DEF_ENABLED)

    #ifndef  HTTPs_CFG_CGI_POLL_EN
        #error  "HTTPs_CFG_CGI_POLL_EN                         not #define'd in 'http-s_cfg.h'"
        #error  "                                  [MUST be  DEF_DISABLED]                    "
        #error  "                                  [     ||  DEF_ENABLED ]                    "
    #elif  ((HTTPs_CFG_CGI_POLL_EN != DEF_ENABLED ) && \
            (HTTPs_CFG_CGI_POLL_EN != DEF_DISABLED))
        #error  "HTTPs_CFG_CGI_POLL_EN                   illegally #define'd in 'http-s_cfg.h'"
        #error  "                                  [MUST be  DEF_DISABLED]                    "
        #error  "                                  [     ||  DEF_ENABLED ]                    "
    #endif


    #ifndef  HTTPs_CFG_CGI_MULTIPART_EN
        #error  "HTTPs_CFG_CGI_MULTIPART_EN                    not #define'd in 'http-s_cfg.h'"
        #error  "                                        [MUST be  DEF_DISABLED]              "
        #error  "                                        [     ||  DEF_ENABLED ]              "
    #elif  ((HTTPs_CFG_CGI_MULTIPART_EN != DEF_ENABLED ) && \
            (HTTPs_CFG_CGI_MULTIPART_EN != DEF_DISABLED))
        #error  "HTTPs_CFG_CGI_MULTIPART_EN              illegally #define'd in 'http-s_cfg.h'"
        #error  "                                        [MUST be  DEF_DISABLED]              "
        #error  "                                        [     ||  DEF_ENABLED ]              "


    #elif (HTTPs_CFG_CGI_MULTIPART_EN == DEF_ENABLED)


        #ifndef  HTTPs_CFG_CGI_FILE_UPLOAD_EN
            #error  "HTTPs_CFG_CGI_FILE_UPLOAD_EN                  not #define'd in 'http-s_cfg.h'"
            #error  "                                        [MUST be  DEF_DISABLED]              "
            #error  "                                        [     ||  DEF_ENABLED ]              "
        #elif  ((HTTPs_CFG_CGI_FILE_UPLOAD_EN != DEF_ENABLED ) && \
                (HTTPs_CFG_CGI_FILE_UPLOAD_EN != DEF_DISABLED))
            #error  "HTTPs_CFG_CGI_FILE_UPLOAD_EN            illegally #define'd in 'http-s_cfg.h'"
            #error  "                                        [MUST be  DEF_DISABLED]              "
            #error  "                                        [     ||  DEF_ENABLED ]              "
        #endif


    #endif


#endif


#ifndef HTTPs_CFG_HDR_EN
    #error  "HTTPs_CFG_HDR_EN                         not #define'd in 'http-s_cfg.h'"
    #error  "                             [MUST be  DEF_DISABLED]                    "
    #error  "                             [     ||  DEF_ENABLED ]                    "
#elif  ((HTTPs_CFG_HDR_EN != DEF_ENABLED ) && \
        (HTTPs_CFG_HDR_EN != DEF_DISABLED))
    #error  "HTTPs_CFG_HDR_EN                   illegally #define'd in 'http-s_cfg.h'"
    #error  "                             [MUST be  DEF_DISABLED]                    "
    #error  "                             [     ||  DEF_ENABLED ]                    "
#endif


#ifndef  HTTPs_CFG_HTML_DFLT_ERR_PAGE
    #error  "HTTPs_CFG_HTML_DFLT_ERR_PAGE                  not #define'd in 'http-s_cfg.h'"
    #error  "                                     [MUST be defined as a string]           "
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                                DEFINES
*********************************************************************************************************
*********************************************************************************************************
*/

#define  HTTPs_CFG_DFLT_PORT                                 80u
#define  HTTPs_CFG_DFLT_PORT_SECURE                         443u

#define  HTTPs_BUF_LEN_MIN                                  256u

#define  HTTPs_OS_LOCK_ACQUIRE_FAIL_DLY_MS                    5u


/*
*********************************************************************************************************
*                                     STATIC ERR FILE LEN DEFINES
*********************************************************************************************************
*/

#define  HTTPs_HTML_DLFT_ERR_LEN                            sizeof(HTTPs_CFG_HTML_DFLT_ERR_PAGE)
#define  HTTPs_HTML_DLFT_ERR_STR_NAME                       "default.html"


/*
*********************************************************************************************************
*                                            TOKEN DEFINES
*********************************************************************************************************
*/

#define  HTTPs_TOKEN_EXTENAL_CHAR_START                     ASCII_CHAR_DOLLAR_SIGN
#define  HTTPs_TOKEN_INTERNAL_CHAR_START                    ASCII_CHAR_NUMBER_SIGN
#define  HTTPs_TOKEN_CHAR_VAR_SEP_START                     ASCII_CHAR_LEFT_CURLY_BRACKET
#define  HTTPs_TOKEN_CHAR_VAR_SEP_END                       ASCII_CHAR_RIGHT_CURLY_BRACKET
#define  HTTPs_TOKEN_CHAR_OFFSET_LEN                        2u                              /* Length of '${'.          */
#define  HTTPs_TOKEN_CHAR_DFLT_VAL                          ASCII_CHAR_TILDE


/*
*********************************************************************************************************
*                                             CGI DEFINES
*********************************************************************************************************
*/

#define  HTTPs_CGI_BOUNDARY_STR_LEN_MAX                     72u

#define  HTTPs_STR_MULTIPART_CTRL_END_SEC                   STR_CR_LF STR_CR_LF
#define  HTTPs_STR_MULTIPART_CTRL_END_SEC_LEN               sizeof(HTTPs_STR_MULTIPART_CTRL_END_SEC) - 1;

#define  HTTPs_STR_MULTIPART_DATA_START                     "--"
#define  HTTPs_STR_MULTIPART_DATA_START_LEN                 sizeof(HTTPs_STR_MULTIPART_DATA_START) - 1;

#define  HTTPs_STR_MULTIPART_DATA_END                       STR_CR_LF "--"
#define  HTTPs_STR_MULTIPART_DATA_END_LEN                   sizeof(HTTPs_STR_MULTIPART_DATA_END) - 1;


/*
*********************************************************************************************************
*                                            SOCK DEFINES
*********************************************************************************************************
*/

#define  HTTPs_SOCK_RX_TH_MIN                               0u

#define  HTTPs_SOCK_OPEN_RETRY_MAX                          3u
#define  HTTPs_SOCK_OPEN_DLY_MS                             1u

#define  HTTPs_SOCK_ACCEPT_RETRY_MAX                        3u
#define  HTTPs_SOCK_ACCEPT_TIMEOUT_MS                       1u
#define  HTTPs_SOCK_ACCEPT_DLY_MS                           1u

#define  HTTPs_SOCK_BIND_RETRY_MAX                          3u
#define  HTTPs_SOCK_BIND_DLY_MS                             1u

#define  HTTPs_SOCK_SEL_TIMEOUT_MS                          1u

#define  HTTPs_SOCK_RX_RETRY_MAX                            3u
#define  HTTPs_SOCK_RX_TIMEOUT_MS                           1u
#define  HTTPs_SOCK_RX_DLY_MS                               1u

#define  HTTPs_SOCK_TX_RETRY_MAX                            3u
#define  HTTPs_SOCK_TX_TIMEOUT_MS                           1u
#define  HTTPs_SOCK_TX_DLY_MS                               1u

#define  HTTPs_SOCK_CLOSE_TIMEOUT_MS                        1u


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            ENUMERATIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                         ERRORS ENUMERATIONS
*********************************************************************************************************
*/

typedef enum https_err {
    HTTPs_ERR_NONE = 1,

    HTTPs_ERR_NULL_PTR,

    HTTPs_ERR_INIT,

    HTTPs_ERR_INSTANCE_INIT_POOL_REM_MEM,
    HTTPs_ERR_INSTANCE_INIT_POOL_CONN,
    HTTPs_ERR_INSTANCE_INIT_POOL_BUF,
    HTTPs_ERR_INSTANCE_INIT_POOL_FILENAME,
    HTTPs_ERR_INSTANCE_INIT_POOL_HOST,
    HTTPs_ERR_INSTANCE_INIT_POOL_TOKEN,
    HTTPs_ERR_INSTANCE_INIT_POOL_TOKEN_VAL,
    HTTPs_ERR_INSTANCE_INIT_POOL_CGI_DATA,
    HTTPs_ERR_INSTANCE_INIT_POOL_CGI_CTRL,
    HTTPs_ERR_INSTANCE_INIT_POOL_CGI_VAL,
    HTTPs_ERR_INSTANCE_INIT_POOL_CGI_BOUNDARY,
    HTTPs_ERR_INSTANCE_INIT_POOL_REQ_HDR,
    HTTPs_ERR_INSTANCE_INIT_POOL_REQ_HDR_STR,
    HTTPs_ERR_INSTANCE_INIT_POOL_RESP_HDR,
    HTTPs_ERR_INSTANCE_INIT_POOL_RESP_HDR_STR,


    HTTPs_ERR_INSTANCE_INIT_HEAP_REM_MEM,

    HTTPs_ERR_INSTANCE_LIST_NULL_ITEM,
    HTTPs_ERR_INSTANCE_ID_NOT_FOUND,

    HTTPs_ERR_SOCK_OPEN,
    HTTPs_ERR_SOCK_BIND,
    HTTPs_ERR_SOCK_LISTEN,
    HTTPs_ERR_SOCK_SET_OPT_BLOCK,
    HTTPs_ERR_SOCK_SET_OPT_SECURE,
    HTTPs_ERR_SOCK_TRANSITORY,
    HTTPs_ERR_SOCK_CONN_CLOSED,
    HTTPs_ERR_SOCK_FAULT,

    HTTPs_ERR_CFG_INVALID_OS_PRIO,
    HTTps_ERR_CFG_INVALID_OS_STACK_SIZE,
    HTTPs_ERR_CFG_INVALID_NBR_CONN,
    HTTPs_ERR_CFG_INVALID_NBR_SEL_EVENTS,
    HTTPs_ERR_CFG_INVALID_DFLT_FILE,
    HTTPs_ERR_CFG_INVALID_SOCK_SEL,
    HTTPs_ERR_CFG_INVALID_SECURE_EN,
    HTTPs_ERR_CFG_INVALID_SECURE_CERT_INVALID,
    HTTPs_ERR_CFG_INVALID_SECURE_KEY_INVALID,
    HTTPs_ERR_CFG_INVALID_HOST_LEN,
    HTTPs_ERR_CFG_INVALID_FS_PATH_LEN,
    HTTPs_ERR_CFG_INVALID_FS_SEP_CHAR,
    HTTPs_ERR_CFG_INVALID_HDR_EN,
    HTTPs_ERR_CFG_INVALID_HDR_PARAM,
    HTTPs_ERR_CFG_INVALID_TOKEN_EN,
    HTTPs_ERR_CFG_INVALID_TOKEN_PARAM,
    HTTPs_ERR_CFG_INVALID_CGI_EN,
    HTTPs_ERR_CFG_INVALID_CGI_PARAM,
    HTTPs_ERR_CFG_INVALID_BUF_LEN,
    HTTPs_ERR_CFG_INVALID_FILE_LEN,

    HTTPs_ERR_OS_INIT_REM_MEM,
    HTTPs_ERR_OS_OBJ_CREATE,
    HTTPs_ERR_OS_TASK_CREATE,
    HTTPs_ERR_OS_TASK_NAME_SET,
    HTTPs_ERR_OS_LOCK_CREATE,
    HTTPs_ERR_OS_LOCK_ACQUIRE,
    HTTPs_ERR_OS_STOP_REQ_CREATE,
    HTTPs_ERR_OS_STOP_REQ_SIGNAL,
    HTTPs_ERR_OS_STOP_COMPLETED_CREATE,
    HTTPs_ERR_OS_STOP_COMPLETED_PEND,
    HTTPs_ERR_OS_SEM_CREATE,
    HTTPs_OS_ERR_TMR_START,

    HTTPs_ERR_STATE_UNKNOWN,

    HTTPs_ERR_REQ_HDR_BLK_NONE_AVAIL,
    HTTPS_ERR_REQ_HDR_BLK_INVALID_VAL_LEN,
    HTTPS_ERR_REQ_HDR_BLK_NO_VAL_FIND,
    HTTPs_ERR_REQ_FORMAT_INV,
    HTTPs_ERR_REQ_METHOD_NOT_SUPPORTED,
    HTTPs_ERR_REQ_PROTOCOL_VER_NOT_SUPPORTED,
    HTTPs_ERR_REQ_MORE_DATA_REQUIRED,
    HTTPs_ERR_REQ_URI_LEN,
    HTTPs_ERR_REQ_URI_INV,

    HTTPs_ERR_REQ_HDR_OVERFLOW,
    HTTPs_ERR_REQ_HDR_POOL_EMPTY,
    HTTPs_ERR_REQ_HDR_POOL_LIB_ERR,
    HTTPs_ERR_REQ_HDR_DATA_TYPE_NOT_SUPPORTED,
    HTTPs_ERR_REQ_HDR_DATA_TYPE_UNKNOWN,

    HTTPs_ERR_METHOD_GET_STATE_UNKNOWN,
    HTTPs_ERR_METHOD_GET_CGI_NOT_EN,

    HTTPs_ERR_METHOD_POST_STATE_UNKNOWN,
    HTTPs_ERR_METHOD_POST_CGI_NOT_EN,
    HTTPs_ERR_METHOD_POST_MULTIPART_NOT_EN,
    HTTPs_ERR_METHOD_POST_CONTENT_TYPE_UNKNOWN,

    HTTPs_ERR_TOKEN_POOL_EMPTY,
    HTTPs_ERR_TOKEN_NO_TOKEN_FOUND,
    HTTPs_ERR_TOKEN_MORE_DATA_REQ,

    HTTPs_ERR_RESP_STATUS_LINE,

    HTTPs_ERR_EXTERN_MOD_INVALID,

    HTTPs_ERR_FILE_NOT_FOUND,
    HTTPs_ERR_FILE_SIZE_INV,
    HTTPs_ERR_FILE_404_NOT_FOUND,
    HTTPs_ERR_FILE_404_SIZE_INV,
    HTTPs_ERR_FILE_WR_FAULT,

    HTTPs_ERR_CGI_FILE_UPLOAD_OPEN,
    HTTPs_ERR_CGI_FILE_UPLOAD_NOT_EN,
    HTTPs_ERR_CGI_FORMAT_INV,
    HTTPs_ERR_CGI_CFG_POOL_SIZE_INV,


    HTTPs_ERR_RESP_HDR_OVERFLOW,
    HTTPs_ERR_RESP_HDR_POOL_EMPTY,
    HTTPs_ERR_RESP_HDR_POOL_LIB_ERR,
    HTTPs_ERR_RESP_HDR_DATA_TYPE_NOT_SUPPORTED,
    HTTPs_ERR_RESP_HDR_DATA_TYPE_UNKNOWN,

    HTTPs_ERR_RESP_HDR_BUF_POOL_EMPTY,
    HTTPs_ERR_RESP_HDR_BUF_POOL_LIB_ERR,

} HTTPs_ERR;


/*
*********************************************************************************************************
*                                         IP TYPE ENUMERATION
*********************************************************************************************************
*/

typedef enum https_sock_sel {
    HTTPs_SOCK_SEL_IPv4,
    HTTPs_SOCK_SEL_IPv6,
    HTTPs_SOCK_SEL_IPv4_IPv6,
} HTTPs_SOCK_SEL;


/*
*********************************************************************************************************
*                                    CONNECTION STATES ENUMERATION
*********************************************************************************************************
*/

typedef enum https_conn_state {
    HTTPs_CONN_STATE_UNKNOWN,

    HTTPs_CONN_STATE_REQ_INIT,
    HTTPs_CONN_STATE_REQ_PARSE_METHOD,
    HTTPs_CONN_STATE_REQ_PARSE_URI,
    HTTPs_CONN_STATE_REQ_PARSE_QUERY_STRING,
    HTTPs_CONN_STATE_REQ_PARSE_PROTOCOL_VERSION,
    HTTPs_CONN_STATE_REQ_PARSE_HDR,
    HTTPs_CONN_STATE_REQ_EXT_PROCESS,

    HTTPs_CONN_STATE_METHOD_INIT,
    HTTPs_CONN_STATE_METHOD_GET_CGI_POST,
    HTTPs_CONN_STATE_METHOD_GET_CGI_POLL,
    HTTPs_CONN_STATE_METHOD_GET_CGI_COMPLETED,
    HTTPs_CONN_STATE_METHOD_GET_CGI_ERR,
    HTTPs_CONN_STATE_METHOD_POST_PARSE,
    HTTPs_CONN_STATE_METHOD_POST_CGI_POST,
    HTTPs_CONN_STATE_METHOD_POST_CGI_POLL,
    HTTPs_CONN_STATE_METHOD_POST_CGI_COMPLETED,
    HTTPs_CONN_STATE_METHOD_POST_CGI_ERR,
    HTTPs_CONN_STATE_METHOD_POST_FILE,

    HTTPs_CONN_STATE_PREPARE_STATUS_CODE,
    HTTPs_CONN_STATE_PREPARE_FILE_OPEN,
    HTTPs_CONN_STATE_PREPARE_METHOD,

    HTTPs_CONN_STATE_RESP_INIT,
    HTTPs_CONN_STATE_RESP_TOKEN,
    HTTPs_CONN_STATE_RESP_STATUS_LINE,
    HTTPs_CONN_STATE_RESP_HDR,
    HTTPs_CONN_STATE_RESP_HDR_CONTENT_TYPE,
    HTTPs_CONN_STATE_RESP_HDR_FILE_TRANSFER,
    HTTPs_CONN_STATE_RESP_HDR_LOCATION,
    HTTPs_CONN_STATE_RESP_HDR_CONN,
    HTTPs_CONN_STATE_RESP_HDR_LIST,
    HTTPs_CONN_STATE_RESP_HDR_TX,
    HTTPs_CONN_STATE_RESP_FILE_STD,
    HTTPs_CONN_STATE_RESP_FILE_CHUNCKED,
    HTTPs_CONN_STATE_RESP_FILE_CHUNCKED_TX_TOKEN,
    HTTPs_CONN_STATE_RESP_FILE_CHUNCKED_TX_LAST_CHUNK,
    HTTPs_CONN_STATE_RESP_COMPLETED,

    HTTPs_CONN_STATE_COMPLETED,

    HTTPs_CONN_STATE_TIMEOUT,

    HTTPs_CONN_STATE_ERR_INTERNAL,
    HTTPs_CONN_STATE_ERR_FATAL
} HTTPs_CONN_STATE;


/*
*********************************************************************************************************
*                                      SOCKET STATES ENUMERATION
*********************************************************************************************************
*/

typedef enum https_sock_state {
    HTTPs_SOCK_STATE_NONE,
    HTTPs_SOCK_STATE_RX,
    HTTPs_SOCK_STATE_TX,
    HTTPs_SOCK_STATE_CLOSE,
    HTTPs_SOCK_STATE_ERR
} HTTPs_SOCK_STATE;


/*
*********************************************************************************************************
*                                         METHODS ENUMARATION
*********************************************************************************************************
*/

typedef enum https_method {
    HTTPs_METHOD_GET,
    HTTPs_METHOD_POST,
    HTTPs_METHOD_HEAD,
    HTTPs_METHOD_PUT,
    HTTPs_METHOD_DELETE,
    HTTPs_METHOD_TRACE,
    HTTPs_METHOD_CONNECT,
    HTTPs_METHOD_UNKNOWN
} HTTPs_METHOD;


/*
*********************************************************************************************************
*                                      STATUS CODES ENUMARATION
*********************************************************************************************************
*/

typedef enum https_status_code {
    HTTPs_STATUS_OK,
    HTTPs_STATUS_CREATED,
    HTTPs_STATUS_ACCEPTED,
    HTTPs_STATUS_NO_CONTENT,
    HTTPs_STATUS_RESET_CONTENT,
    HTTPs_STATUS_MOVED_PERMANENTLY,
    HTTPs_STATUS_FOUND,
    HTTPs_STATUS_SEE_OTHER,
    HTTPs_STATUS_NOT_MODIFIED,
    HTTPs_STATUS_USE_PROXY,
    HTTPs_STATUS_TEMPORARY_REDIRECT,
    HTTPs_STATUS_BAD_REQUEST,
    HTTPs_STATUS_UNAUTHORIZED,
    HTTPs_STATUS_FORBIDDEN,
    HTTPs_STATUS_NOT_FOUND,
    HTTPs_STATUS_METHOD_NOT_ALLOWED,
    HTTPs_STATUS_NOT_ACCEPTABLE,             /* With the Accept Req Hdr */
    HTTPs_STATUS_REQUEST_TIMEOUT,
    HTTPs_STATUS_CONFLIT,
    HTTPs_STATUS_GONE,
    HTTPs_STATUS_LENGTH_REQUIRED,
    HTTPs_STATUS_PRECONDITION_FAILED,
    HTTPs_STATUS_REQUEST_ENTITY_TOO_LARGE,
    HTTPs_STATUS_REQUEST_URI_TOO_LONG,
    HTTPs_STATUS_UNSUPPORTED_MEDIA_TYPE,
    HTTPs_STATUS_REQUESTED_RANGE_NOT_SATISFIABLE,
    HTTPs_STATUS_EXPECTATION_FAILED,
    HTTPs_STATUS_INTERNAL_SERVER_ERR,
    HTTPs_STATUS_NOT_IMPLEMENTED,
    HTTPs_STATUS_SERVICE_UNAVAILABLE,
    HTTPs_STATUS_HTTP_VERSION_NOT_SUPPORTED,
    HTTPs_STATUS_UNKOWN
} HTTPs_STATUS_CODE;


/*
*********************************************************************************************************
*                                     HTTPS HEADER ENUMARATION
*********************************************************************************************************
*/

typedef  enum https_hdr_field {

    HTTPs_HDR_FIELD_CONTENT_TYPE,
    HTTPs_HDR_FIELD_CONTENT_LEN,
    HTTPs_HDR_FIELD_HOST,
    HTTPs_HDR_FIELD_LOCATION,
    HTTPs_HDR_FIELD_CONN,
    HTTPs_HDR_FIELD_TRANSFER_ENCODING,
#if (HTTPs_CFG_HDR_EN == DEF_ENABLED)
    HTTPs_HDR_FIELD_ACCEPT,
    HTTPs_HDR_FIELD_ACCEPT_CHARSET,
    HTTPs_HDR_FIELD_ACCEPT_ENCODING,
    HTTPs_HDR_FIELD_ACCEPT_LANGUAGE,
    HTTPs_HDR_FIELD_ACCEPT_RANGES,
    HTTPs_HDR_FIELD_AGE,
    HTTPs_HDR_FIELD_ALLOW,
    HTTPs_HDR_FIELD_AUTHORIZATION,
    HTTPs_HDR_FIELD_CLIENT_IP,
    HTTPs_HDR_FIELD_CONTENT_BASE,
    HTTPs_HDR_FIELD_CONTENT_ENCODING,
    HTTPs_HDR_FIELD_CONTENT_LANGUAGE,
    HTTPs_HDR_FIELD_CONTENT_LOCATION,
    HTTPs_HDR_FIELD_CONTENT_MD5,
    HTTPs_HDR_FIELD_CONTENT_RANGE,
    HTTPs_HDR_FIELD_COOKIE,
    HTTPs_HDR_FIELD_COOKIE2,
    HTTPs_HDR_FIELD_DATE,
    HTTPs_HDR_FIELD_ETAG,
    HTTPs_HDR_FIELD_EXPECT,
    HTTPs_HDR_FIELD_EXPIRES,
    HTTPs_HDR_FIELD_FROM,
    HTTPs_HDR_FIELD_IF_MODIFIED_SINCE,
    HTTPs_HDR_FIELD_IF_MATCH,
    HTTPs_HDR_FIELD_IF_NONE_MATCH,
    HTTPs_HDR_FIELD_IF_RANGE,
    HTTPs_HDR_FIELD_IF_UNMODIFIED_SINCE,
    HTTPs_HDR_FIELD_LAST_MODIFIED,
    HTTPs_HDR_FIELD_PUBLIC,
    HTTPs_HDR_FIELD_RANGE,
    HTTPs_HDR_FIELD_REFERER,
    HTTPs_HDR_FIELD_RETRY_AFTER,
    HTTPs_HDR_FIELD_SERVER,
    HTTPs_HDR_FIELD_SET_COOKIE,
    HTTPs_HDR_FIELD_SET_COOKIE2,
    HTTPs_HDR_FIELD_TE,
    HTTPs_HDR_FIELD_TRAILER,
    HTTPs_HDR_FIELD_UPGRATE,
    HTTPs_HDR_FIELD_USER_AGENT,
    HTTPs_HDR_FIELD_VARY,
    HTTPs_HDR_FIELD_VIA,
    HTTPs_HDR_FIELD_WARNING,
    HTTPs_HDR_FIELD_WWW_AUTHENTICATE,
#endif
    HTTPs_HDR_FIELD_UNKNOWN
} HTTPs_HDR_FIELD;


/*
*********************************************************************************************************
*                                     HTTPS VERSIONS ENUMARATION
*********************************************************************************************************
*/

typedef  enum  https_protocol_ver {
    HTTPs_REQ_PROTOCOL_VER_0_9,
    HTTPs_REQ_PROTOCOL_VER_1_0,
    HTTPs_REQ_PROTOCOL_VER_1_1,
    HTTPs_REQ_PROTOCOL_VER_UNKNOWN
} HTTPs_PROTOCOL_VER;


/*
*********************************************************************************************************
*                                  REQUEST CONTENT TYPES ENUMARATION
*********************************************************************************************************
*/

typedef enum https_req_content_type {
    HTTPs_REQ_CONTENT_TYPE_APP,
    HTTPs_REQ_CONTENT_TYPE_MULTIPART,
    HTTPs_REQ_CONTENT_TYPE_BOUNDARY,
    HTTPs_REQ_CONTENT_TYPE_UNKNOWN
} HTTPs_REQ_CONTENT_TYPE;


/*
*********************************************************************************************************
*                             HEADER FIELD CONNECTION VALUES ENUMARATION
*********************************************************************************************************
*/

typedef enum https_hdr_field_conn_val {
    HTTPs_HDR_FIELD_CONN_CLOSE,
    HTTPs_HDR_FIELD_CONN_UNKNOWN
} HTTPs_HDR_FIELD_CONN_VAL;


/*
*********************************************************************************************************
*                                      CONTENT TYPES ENUMARATION
*********************************************************************************************************
*/

typedef enum https_content_type {
    HTTPs_CONTENT_TYPE_HTML,
    HTTPs_CONTENT_TYPE_OCTET_STREAM,
    HTTPs_CONTENT_TYPE_PDF,
    HTTPs_CONTENT_TYPE_ZIP,
    HTTPs_CONTENT_TYPE_GIF,
    HTTPs_CONTENT_TYPE_JPEG,
    HTTPs_CONTENT_TYPE_PNG,
    HTTPs_CONTENT_TYPE_JS,
    HTTPs_CONTENT_TYPE_PLAIN,
    HTTPs_CONTENT_TYPE_CSS,
    HTTPs_CONTENT_TYPE_UNKOWN
} HTTPs_CONTENT_TYPE;


/*
*********************************************************************************************************
*                                       FILE TYPES ENUMARATION
*********************************************************************************************************
*/

typedef  enum https_file_type {
    HTTPs_FILE_TYPE_FS,
    HTTPs_FILE_TYPE_STATIC_DATA,
    HTTPs_FILE_TYPE_NO_BODY
} HTTPs_FILE_TYPE;


/*
*********************************************************************************************************
*                                     TOKEN TYPES ENUMARATION
*********************************************************************************************************
*/

typedef enum  HTTPs_token_type {
    HTTPs_TOKEN_TYPE_EXTERNAL,
    HTTPs_TOKEN_TYPE_INTERNAL,
    HTTPs_TOKEN_TYPE_NONE
} HTTPs_TOKEN_TYPE;


/*
*********************************************************************************************************
*                                     CGI DATA TYPES ENUMARATION
*********************************************************************************************************
*/

typedef enum  HTTPs_CGI_DataType {
    HTTPs_CGI_DATA_TYPE_CTRL_VAL,
    HTTPs_CGI_DATA_TYPE_FILE
} HTTPs_CGI_DATA_TYPE;


/*
*********************************************************************************************************
*********************************************************************************************************
*                                              DATA TYPES
*********************************************************************************************************
*********************************************************************************************************
*/

typedef  CPU_INT08U       HTTPs_OS_TASK_PRIO;
typedef  CPU_INT32U       HTTPs_OS_TASK_STACK_SIZE;


/*
*********************************************************************************************************
*                                           SECURE DATA TYPE
*********************************************************************************************************
*/

typedef  struct  HTTPs_SecureCfg {
    CPU_CHAR                      *CertPtr;
    CPU_INT32U                     CertLen;
    CPU_CHAR                      *KeyPtr;
    CPU_INT32U                     KeyLen;
    NET_SOCK_SECURE_CERT_KEY_FMT   Fmt;
    CPU_BOOLEAN                    CertChain;
} HTTPs_SECURE_CFG;


/*
*********************************************************************************************************
*                                           TOKEN DATA TYPE
*********************************************************************************************************
*/


typedef  struct  HTTPs_ConnTokenCtrl {
    CPU_CHAR    *ValPtr;
    CPU_INT16U   ValLen;

    CPU_CHAR    *TxPtr;
    CPU_INT16U   TxLen;
} HTTPs_CONN_TOKEN_CTRL;


/*
*********************************************************************************************************
*                                            CGI DATA TYPE
*********************************************************************************************************
*/

typedef  struct  HTTPs_CGI_Data  HTTPs_CGI_DATA;

struct  HTTPs_CGI_Data {
    HTTPs_CGI_DATA_TYPE   DataType;
    CPU_CHAR             *CtrlNamePtr;
    CPU_INT16U            CtrlNameLen;
    CPU_CHAR             *ValPtr;
    CPU_INT16U            ValLen;
    HTTPs_CGI_DATA       *DataNextPtr;
};


/*
*********************************************************************************************************
*                                       HTTP RESP/REQ HDR FIELD BLK
*********************************************************************************************************
*/

typedef  enum  HTTPs_hdr_val_type {
    HTTPs_HDR_VAL_TYPE_NONE,
    HTTPs_HDR_VAL_TYPE_BOOL,
    HTTPs_HDR_VAL_TYPE_INT,
    HTTPs_HDR_VAL_TYPE_CLK,
    HTTPs_HDR_VAL_TYPE_STR_CONST,
    HTTPs_HDR_VAL_TYPE_STR_DYN
} HTTPs_HDR_VAL_TYPE;


typedef  struct  HTTPs_hdr_blk  HTTPs_HDR_BLK;

struct  HTTPs_hdr_blk {
    HTTPs_HDR_FIELD      HdrField;
    HTTPs_HDR_VAL_TYPE   ValType;
    void                *ValPtr;
    CPU_INT32U           ValLen;
    HTTPs_HDR_BLK       *HdrBlkNextPtr;
    HTTPs_HDR_BLK       *HdrBlkPrevPtr;
};


/*
*********************************************************************************************************
*                                        CONNECTIONS DATA TYPE
*********************************************************************************************************
*/

typedef  struct  HTTPs_Conn  HTTPs_CONN;

struct  HTTPs_Conn {
    void                     *OS_InstanceLockPtr;               /* Ptr to instance os lock obj.                         */
    void                     *OS_ConnTmr;                       /* Ptr to conn os tmr obj.                              */

    NET_SOCK_ID               SockID;                           /* Connection's Socket ID.                              */
    HTTPs_SOCK_STATE          SockState;                        /* Connection's Socket State.                           */
    CPU_BOOLEAN               SockRdyRd;                        /* Connection's Socket Read Ready flag.                 */
    CPU_BOOLEAN               SockRdyWr;                        /* Connection's Socket Write Ready flag.                */
    CPU_BOOLEAN               SockRdyErr;                       /* Connection's Socket Error Ready flag.                */

    NET_SOCK_ADDR             ClientAddr;                       /* Client socket information.                           */

    NET_TMR                  *ConnTmrPtr;                       /* Pointer to Connection's Timer.                       */

    HTTPs_CONN_STATE          State;                            /* Connection State.                                    */

    HTTPs_METHOD              Method;                           /* HTTP method received in request message.             */
    HTTPs_REQ_CONTENT_TYPE    ReqContentType;                   /* Content-Type received in request message.            */
    CPU_INT32U                ReqContentLen;                    /* Content-Length received in request message.          */
    CPU_INT32U                ReqContentLenRxd;

    CPU_BOOLEAN               RespLocation;                     /* Flag indicating Location hdr requirement in resp.    */
    HTTPs_HDR_FIELD_CONN_VAL  RespConn;                         /* Connection State value to put in response message.   */

    HTTPs_STATUS_CODE         StatusCode;                       /* Status code of the resp after parsing of the req.    */
    HTTPs_PROTOCOL_VER        ProtocolVer;                      /* HTTP version received in request message.            */

    HTTPs_ERR                 ErrCode;                          /* Error code when internal error occurs.               */

    CPU_CHAR                 *FileNamePtr;                      /* Pointer to file name requested by client.            */
    CPU_SIZE_T                FileNameLenMax;                   /* Maximum file name length.                            */
    void                     *FilePtr;                          /* Pointer to file in FS.                               */
    CPU_INT32U                FileLen;                          /* File length.                                         */
    CPU_INT32U                FileTxdLen;                       /* The current Length of data sent.                     */
    HTTPs_CONTENT_TYPE        FileContentType;                  /* Content-Type of file to send                         */
    HTTPs_FILE_TYPE           FileType;                         /* Type of file. (file in FS or fixed file)             */
    CPU_INT32U                FileFixPosCur;                    /* Current position in the fixed file.                  */

#if (HTTPs_CFG_HDR_EN == DEF_ENABLED)
    HTTPs_HDR_BLK            *ReqHdrFirstPtr;                   /* Ptr to first additional request hdr.                 */
    HTTPs_HDR_BLK            *ReqHdrLastPtr;                    /* Ptr to last additional request hdr.                  */
    CPU_INT08U                ReqHdrCtr;                        /* Nbr of additional request hdrs.                      */

    HTTPs_HDR_BLK            *RespHdrFirstPtr;                  /* Ptr to first additional response hdr.                */
    HTTPs_HDR_BLK            *RespHdrLastPtr;                   /* Ptr to last additional response hdr.                 */
    CPU_INT08U                RespHdrCtr;                       /* Nbr of additional response hdrs.                     */
#endif

#if (HTTPs_CFG_ABSOLUTE_URI_EN == DEF_ENABLED)
    CPU_CHAR                 *HostPtr;                          /* Ptr to host name received in the request.            */
#endif

#if (HTTPs_CFG_TOKEN_PARSE_EN == DEF_ENABLED)
    HTTPs_CONN_TOKEN_CTRL    *TokenCtrlPtr;                     /* Ptr to the ctrl token structure.                     */
    CPU_CHAR                 *TokenPtr;                         /* Ptr to the token name.                               */
    CPU_INT16U                TokenLen;                         /* Length of the token name.                            */
    CPU_INT16U                TokenBufRemLen;                   /* Remaining data len in the buf.                       */
#endif

#if (HTTPs_CFG_CGI_EN == DEF_ENABLED)
    HTTPs_CGI_DATA           *CGI_DataFirstPtr;                 /* Ptr to first CGI key pair-value rxd.                 */
    HTTPs_CGI_DATA           *CGI_DataLastPtr;                  /* Ptr to last  CGI key pair-value rxd.                 */
    CPU_INT16U                CGI_DataAcquiredCtr;
#if (HTTPs_CFG_CGI_MULTIPART_EN == DEF_ENABLED)
    CPU_CHAR                 *CGI_BoundaryPtr;                  /* Ptr to the boundary for CGI parsing.                 */
    CPU_INT08U                CGI_BoundaryLen;
#endif
#endif

    CPU_CHAR                 *BufPtr;                           /* Ptr to conn buf.                                     */
    CPU_INT16U                BufLen;                           /* Conn buf len.                                        */

    CPU_CHAR                 *RxBufPtr;                         /* Ptr where to receive buf.                            */
    CPU_INT16U                RxBufLenRem;                      /* Rem data in the rx buffer.                           */
    CPU_INT32U                RxDataLen;                        /* Len of data rxd.                                     */

    CPU_CHAR                 *TxBufPtr;                         /* Ptr to buf to tx.                                    */
    CPU_INT16U                TxDataLen;                        /* Data len to tx.                                      */

    void                     *ConnDataPtr;                      /* Ptr for user conn data.                              */

    HTTPs_CONN               *ConnPrevPtr;                      /* Pointer to previous connection.                      */
    HTTPs_CONN               *ConnNextPtr;                      /* Pointer to next connection.                          */
};


/*
*********************************************************************************************************
*                                       CONFIGURATION DATA TYPE
*********************************************************************************************************
*/

typedef  struct  HTTPs_Instance     HTTPs_INSTANCE;

typedef  struct  HTTPs_Cfg {

/*
*--------------------------------------------------------------------------------------------------------
*                                      INSTANCE OS CONFIGURATION
*--------------------------------------------------------------------------------------------------------
*/

    HTTPs_OS_TASK_PRIO         OS_TaskPrio;
    HTTPs_OS_TASK_STACK_SIZE   OS_TaskStackSize;
    CPU_INT32U                 OS_TaskDly_ms;


/*
*--------------------------------------------------------------------------------------------------------
*                                INSTANCE LISTEN SOCKET CONFIGURATION
*--------------------------------------------------------------------------------------------------------
*/

    HTTPs_SOCK_SEL             SockSel;
    HTTPs_SECURE_CFG          *SecurePtr;
    CPU_INT16U                 Port;

/*
*--------------------------------------------------------------------------------------------------------
*                                  INSTANCE CONNECTION CONFIGURATION
*--------------------------------------------------------------------------------------------------------
*/

    CPU_INT08U                 ConnNbrMax;
    CPU_INT16U                 ConnInactivityTimeout_s;
    CPU_INT16U                 BufLen;

    void                     (*InstanceConnObjsInit)  (const  HTTPs_INSTANCE  *p_instance);


/*
*--------------------------------------------------------------------------------------------------------
*                                  INSTANCE FILE SYSTEM CONFIGURATION
*--------------------------------------------------------------------------------------------------------
*/

    CPU_CHAR                  *WorkingFolderPtr;
    CPU_CHAR                  *FileStr_DfltFilePtr;


/*
*--------------------------------------------------------------------------------------------------------
*                               CONNECTION CALLBACK HOOK CONFIGURATION
*--------------------------------------------------------------------------------------------------------
*/

    CPU_BOOLEAN              (*ConnReqFnctPtr)  (const  HTTPs_INSTANCE     *p_instance,
                                                        HTTPs_CONN         *p_conn);

    void                     (*ConnErrFnctPtr)  (const  HTTPs_INSTANCE     *p_instance,
                                                        HTTPs_CONN         *p_conn,
                                                        HTTPs_ERR           err);

    void                     (*ConnCloseFnctPtr)(const  HTTPs_INSTANCE     *p_instance,
                                                        HTTPs_CONN         *p_conn);

    void                     (*ErrFileGetFnctPtr)(      HTTPs_STATUS_CODE    status_code,
                                                        CPU_CHAR            *p_file_str,
                                                        CPU_INT32U           file_len_max,
                                                        HTTPs_FILE_TYPE     *p_file_type,
                                                        void               **p_data,
                                                        CPU_INT32U          *p_date_len);


/*
*--------------------------------------------------------------------------------------------------------
*                                     HEADER FIELD CONFIGURATION
*--------------------------------------------------------------------------------------------------------
*/

    CPU_BOOLEAN                HdrFieldEn;

    CPU_INT16U                 ReqHdrPoolSize;
    CPU_INT16U                 ReqHdrConnNbrMax;
    CPU_INT16U                 ReqHdrDataLen;

    CPU_BOOLEAN              (*ReqHdrRxFnctPtr) (const  HTTPs_INSTANCE   *p_instance,
                                                 const  HTTPs_CONN       *p_conn,
                                                        HTTPs_HDR_FIELD   hdr_field);


    CPU_INT16U                 RespHdrPoolSize;
    CPU_INT16U                 RespHdrConnNbrMax;

    CPU_INT16U                 RespHdrBoolPoolSize;
    CPU_INT16U                 RespHdrIntPoolSize;
    CPU_INT16U                 RespHdrClkPoolSize;
    CPU_INT16U                 RespHdrStrPoolSize;
    CPU_INT16U                 RespHdrStrLenMax;

    CPU_BOOLEAN              (*RespHdrTxFnctPtr)(HTTPs_INSTANCE  *p_instance,
                                                 HTTPs_CONN      *p_conn);


/*
*--------------------------------------------------------------------------------------------------------
*                                    INSTANCE CGI FORM CONFIGURATION
*--------------------------------------------------------------------------------------------------------
*/

    CPU_BOOLEAN                CGI_En;
    CPU_INT16U                 CGI_PoolSize;
    CPU_INT16U                 CGI_CtrlNameLenMax;
    CPU_INT16U                 CGI_ValLenMax;

    CPU_BOOLEAN                CGI_MultipartEn;
    CPU_BOOLEAN                CGI_MultipartFileUploadEn;
    CPU_BOOLEAN                CGI_MultipartFileUploadOverwriteEn;
    CPU_CHAR                  *CGI_MultipartFileUploadFolderPtr;

    CPU_BOOLEAN              (*CGI_PostFnctPtr)(const  HTTPs_INSTANCE  *p_instance,
                                                       HTTPs_CONN      *p_conn,
                                                const  HTTPs_CGI_DATA  *p_data);

    CPU_BOOLEAN              (*CGI_PollFnctPtr)(const  HTTPs_INSTANCE  *p_instance,
                                                       HTTPs_CONN      *p_conn);


/*
*--------------------------------------------------------------------------------------------------------
*                               DYNAMIC TOKEN REPLACEMENT CONFIGURATION
*--------------------------------------------------------------------------------------------------------
*/

    CPU_BOOLEAN                TokenParseEn;
    CPU_INT16U                 TokenPoolSize;
    CPU_INT16U                 TokenValLenMax;
    CPU_BOOLEAN              (*TokenValGetFnctPtr)(const  HTTPs_INSTANCE  *p_instance,
                                                   const  HTTPs_CONN      *p_conn,
                                                   const  CPU_CHAR        *p_token,
                                                          CPU_INT16U       token_len,
                                                          CPU_CHAR        *p_val,
                                                          CPU_INT16U       val_len_max);


/*
*--------------------------------------------------------------------------------------------------------
*                                    INSTANCE PROXY CONFIGURATION
*--------------------------------------------------------------------------------------------------------
*/

    CPU_INT16U                 HostNameLenMax;

} HTTPs_CFG;                                                    /* End of configuration structure.                      */


/*
*********************************************************************************************************
*                                INSTANCE STATISTIC COUNTERS DATA TYPE
*********************************************************************************************************
*/

typedef  struct  HTTPs_InstanceStats {
    CPU_INT32U  Conn_StatAcceptedCtr;
    CPU_INT32U  Conn_StatClosedCtr;
    CPU_INT32U  Conn_StatAcquiredCtr;
    CPU_INT32U  Conn_StatReleasedCtr;

#if (HTTPs_CFG_HDR_EN)
    CPU_INT32U  ReqHdr_StatAcquiredCtr;
    CPU_INT32U  ReqHdr_StatReleaseCtr;
    CPU_INT32U  RespHdr_StatAcquiredCtr;
    CPU_INT32U  RespHdr_StatReleaseCtr;
#endif

#if (HTTPs_CFG_TOKEN_PARSE_EN == DEF_ENABLED)
    CPU_INT32U  Token_StatAcquiredCtr;
    CPU_INT32U  Token_StatReleaseCtr;
    CPU_INT32U  Token_StatFileParsedCtr;

    CPU_INT32U  File_StatTokenReplacedCtr;
#endif

#if (HTTPs_CFG_CGI_EN == DEF_ENABLED)
    CPU_INT32U  CGI_StatDataAcquiredCtr;
    CPU_INT32U  CGI_StatDataReleaseCtr;
    CPU_INT32U  CGI_StatDataProcessedCtr;

#if ((HTTPs_CFG_CGI_MULTIPART_EN   == DEF_ENABLED) && \
     (HTTPs_CFG_CGI_FILE_UPLOAD_EN == DEF_ENABLED))
    CPU_INT32U  CGI_StatFileUploadOpenedCtr;
    CPU_INT32U  CGI_StatFileUploadClosedCtr;
#endif
#endif

    CPU_INT32U  CGI_StatContentTypeAppRxdCtr;
    CPU_INT32U  CGI_StatContentTypeMultipartRxdCtr;
    CPU_INT32U  CGI_StatContentTypeUnknownRxdCtr;

    CPU_INT32U  Sock_StatListenCloseCtr;
    CPU_INT32U  Sock_StatOctetRxdCtr;
    CPU_INT32U  Sock_StatOctetTxdCtr;


    CPU_INT32U  Req_StatRxdCtr;
    CPU_INT32U  Req_StatProcessedCtr;
    CPU_INT32U  Req_StatMethodGetRxdCtr;
    CPU_INT32U  Req_StatMethodHeadRxdCtr;
    CPU_INT32U  Req_StatMethodPostRxdCtr;
    CPU_INT32U  Req_StatMethodUnsupportedRxdCtr;
    CPU_INT32U  Req_StatProtocolVer0_9Ctr;
    CPU_INT32U  Req_StatProtocolVer1_0Ctr;
    CPU_INT32U  Req_StatProtocolVer1_1Ctr;
    CPU_INT32U  Req_StatProtocolVerUnsupportedCtr;

    CPU_INT32U  Method_StatGetProcessedCtr;
    CPU_INT32U  Method_StatHeadProcessedCtr;
    CPU_INT32U  Method_StatPostProcessedCtr;
    CPU_INT32U  Method_StatUnsupportedProcessedCtr;

    CPU_INT32U  Resp_StatTxdCtr;

    CPU_INT32U  FS_StatTxdCtr;
    CPU_INT32U  FS_StatOpenedCtr;
    CPU_INT32U  FS_StatClosedCtr;

    CPU_INT32U  Prepare_StatOKCtr;
    CPU_INT32U  Prepare_StatMovedPermanentlyCtr;
    CPU_INT32U  Prepare_StatFoundCtr;
    CPU_INT32U  Prepare_StatSeeOtherCtr;
    CPU_INT32U  Prepare_StatNotModifiedCtr;
    CPU_INT32U  Prepare_StatUseProxyCtr;
    CPU_INT32U  Prepare_StatTemporaryredirectCtr;
    CPU_INT32U  Prepare_StatCreatedCtr;
    CPU_INT32U  Prepare_StatAcceptedCtr;
    CPU_INT32U  Prepare_StatNoContentCtr;
    CPU_INT32U  Prepare_StatResetContentCtr;
    CPU_INT32U  Prepare_StatBadRequestCtr;
    CPU_INT32U  Prepare_StatUnauthorizedCtr;
    CPU_INT32U  Prepare_StatForbiddenCtr;
    CPU_INT32U  Prepare_StatNotFoundCtr;
    CPU_INT32U  Prepare_StatMehtodNotAllowedCtr;
    CPU_INT32U  Prepare_StatNotAcceptableCtr;
    CPU_INT32U  Prepare_StatRequestTimeoutCtr;
    CPU_INT32U  Prepare_StatConflitCtr;
    CPU_INT32U  Prepare_StatGoneCtr;
    CPU_INT32U  Prepare_StatLengthRequiredCtr;
    CPU_INT32U  Prepare_StatConditionFailedCtr;
    CPU_INT32U  Prepare_StatEntityTooLongCtr;
    CPU_INT32U  Prepare_StatURI_TooLongCtr;
    CPU_INT32U  Prepare_StatUnsupportedTypeCtr;
    CPU_INT32U  Prepare_StatNotSatisfiableCtr;
    CPU_INT32U  Prepare_StatExpectationFailedCtr;
    CPU_INT32U  Prepare_StatInternalServerErrCtr;
    CPU_INT32U  Prepare_StatNotImplementedCtr;
    CPU_INT32U  Prepare_StatSerUnavailableCtr;
    CPU_INT32U  Prepare_StatVerNotSupportedCtr;
    CPU_INT32U  Prepare_StatStatusCodeUnknownCtr;
} HTTPs_INSTANCE_STATS;


/*
*********************************************************************************************************
*                                  INSTANCE ERROR COUNTERS DATA TYPE
*********************************************************************************************************
*/

typedef  struct  HTTPs_InstanceErrs {
    CPU_INT32U  Conn_ErrFreePtrNullCtr;

    CPU_INT32U  Conn_ErrPoolEmptyCtr;
    CPU_INT32U  Conn_ErrPoolLibGetCtr;
    CPU_INT32U  Conn_ErrPoolLibFreeCtr;

    CPU_INT32U  Conn_ErrBufPoolEmptyCtr;
    CPU_INT32U  Conn_ErrBufPoolLibGetCtr;
    CPU_INT32U  Conn_ErrBufPoolLibFreeCtr;

    CPU_INT32U  Conn_ErrFileNamePoolEmptyCtr;
    CPU_INT32U  Conn_ErrFileNamePoolLibGetCtr;
    CPU_INT32U  Conn_ErrFileNamePoolLibFreeCtr;

    CPU_INT32U  Conn_ErrNoneAvailCtr;
    CPU_INT32U  Conn_ErrTmrStartCtr;
    CPU_INT32U  Conn_ErrTimeoutCtr;

    CPU_INT32U  Req_ErrInvCtr;
    CPU_INT32U  Req_ErrStateInvCtr;

    CPU_INT32U  Method_ErrStateUnknownCtr;
    CPU_INT32U  Method_ErrPostCGICtr;

#if (HTTPs_CFG_ABSOLUTE_URI_EN == DEF_ENABLED)
    CPU_INT32U  Host_ErrPoolEmptyCtr;
    CPU_INT32U  Host_ErrPoolLibGetCtr;
    CPU_INT32U  Host_ErrPoolLibFreeCtr;
#endif

#if (HTTPs_CFG_TOKEN_PARSE_EN == DEF_ENABLED)
    CPU_INT32U  Token_ErrPtrNullCtr;

    CPU_INT32U  Token_ErrPoolEmptyCtr;
    CPU_INT32U  Token_ErrPoolLibGetCtr;
    CPU_INT32U  Token_ErrPoolLibFreeCtr;

    CPU_INT32U  Token_ErrValPoolEmptyCtr;
    CPU_INT32U  Token_ErrValPoolLibGetCtr;
    CPU_INT32U  Token_ErrValPoolLibFreeCtr;

    CPU_INT32U  Token_ErrCloseNotEmptyCtr;

    CPU_INT32U  Token_ErrInternalInv;
    CPU_INT32U  Token_ErrInternalStatusCodeInv;
    CPU_INT32U  Token_ErrInternalReasonPhraseIntInv;

    CPU_INT32U  Token_ErrTypeInvCtr;
#endif

#if (HTTPs_CFG_CGI_EN == DEF_ENABLED)
    CPU_INT32U  CGI_ErrPoolEmptyCtr;
    CPU_INT32U  CGI_ErrPoolLibGetCtr;
    CPU_INT32U  CGI_ErrPoolLibFreeCtr;

    CPU_INT32U  CGI_ErrCtrlPoolEmptyCtr;
    CPU_INT32U  CGI_ErrCtrlPoolLibGetCtr;
    CPU_INT32U  CGI_ErrCtrlPoolLibFreeCtr;

    CPU_INT32U  CGI_ErrValPoolEmptyCtr;
    CPU_INT32U  CGI_ErrValPoolLibGetCtr;
    CPU_INT32U  CGI_ErrValPoolLibFreeCtr;

    CPU_INT32U  CGI_ErrCloseNotEmptyCtr;

#if (HTTPs_CFG_CGI_MULTIPART_EN == DEF_ENABLED)
    CPU_INT32U  CGI_ErrBoundaryPoolEmptyCtr;
    CPU_INT32U  CGI_ErrBoundaryPoolLibGetCtr;
    CPU_INT32U  CGI_ErrBoundaryPoolLibFreeCtr;
#endif
#endif


    CPU_INT32U  Prepare_ErrFS_NullPtr;
    CPU_INT32U  Prepare_ErrFS_TypeInv;
    CPU_INT32U  Prepare_ErrFS_DoesntExist;


#if (HTTPs_CFG_HDR_EN)
    CPU_INT32U  ReqHdr_ErrPtrNullCtr;

    CPU_INT32U  ReqHdr_ErrPoolEmptyCtr;
    CPU_INT32U  ReqHdr_ErrPoolLibGetCtr;
    CPU_INT32U  ReqHdr_ErrPoolLibFreeCtr;

    CPU_INT32U  ReqHdr_ErrBufPoolEmptyCtr;
    CPU_INT32U  ReqHdr_ErrBufPoolLibGetCtr;
    CPU_INT32U  ReqHdr_ErrBufPoolLibFreeCtr;

    CPU_INT32U  ReqHdr_ErrValTypeNotsupported;
    CPU_INT32U  ReqHdr_ErrValTypeUnknown;

    CPU_INT32U  ReqHdr_ErrDataLenInv;



    CPU_INT32U  RespHdr_ErrPtrNullCtr;

    CPU_INT32U  RespHdr_ErrPoolEmptyCtr;
    CPU_INT32U  RespHdr_ErrPoolLibGetCtr;
    CPU_INT32U  RespHdr_ErrPoolLibFreeCtr;

    CPU_INT32U  RespHdr_ErrBufPoolEmptyCtr;
    CPU_INT32U  RespHdr_ErrBufPoolLibGetCtr;
    CPU_INT32U  RespHdr_ErrBufPoolLibFreeCtr;

    CPU_INT32U  RespHdr_ErrValTypeNotsupported;
    CPU_INT32U  RespHdr_ErrValTypeUnknown;

    CPU_INT32U  RespHdr_ErrCloseNotEmptyCtr;
#endif

    CPU_INT32U  Sock_ErrListenCloseCtr;
    CPU_INT32U  Sock_ErrAcceptCtr;
    CPU_INT32U  Sock_ErrSelCtr;
    CPU_INT32U  Sock_ErrCloseCtr;
    CPU_INT32U  Sock_ErrRxCtr;
    CPU_INT32U  Sock_ErrRxConnClosedCtr;
    CPU_INT32U  Sock_ErrRxFaultCtr;

    CPU_INT32U  Sock_ErrTxConnClosedCtr;
    CPU_INT32U  Sock_ErrTxFaultCtr;


    CPU_INT32U  Resp_ErrChunckStateInvCtr;

    CPU_INT32U  Resp_ErrPrepareStatusLineCtr;
    CPU_INT32U  Resp_ErrPrepareHdrCtr;
    CPU_INT32U  Resp_ErrStateUnknownCtr;

    CPU_INT32U  ErrInternal_ErrReqMethodNotSupported;
    CPU_INT32U  ErrInternal_ErrReqFormatInv;
    CPU_INT32U  ErrInternal_ErrReqURI_Inv;
    CPU_INT32U  ErrInternal_ErrReqURI_Len;
    CPU_INT32U  ErrInternal_ErrReqProtocolNotSupported;
    CPU_INT32U  ErrInternal_ErrReqMoreDataRequired;
    CPU_INT32U  ErrInternal_ErrReqHdrOverflow;

    CPU_INT32U  ErrInternal_ErrMethodPostCGINotEn;
    CPU_INT32U  ErrInternal_ErrMethodPostMultipartNotEn;
    CPU_INT32U  ErrInternal_ErrMethodPostContentTypeUnknown;
    CPU_INT32U  ErrInternal_ErrMethodPostStateUnknown;

    CPU_INT32U  ErrInternal_ErrCGI_FormatInv;
    CPU_INT32U  ErrInternal_ErrCGI_PoolSizeInv;
    CPU_INT32U  ErrInternal_ErrCGI_FileUploadOpen;

    CPU_INT32U  ErrInternal_StateUnknown;
    CPU_INT32U  ErrInternal_Unknown;

} HTTPs_INSTANCE_ERRS;


/*
*********************************************************************************************************
*                                         INSTANCE DATA TYPE
*********************************************************************************************************
*/

typedef  CPU_INT08U                 HTTPs_INSTANCE_ID;
#define  HTTPs_INSTANCE_ID_NONE     0


struct  HTTPs_Instance {
           HTTPs_INSTANCE_ID      ID;
           void                  *OS_TaskObjPtr;
           void                  *OS_LockObjPtr;
           CPU_BOOLEAN            Started;

    const  HTTPs_CFG             *CfgPtr;

#ifdef   NET_IPv4_MODULE_EN
           NET_SOCK_ID            SockListenID_IPv4;
#endif

#ifdef   NET_IPv6_MODULE_EN
           NET_SOCK_ID            SockListenID_IPv6;
#endif


    const  NET_FS_API            *FS_API_Ptr;
           CPU_INT32U             FS_PathLenMax;
           CPU_CHAR               FS_PathSepChar;

           MEM_POOL               PoolConn;
           MEM_POOL               PoolBuf;
           MEM_POOL               PoolFileName;

#if (HTTPs_CFG_ABSOLUTE_URI_EN == DEF_ENABLED)
           MEM_POOL               PoolHost;
#endif

#if (HTTPs_CFG_TOKEN_PARSE_EN == DEF_ENABLED)
           MEM_POOL               PoolTokenCtrl;
           MEM_POOL               PoolTokenVal;
#endif


#if (HTTPs_CFG_CGI_EN == DEF_ENABLED)
           MEM_POOL               PoolCGI_Data;
           MEM_POOL               PoolCGI_Ctrl;
           MEM_POOL               PoolCGI_Val;

#if (HTTPs_CFG_CGI_MULTIPART_EN == DEF_ENABLED)
           MEM_POOL               PoolCGI_Boundary;
#endif
#endif

#if (HTTPs_CFG_HDR_EN == DEF_ENABLED)
           MEM_POOL               PoolReqHdr;
           MEM_POOL               PoolReqHdrStr;

           MEM_POOL               PoolRespHdr;
           MEM_POOL               PoolRespHdrStr;

#if 0                                                           /* #### NET-444                                         */
           MEM_POOL               PoolRespHdrInt;
           MEM_POOL               PoolRespHdrClk;
#endif

#endif

           NET_TMR_TICK           ConnTimeoutTick;

           HTTPs_CONN            *ConnFirstPtr;
           HTTPs_CONN            *ConnLastPtr;

           CPU_INT08U             ConnActiveCtr;

#if (HTTPs_CFG_DBG_INFO_EN == DEF_ENABLED)
           HTTPs_INSTANCE        *InstancePrevPtr;
           HTTPs_INSTANCE        *InstanceNextPtr;
#endif

#if (HTTPs_CFG_CTR_STAT_EN == DEF_ENABLED)
           HTTPs_INSTANCE_STATS   StatsCtr;
#endif

#if (HTTPs_CFG_CTR_ERR_EN  == DEF_ENABLED)
           HTTPs_INSTANCE_ERRS    ErrsCtr;
#endif
};


/*
*********************************************************************************************************
*                                        DICTIONARY DATA TYPE
*********************************************************************************************************
*/

typedef  CPU_INT32U  HTTPs_DICTIONARY_KEY;

#define  HTTPs_DICTIONARY_KEY_INVALID                       DEF_INT_32U_MAX_VAL


typedef  struct  HTTPs_dictionary {
    const  HTTPs_DICTIONARY_KEY   Key;
    const  CPU_CHAR              *StrPtr;
    const  CPU_INT32U             StrLen;
} HTTPs_DICTIONARY;


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           GLOBAL VARIABLES
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             HTTP METHOD
*********************************************************************************************************
*/

#define  HTTPs_STR_METHOD_GET                               "GET"
#define  HTTPs_STR_METHOD_POST                              "POST"
#define  HTTPs_STR_METHOD_HEAD                              "HEAD"
#define  HTTPs_STR_METHOD_PUT                               "PUT"
#define  HTTPs_STR_METHOD_DELETE                            "DELETE"
#define  HTTPs_STR_METHOD_TRACE                             "TRACE"
#define  HTTPs_STR_METHOD_CONNECT                           "CONNECT"

static  const  HTTPs_DICTIONARY  HTTPs_DictionaryReqMethods[] = {      /* See Note #1.                                         */
    { HTTPs_METHOD_GET,     HTTPs_STR_METHOD_GET,     (sizeof(HTTPs_STR_METHOD_GET)     - 1) },
    { HTTPs_METHOD_POST,    HTTPs_STR_METHOD_POST,    (sizeof(HTTPs_STR_METHOD_POST)    - 1) },
    { HTTPs_METHOD_HEAD,    HTTPs_STR_METHOD_HEAD,    (sizeof(HTTPs_STR_METHOD_HEAD)    - 1) },
    { HTTPs_METHOD_PUT,     HTTPs_STR_METHOD_PUT,     (sizeof(HTTPs_STR_METHOD_PUT)     - 1) },
    { HTTPs_METHOD_DELETE,  HTTPs_STR_METHOD_DELETE,  (sizeof(HTTPs_STR_METHOD_DELETE)  - 1) },
    { HTTPs_METHOD_TRACE,   HTTPs_STR_METHOD_TRACE,   (sizeof(HTTPs_STR_METHOD_TRACE)   - 1) },
    { HTTPs_METHOD_CONNECT, HTTPs_STR_METHOD_CONNECT, (sizeof(HTTPs_STR_METHOD_CONNECT) - 1) },
};


/*
*********************************************************************************************************
*                                            HTTP VERSION
*********************************************************************************************************
*/

#define  HTTPs_STR_REQ_PROTOCOL_VER_0_9                     "HTTP/0.9"
#define  HTTPs_STR_REQ_PROTOCOL_VER_1_0                     "HTTP/1.0"
#define  HTTPs_STR_REQ_PROTOCOL_VER_1_1                     "HTTP/1.1"

static  const  HTTPs_DICTIONARY   HTTPs_DictionaryReqProtocolVers[] = {
    { HTTPs_REQ_PROTOCOL_VER_0_9, HTTPs_STR_REQ_PROTOCOL_VER_0_9, (sizeof(HTTPs_STR_REQ_PROTOCOL_VER_0_9) - 1) },
    { HTTPs_REQ_PROTOCOL_VER_1_0, HTTPs_STR_REQ_PROTOCOL_VER_1_0, (sizeof(HTTPs_STR_REQ_PROTOCOL_VER_1_0) - 1) },
    { HTTPs_REQ_PROTOCOL_VER_1_1, HTTPs_STR_REQ_PROTOCOL_VER_1_1, (sizeof(HTTPs_STR_REQ_PROTOCOL_VER_1_1) - 1) },
};


/*
*********************************************************************************************************
*                                          HTTPS STATUS CODE
*********************************************************************************************************
*/

#define  HTTPs_STR_STATUS_CODE_OK                               "200"
#define  HTTPs_STR_STATUS_CODE_CREATED                          "201"
#define  HTTPs_STR_STATUS_CODE_ACCEPTED                         "202"
#define  HTTPs_STR_STATUS_CODE_NO_CONTENT                       "204"
#define  HTTPs_STR_STATUS_CODE_RESET_CONTENT                    "205"
#define  HTTPs_STR_STATUS_CODE_MOVED_PERMANENTLY                "301"
#define  HTTPs_STR_STATUS_CODE_FOUND                            "302"
#define  HTTPs_STR_STATUS_CODE_SEE_OTHER                        "303"
#define  HTTPs_STR_STATUS_CODE_NOT_MODIFIED                     "304"
#define  HTTPs_STR_STATUS_CODE_USE_PROXY                        "305"
#define  HTTPs_STR_STATUS_CODE_TEMPORARY_REDIRECT               "307"
#define  HTTPs_STR_STATUS_CODE_BAD_REQUEST                      "400"
#define  HTTPs_STR_STATUS_CODE_UNAUTHORIZED                     "401"
#define  HTTPs_STR_STATUS_CODE_FORBIDDEN                        "403"
#define  HTTPs_STR_STATUS_CODE_NOT_FOUND                        "404"
#define  HTTPs_STR_STATUS_CODE_METHOD_NOT_ALLOWED               "405"
#define  HTTPs_STR_STATUS_CODE_NOT_ACCEPTABLE                   "406"
#define  HTTPs_STR_STATUS_CODE_REQUEST_TIMEOUT                  "408"
#define  HTTPs_STR_STATUS_CODE_CONFLIT                          "409"
#define  HTTPs_STR_STATUS_CODE_GONE                             "410"
#define  HTTPs_STR_STATUS_CODE_LENGTH_REQUIRED                  "411"
#define  HTTPs_STR_STATUS_CODE_PRECONDITION_FAILED              "412"
#define  HTTPs_STR_STATUS_CODE_REQUEST_ENTITY_TOO_LARGE         "413"
#define  HTTPs_STR_STATUS_CODE_REQUEST_URI_TOO_LONG             "414"
#define  HTTPs_STR_STATUS_CODE_UNSUPPORTED_MEDIA_TYPE           "415"
#define  HTTPs_STR_STATUS_CODE_REQUESTED_RANGE_NOT_SATISFIABLE  "416"
#define  HTTPs_STR_STATUS_CODE_EXPECTATION_FAILED               "417"
#define  HTTPs_STR_STATUS_CODE_INTERNAL_SERVER_ERR              "500"
#define  HTTPs_STR_STATUS_CODE_NOT_IMPLEMENTED                  "501"
#define  HTTPs_STR_STATUS_CODE_SERVICE_UNAVAILABLE              "503"
#define  HTTPs_STR_STATUS_CODE_HTTP_VERSION_NOT_SUPPORTED       "505"

static  const  HTTPs_DICTIONARY  HTTPs_DictionaryStatusCodes[] = {
    { HTTPs_STATUS_OK,                               HTTPs_STR_STATUS_CODE_OK,                               (sizeof(HTTPs_STR_STATUS_CODE_OK)                              - 1) },
    { HTTPs_STATUS_CREATED,                          HTTPs_STR_STATUS_CODE_CREATED,                          (sizeof(HTTPs_STR_STATUS_CODE_CREATED)                         - 1) },
    { HTTPs_STATUS_ACCEPTED,                         HTTPs_STR_STATUS_CODE_ACCEPTED,                         (sizeof(HTTPs_STR_STATUS_CODE_ACCEPTED)                        - 1) },
    { HTTPs_STATUS_NO_CONTENT,                       HTTPs_STR_STATUS_CODE_NO_CONTENT,                       (sizeof(HTTPs_STR_STATUS_CODE_NO_CONTENT)                      - 1) },
    { HTTPs_STATUS_RESET_CONTENT,                    HTTPs_STR_STATUS_CODE_RESET_CONTENT,                    (sizeof(HTTPs_STR_STATUS_CODE_RESET_CONTENT)                   - 1) },
    { HTTPs_STATUS_MOVED_PERMANENTLY,                HTTPs_STR_STATUS_CODE_MOVED_PERMANENTLY,                (sizeof(HTTPs_STR_STATUS_CODE_MOVED_PERMANENTLY)               - 1) },
    { HTTPs_STATUS_FOUND,                            HTTPs_STR_STATUS_CODE_FOUND,                            (sizeof(HTTPs_STR_STATUS_CODE_FOUND)                           - 1) },
    { HTTPs_STATUS_SEE_OTHER,                        HTTPs_STR_STATUS_CODE_SEE_OTHER,                        (sizeof(HTTPs_STR_STATUS_CODE_SEE_OTHER)                       - 1) },
    { HTTPs_STATUS_NOT_MODIFIED,                     HTTPs_STR_STATUS_CODE_NOT_MODIFIED,                     (sizeof(HTTPs_STR_STATUS_CODE_NOT_MODIFIED)                    - 1) },
    { HTTPs_STATUS_USE_PROXY,                        HTTPs_STR_STATUS_CODE_USE_PROXY,                        (sizeof(HTTPs_STR_STATUS_CODE_USE_PROXY)                       - 1) },
    { HTTPs_STATUS_TEMPORARY_REDIRECT,               HTTPs_STR_STATUS_CODE_TEMPORARY_REDIRECT,               (sizeof(HTTPs_STR_STATUS_CODE_TEMPORARY_REDIRECT)              - 1) },
    { HTTPs_STATUS_BAD_REQUEST,                      HTTPs_STR_STATUS_CODE_BAD_REQUEST,                      (sizeof(HTTPs_STR_STATUS_CODE_BAD_REQUEST)                     - 1) },
    { HTTPs_STATUS_UNAUTHORIZED,                     HTTPs_STR_STATUS_CODE_UNAUTHORIZED,                     (sizeof(HTTPs_STR_STATUS_CODE_UNAUTHORIZED)                    - 1) },
    { HTTPs_STATUS_FORBIDDEN,                        HTTPs_STR_STATUS_CODE_FORBIDDEN,                        (sizeof(HTTPs_STR_STATUS_CODE_FORBIDDEN)                       - 1) },
    { HTTPs_STATUS_NOT_FOUND,                        HTTPs_STR_STATUS_CODE_NOT_FOUND,                        (sizeof(HTTPs_STR_STATUS_CODE_NOT_FOUND)                       - 1) },
    { HTTPs_STATUS_METHOD_NOT_ALLOWED,               HTTPs_STR_STATUS_CODE_METHOD_NOT_ALLOWED,               (sizeof(HTTPs_STR_STATUS_CODE_METHOD_NOT_ALLOWED)              - 1) },
    { HTTPs_STATUS_NOT_ACCEPTABLE,                   HTTPs_STR_STATUS_CODE_NOT_ACCEPTABLE,                   (sizeof(HTTPs_STR_STATUS_CODE_NOT_ACCEPTABLE)                  - 1) },
    { HTTPs_STATUS_REQUEST_TIMEOUT,                  HTTPs_STR_STATUS_CODE_REQUEST_TIMEOUT,                  (sizeof(HTTPs_STR_STATUS_CODE_REQUEST_TIMEOUT)                 - 1) },
    { HTTPs_STATUS_CONFLIT,                          HTTPs_STR_STATUS_CODE_CONFLIT,                          (sizeof(HTTPs_STR_STATUS_CODE_CONFLIT)                         - 1) },
    { HTTPs_STATUS_GONE,                             HTTPs_STR_STATUS_CODE_GONE,                             (sizeof(HTTPs_STR_STATUS_CODE_GONE)                            - 1) },
    { HTTPs_STATUS_LENGTH_REQUIRED,                  HTTPs_STR_STATUS_CODE_LENGTH_REQUIRED,                  (sizeof(HTTPs_STR_STATUS_CODE_LENGTH_REQUIRED)                 - 1) },
    { HTTPs_STATUS_PRECONDITION_FAILED,              HTTPs_STR_STATUS_CODE_PRECONDITION_FAILED,              (sizeof(HTTPs_STR_STATUS_CODE_PRECONDITION_FAILED)             - 1) },
    { HTTPs_STATUS_REQUEST_ENTITY_TOO_LARGE,         HTTPs_STR_STATUS_CODE_REQUEST_ENTITY_TOO_LARGE,         (sizeof(HTTPs_STR_STATUS_CODE_REQUEST_ENTITY_TOO_LARGE)        - 1) },
    { HTTPs_STATUS_REQUEST_URI_TOO_LONG,             HTTPs_STR_STATUS_CODE_REQUEST_URI_TOO_LONG,             (sizeof(HTTPs_STR_STATUS_CODE_REQUEST_URI_TOO_LONG)            - 1) },
    { HTTPs_STATUS_UNSUPPORTED_MEDIA_TYPE,           HTTPs_STR_STATUS_CODE_UNSUPPORTED_MEDIA_TYPE,           (sizeof(HTTPs_STR_STATUS_CODE_UNSUPPORTED_MEDIA_TYPE)          - 1) },
    { HTTPs_STATUS_REQUESTED_RANGE_NOT_SATISFIABLE,  HTTPs_STR_STATUS_CODE_REQUESTED_RANGE_NOT_SATISFIABLE,  (sizeof(HTTPs_STR_STATUS_CODE_REQUESTED_RANGE_NOT_SATISFIABLE) - 1) },
    { HTTPs_STATUS_EXPECTATION_FAILED,               HTTPs_STR_STATUS_CODE_EXPECTATION_FAILED,               (sizeof(HTTPs_STR_STATUS_CODE_EXPECTATION_FAILED)              - 1) },
    { HTTPs_STATUS_INTERNAL_SERVER_ERR,              HTTPs_STR_STATUS_CODE_INTERNAL_SERVER_ERR,              (sizeof(HTTPs_STR_STATUS_CODE_INTERNAL_SERVER_ERR)             - 1) },
    { HTTPs_STATUS_NOT_IMPLEMENTED,                  HTTPs_STR_STATUS_CODE_NOT_IMPLEMENTED,                  (sizeof(HTTPs_STR_STATUS_CODE_NOT_IMPLEMENTED)                 - 1) },
    { HTTPs_STATUS_SERVICE_UNAVAILABLE,              HTTPs_STR_STATUS_CODE_SERVICE_UNAVAILABLE,              (sizeof(HTTPs_STR_STATUS_CODE_SERVICE_UNAVAILABLE)             - 1) },
    { HTTPs_STATUS_HTTP_VERSION_NOT_SUPPORTED,       HTTPs_STR_STATUS_CODE_HTTP_VERSION_NOT_SUPPORTED,       (sizeof(HTTPs_STR_STATUS_CODE_HTTP_VERSION_NOT_SUPPORTED)      - 1) },
};


#define  HTTPs_STR_REASON_PHRASE_OK                               "OK"
#define  HTTPs_STR_REASON_PHRASE_CREATED                          "Created"
#define  HTTPs_STR_REASON_PHRASE_ACCEPTED                         "Accepted"
#define  HTTPs_STR_REASON_PHRASE_NO_CONTENT                       "No Content"
#define  HTTPs_STR_REASON_PHRASE_RESET_CONTENT                    "Reset Content"
#define  HTTPs_STR_REASON_PHRASE_MOVED_PERMANENTLY                "Moved Permanently"
#define  HTTPs_STR_REASON_PHRASE_FOUND                            "Found"
#define  HTTPs_STR_REASON_PHRASE_SEE_OTHER                        "See Other"
#define  HTTPs_STR_REASON_PHRASE_NOT_MODIFIED                     "Not Modified"
#define  HTTPs_STR_REASON_PHRASE_USE_PROXY                        "Use Proxy"
#define  HTTPs_STR_REASON_PHRASE_TEMPORARY_REDIRECT               "Temporary Redirect"
#define  HTTPs_STR_REASON_PHRASE_BAD_REQUEST                      "Bad Request"
#define  HTTPs_STR_REASON_PHRASE_UNAUTHORIZED                     "Unauthorized"
#define  HTTPs_STR_REASON_PHRASE_FORBIDDEN                        "Forbidden"
#define  HTTPs_STR_REASON_PHRASE_NOT_FOUND                        "Not Found"
#define  HTTPs_STR_REASON_PHRASE_METHOD_NOT_ALLOWED               "Method Not Allowed"
#define  HTTPs_STR_REASON_PHRASE_NOT_ACCEPTABLE                   "Not Acceptable"
#define  HTTPs_STR_REASON_PHRASE_REQUEST_TIMEOUT                  "Request Timeout"
#define  HTTPs_STR_REASON_PHRASE_CONFLIT                          "Conflit"
#define  HTTPs_STR_REASON_PHRASE_GONE                             "Gone"
#define  HTTPs_STR_REASON_PHRASE_LENGTH_REQUIRED                  "Length Required"
#define  HTTPs_STR_REASON_PHRASE_PRECONDITION_FAILED              "Precondition Failed"
#define  HTTPs_STR_REASON_PHRASE_REQUEST_ENTITY_TOO_LARGE         "Request Entity Too Large"
#define  HTTPs_STR_REASON_PHRASE_REQUEST_URI_TOO_LONG             "Request URI Too Long"
#define  HTTPs_STR_REASON_PHRASE_UNSUPPORTED_MEDIA_TYPE           "Unsupported Media Type"
#define  HTTPs_STR_REASON_PHRASE_REQUESTED_RANGE_NOT_SATISFIABLE  "Requested Range Not Satisfiable"
#define  HTTPs_STR_REASON_PHRASE_EXPECTATION_FAILED               "Expectation Failed"
#define  HTTPs_STR_REASON_PHRASE_INTERNAL_SERVER_ERR              "Internal Server Error"
#define  HTTPs_STR_REASON_PHRASE_NOT_IMPLEMENTED                  "Not Implemented"
#define  HTTPs_STR_REASON_PHRASE_BAD_GATEWAY                      "Bad Gateway"
#define  HTTPs_STR_REASON_PHRASE_SERVICE_UNAVAILABLE              "Service Unavailable"
#define  HTTPs_STR_REASON_PHRASE_HTTP_VERSION_NOT_SUPPORTED       "HTTP Version Not Supported"

static  const  HTTPs_DICTIONARY  HTTPs_DictionaryReasonPhrases[] = {
    { HTTPs_STATUS_OK,                               HTTPs_STR_REASON_PHRASE_OK,                               (sizeof(HTTPs_STR_REASON_PHRASE_OK)                              - 1) },
    { HTTPs_STATUS_CREATED,                          HTTPs_STR_REASON_PHRASE_CREATED,                          (sizeof(HTTPs_STR_REASON_PHRASE_CREATED)                         - 1) },
    { HTTPs_STATUS_ACCEPTED,                         HTTPs_STR_REASON_PHRASE_ACCEPTED,                         (sizeof(HTTPs_STR_REASON_PHRASE_ACCEPTED)                        - 1) },
    { HTTPs_STATUS_NO_CONTENT,                       HTTPs_STR_REASON_PHRASE_NO_CONTENT,                       (sizeof(HTTPs_STR_REASON_PHRASE_NO_CONTENT)                      - 1) },
    { HTTPs_STATUS_RESET_CONTENT,                    HTTPs_STR_REASON_PHRASE_RESET_CONTENT,                    (sizeof(HTTPs_STR_REASON_PHRASE_RESET_CONTENT)                   - 1) },
    { HTTPs_STATUS_MOVED_PERMANENTLY,                HTTPs_STR_REASON_PHRASE_MOVED_PERMANENTLY,                (sizeof(HTTPs_STR_REASON_PHRASE_MOVED_PERMANENTLY)               - 1) },
    { HTTPs_STATUS_FOUND,                            HTTPs_STR_REASON_PHRASE_FOUND,                            (sizeof(HTTPs_STR_REASON_PHRASE_FOUND)                           - 1) },
    { HTTPs_STATUS_SEE_OTHER,                        HTTPs_STR_REASON_PHRASE_SEE_OTHER,                        (sizeof(HTTPs_STR_REASON_PHRASE_SEE_OTHER)                       - 1) },
    { HTTPs_STATUS_NOT_MODIFIED,                     HTTPs_STR_REASON_PHRASE_NOT_MODIFIED,                     (sizeof(HTTPs_STR_REASON_PHRASE_NOT_MODIFIED)                    - 1) },
    { HTTPs_STATUS_USE_PROXY,                        HTTPs_STR_REASON_PHRASE_USE_PROXY,                        (sizeof(HTTPs_STR_REASON_PHRASE_USE_PROXY)                       - 1) },
    { HTTPs_STATUS_TEMPORARY_REDIRECT,               HTTPs_STR_REASON_PHRASE_TEMPORARY_REDIRECT,               (sizeof(HTTPs_STR_REASON_PHRASE_TEMPORARY_REDIRECT)              - 1) },
    { HTTPs_STATUS_BAD_REQUEST,                      HTTPs_STR_REASON_PHRASE_BAD_REQUEST,                      (sizeof(HTTPs_STR_REASON_PHRASE_BAD_REQUEST)                     - 1) },
    { HTTPs_STATUS_UNAUTHORIZED,                     HTTPs_STR_REASON_PHRASE_UNAUTHORIZED,                     (sizeof(HTTPs_STR_REASON_PHRASE_UNAUTHORIZED)                    - 1) },
    { HTTPs_STATUS_FORBIDDEN,                        HTTPs_STR_REASON_PHRASE_FORBIDDEN,                        (sizeof(HTTPs_STR_REASON_PHRASE_FORBIDDEN)                       - 1) },
    { HTTPs_STATUS_NOT_FOUND,                        HTTPs_STR_REASON_PHRASE_NOT_FOUND,                        (sizeof(HTTPs_STR_REASON_PHRASE_NOT_FOUND)                       - 1) },
    { HTTPs_STATUS_METHOD_NOT_ALLOWED,               HTTPs_STR_REASON_PHRASE_METHOD_NOT_ALLOWED,               (sizeof(HTTPs_STR_REASON_PHRASE_METHOD_NOT_ALLOWED)              - 1) },
    { HTTPs_STATUS_NOT_ACCEPTABLE,                   HTTPs_STR_REASON_PHRASE_NOT_ACCEPTABLE,                   (sizeof(HTTPs_STR_REASON_PHRASE_NOT_ACCEPTABLE)                  - 1) },
    { HTTPs_STATUS_REQUEST_TIMEOUT,                  HTTPs_STR_REASON_PHRASE_REQUEST_TIMEOUT,                  (sizeof(HTTPs_STR_REASON_PHRASE_REQUEST_TIMEOUT)                 - 1) },
    { HTTPs_STATUS_CONFLIT,                          HTTPs_STR_REASON_PHRASE_CONFLIT,                          (sizeof(HTTPs_STR_REASON_PHRASE_CONFLIT)                         - 1) },
    { HTTPs_STATUS_GONE,                             HTTPs_STR_REASON_PHRASE_GONE,                             (sizeof(HTTPs_STR_REASON_PHRASE_GONE)                            - 1) },
    { HTTPs_STATUS_LENGTH_REQUIRED,                  HTTPs_STR_REASON_PHRASE_LENGTH_REQUIRED,                  (sizeof(HTTPs_STR_REASON_PHRASE_LENGTH_REQUIRED)                 - 1) },
    { HTTPs_STATUS_PRECONDITION_FAILED,              HTTPs_STR_REASON_PHRASE_PRECONDITION_FAILED,              (sizeof(HTTPs_STR_REASON_PHRASE_PRECONDITION_FAILED)             - 1) },
    { HTTPs_STATUS_REQUEST_ENTITY_TOO_LARGE,         HTTPs_STR_REASON_PHRASE_REQUEST_ENTITY_TOO_LARGE,         (sizeof(HTTPs_STR_REASON_PHRASE_REQUEST_ENTITY_TOO_LARGE)        - 1) },
    { HTTPs_STATUS_REQUEST_URI_TOO_LONG,             HTTPs_STR_REASON_PHRASE_REQUEST_URI_TOO_LONG,             (sizeof(HTTPs_STR_REASON_PHRASE_REQUEST_URI_TOO_LONG)            - 1) },
    { HTTPs_STATUS_UNSUPPORTED_MEDIA_TYPE,           HTTPs_STR_REASON_PHRASE_UNSUPPORTED_MEDIA_TYPE,           (sizeof(HTTPs_STR_REASON_PHRASE_UNSUPPORTED_MEDIA_TYPE)          - 1) },
    { HTTPs_STATUS_REQUESTED_RANGE_NOT_SATISFIABLE,  HTTPs_STR_REASON_PHRASE_REQUESTED_RANGE_NOT_SATISFIABLE,  (sizeof(HTTPs_STR_REASON_PHRASE_REQUESTED_RANGE_NOT_SATISFIABLE) - 1) },
    { HTTPs_STATUS_EXPECTATION_FAILED,               HTTPs_STR_REASON_PHRASE_EXPECTATION_FAILED,               (sizeof(HTTPs_STR_REASON_PHRASE_EXPECTATION_FAILED)              - 1) },
    { HTTPs_STATUS_INTERNAL_SERVER_ERR,              HTTPs_STR_REASON_PHRASE_INTERNAL_SERVER_ERR,              (sizeof(HTTPs_STR_REASON_PHRASE_INTERNAL_SERVER_ERR)             - 1) },
    { HTTPs_STATUS_NOT_IMPLEMENTED,                  HTTPs_STR_REASON_PHRASE_NOT_IMPLEMENTED,                  (sizeof(HTTPs_STR_REASON_PHRASE_NOT_IMPLEMENTED)                 - 1) },
    { HTTPs_STATUS_SERVICE_UNAVAILABLE,              HTTPs_STR_REASON_PHRASE_SERVICE_UNAVAILABLE,              (sizeof(HTTPs_STR_REASON_PHRASE_SERVICE_UNAVAILABLE)             - 1) },
    { HTTPs_STATUS_HTTP_VERSION_NOT_SUPPORTED,       HTTPs_STR_REASON_PHRASE_HTTP_VERSION_NOT_SUPPORTED,       (sizeof(HTTPs_STR_REASON_PHRASE_HTTP_VERSION_NOT_SUPPORTED)      - 1) },
};


/*
*********************************************************************************************************
*                                          HTTP CONTENT TYPE
*********************************************************************************************************
*/

#define  HTTPs_STR_FILE_EXT_HTM                             "htm"
#define  HTTPs_STR_FILE_EXT_HTML                            "html"
#define  HTTPs_STR_FILE_EXT_GIF                             "gif"
#define  HTTPs_STR_FILE_EXT_JPEG                            "jpeg"
#define  HTTPs_STR_FILE_EXT_JPG                             "jpg"
#define  HTTPs_STR_FILE_EXT_PNG                             "png"
#define  HTTPs_STR_FILE_EXT_JS                              "js"
#define  HTTPs_STR_FILE_EXT_TXT                             "txt"
#define  HTTPs_STR_FILE_EXT_CSS                             "css"
#define  HTTPs_STR_FILE_EXT_PDF                             "pdf"
#define  HTTPs_STR_FILE_EXT_ZIP                             "zip"
#define  HTTPs_STR_FILE_EXT_ASTERISK                        "*"
#define  HTTPs_STR_FILE_EXT_CLASS                           "class"

static  const  HTTPs_DICTIONARY  HTTPs_DictionaryFileExts[] = {
    { HTTPs_CONTENT_TYPE_HTML,         HTTPs_STR_FILE_EXT_HTM,      (sizeof(HTTPs_STR_FILE_EXT_HTM)      - 1) },
    { HTTPs_CONTENT_TYPE_HTML,         HTTPs_STR_FILE_EXT_HTML,     (sizeof(HTTPs_STR_FILE_EXT_HTML)     - 1) },
    { HTTPs_CONTENT_TYPE_GIF,          HTTPs_STR_FILE_EXT_GIF,      (sizeof(HTTPs_STR_FILE_EXT_GIF)      - 1) },
    { HTTPs_CONTENT_TYPE_JPEG,         HTTPs_STR_FILE_EXT_JPEG,     (sizeof(HTTPs_STR_FILE_EXT_JPEG)     - 1) },
    { HTTPs_CONTENT_TYPE_JPEG,         HTTPs_STR_FILE_EXT_JPG,      (sizeof(HTTPs_STR_FILE_EXT_JPG)      - 1) },
    { HTTPs_CONTENT_TYPE_PNG,          HTTPs_STR_FILE_EXT_PNG,      (sizeof(HTTPs_STR_FILE_EXT_PNG)      - 1) },
    { HTTPs_CONTENT_TYPE_JS,           HTTPs_STR_FILE_EXT_JS,       (sizeof(HTTPs_STR_FILE_EXT_JS)       - 1) },
    { HTTPs_CONTENT_TYPE_PLAIN,        HTTPs_STR_FILE_EXT_TXT,      (sizeof(HTTPs_STR_FILE_EXT_TXT)      - 1) },
    { HTTPs_CONTENT_TYPE_CSS,          HTTPs_STR_FILE_EXT_CSS,      (sizeof(HTTPs_STR_FILE_EXT_CSS)      - 1) },
    { HTTPs_CONTENT_TYPE_PDF,          HTTPs_STR_FILE_EXT_PDF,      (sizeof(HTTPs_STR_FILE_EXT_PDF)      - 1) },
    { HTTPs_CONTENT_TYPE_ZIP,          HTTPs_STR_FILE_EXT_ZIP,      (sizeof(HTTPs_STR_FILE_EXT_ZIP)      - 1) },
    { HTTPs_CONTENT_TYPE_OCTET_STREAM, HTTPs_STR_FILE_EXT_ASTERISK, (sizeof(HTTPs_STR_FILE_EXT_ASTERISK) - 1) },
    { HTTPs_CONTENT_TYPE_OCTET_STREAM, HTTPs_STR_FILE_EXT_CLASS,    (sizeof(HTTPs_STR_FILE_EXT_CLASS)    - 1) },
};


#define  HTTPs_STR_CONTENT_TYPE_HTML                        "text/html"
#define  HTTPs_STR_CONTENT_TYPE_GIF                         "image/gif"
#define  HTTPs_STR_CONTENT_TYPE_JPEG                        "image/jpeg"
#define  HTTPs_STR_CONTENT_TYPE_PNG                         "image/png"
#define  HTTPs_STR_CONTENT_TYPE_JS                          "text/javascript"
#define  HTTPs_STR_CONTENT_TYPE_PLAIN                       "text/plain"
#define  HTTPs_STR_CONTENT_TYPE_CSS                         "text/css"
#define  HTTPs_STR_CONTENT_TYPE_OCTET_STREAM                "application/octet-stream"
#define  HTTPs_STR_CONTENT_TYPE_PDF                         "application/pdf"
#define  HTTPs_STR_CONTENT_TYPE_ZIP                         "application/zip"

static  const  HTTPs_DICTIONARY  HTTPs_DictionaryContentTypes[] = {
    { HTTPs_CONTENT_TYPE_HTML,         HTTPs_STR_CONTENT_TYPE_HTML,         (sizeof(HTTPs_STR_CONTENT_TYPE_HTML)         - 1) },
    { HTTPs_CONTENT_TYPE_GIF,          HTTPs_STR_CONTENT_TYPE_GIF,          (sizeof(HTTPs_STR_CONTENT_TYPE_GIF)          - 1) },
    { HTTPs_CONTENT_TYPE_JPEG,         HTTPs_STR_CONTENT_TYPE_JPEG,         (sizeof(HTTPs_STR_CONTENT_TYPE_JPEG)         - 1) },
    { HTTPs_CONTENT_TYPE_PNG,          HTTPs_STR_CONTENT_TYPE_PNG,          (sizeof(HTTPs_STR_CONTENT_TYPE_PNG)          - 1) },
    { HTTPs_CONTENT_TYPE_JS,           HTTPs_STR_CONTENT_TYPE_JS,           (sizeof(HTTPs_STR_CONTENT_TYPE_JS)           - 1) },
    { HTTPs_CONTENT_TYPE_PLAIN,        HTTPs_STR_CONTENT_TYPE_PLAIN,        (sizeof(HTTPs_STR_CONTENT_TYPE_PLAIN)        - 1) },
    { HTTPs_CONTENT_TYPE_CSS,          HTTPs_STR_CONTENT_TYPE_CSS,          (sizeof(HTTPs_STR_CONTENT_TYPE_CSS)          - 1) },
    { HTTPs_CONTENT_TYPE_OCTET_STREAM, HTTPs_STR_CONTENT_TYPE_OCTET_STREAM, (sizeof(HTTPs_STR_CONTENT_TYPE_OCTET_STREAM) - 1) },
    { HTTPs_CONTENT_TYPE_PDF,          HTTPs_STR_CONTENT_TYPE_PDF,          (sizeof(HTTPs_STR_CONTENT_TYPE_PDF)          - 1) },
    { HTTPs_CONTENT_TYPE_ZIP,          HTTPs_STR_CONTENT_TYPE_ZIP,          (sizeof(HTTPs_STR_CONTENT_TYPE_ZIP)          - 1) },
};


/*
*********************************************************************************************************
*                                      HTTP REQUEST CONTENT TYPE
*********************************************************************************************************
*/

#define  HTTPs_STR_REQ_CONTENT_TYPE_APP                     "application/x-www-form-urlencoded"
#define  HTTPs_STR_REQ_CONTENT_TYPE_MULTIPART               "multipart/form-data"
#define  HTTPs_STR_REQ_CONTENT_TYPE_BOUNDARY                "boundary"

static  const  HTTPs_DICTIONARY  HTTPs_DictionaryReqContentTypes[] = {
    { HTTPs_REQ_CONTENT_TYPE_APP,       HTTPs_STR_REQ_CONTENT_TYPE_APP,       (sizeof(HTTPs_STR_REQ_CONTENT_TYPE_APP      ) - 1) },
    { HTTPs_REQ_CONTENT_TYPE_MULTIPART, HTTPs_STR_REQ_CONTENT_TYPE_MULTIPART, (sizeof(HTTPs_STR_REQ_CONTENT_TYPE_MULTIPART) - 1) },
    { HTTPs_REQ_CONTENT_TYPE_BOUNDARY,  HTTPs_STR_REQ_CONTENT_TYPE_BOUNDARY,  (sizeof(HTTPs_STR_REQ_CONTENT_TYPE_BOUNDARY)  - 1) },
};


/*
*********************************************************************************************************
*                                          HTTP HEADER FIELD
*********************************************************************************************************
*/

#define  HTTPs_STR_HDR_FIELD_CONTENT_TYPE                   "Content-Type"
#define  HTTPs_STR_HDR_FIELD_CONTENT_LEN                    "Content-Length"
#define  HTTPs_STR_HDR_FIELD_HOST                           "Host"
#define  HTTPs_STR_HDR_FIELD_LOCATION                       "Location"
#define  HTTPs_STR_HDR_FIELD_CONN                           "Connection"
#define  HTTPs_STR_HDR_FIELD_TRANSFER_ENCODING              "Transfer-Encoding"
#if (HTTPs_CFG_HDR_EN == DEF_ENABLED)
#define  HTTPs_STR_HDR_FIELD_ACCEPT                         "Accept"
#define  HTTPs_STR_HDR_FIELD_ACCEPT_CHARSET                 "Accept-Charset"
#define  HTTPs_STR_HDR_FIELD_ACCEPT_ENCODING                "Accept-Encoding"
#define  HTTPs_STR_HDR_FIELD_ACCEPT_LANGUAGE                "Accept-Language"
#define  HTTPs_STR_HDR_FIELD_ACCEPT_RANGES                  "Accept-Ranges"
#define  HTTPs_STR_HDR_FIELD_AGE                            "Age"
#define  HTTPs_STR_HDR_FIELD_ALLOW                          "Allow"
#define  HTTPs_STR_HDR_FIELD_AUTHORIZATION                  "Authorization"
#define  HTTPs_STR_HDR_FIELD_CLIENT_IP                      "Client-ip"
#define  HTTPs_STR_HDR_FIELD_CONTENT_BASE                   "Content-Base"
#define  HTTPs_STR_HDR_FIELD_CONTENT_ENCODING               "Content-Encoding"
#define  HTTPs_STR_HDR_FIELD_CONTENT_LANGUAGE               "Content-Language"
#define  HTTPs_STR_HDR_FIELD_CONTENT_LOCATION               "Content-Location"
#define  HTTPs_STR_HDR_FIELD_CONTENT_MD5                    "Content-MD5"
#define  HTTPs_STR_HDR_FIELD_CONTENT_RANGE                  "Content-Range"
#define  HTTPs_STR_HDR_FIELD_COOKIE                         "Cookie"
#define  HTTPs_STR_HDR_FIELD_COOKIE2                        "Cookie2"
#define  HTTPs_STR_HDR_FIELD_DATE                           "Date"
#define  HTTPs_STR_HDR_FIELD_ETAG                           "ETag"
#define  HTTPs_STR_HDR_FIELD_EXPECT                         "Expect"
#define  HTTPs_STR_HDR_FIELD_EXPIRES                        "Expires"
#define  HTTPs_STR_HDR_FIELD_FROM                           "From"
#define  HTTPs_STR_HDR_FIELD_IF_MODIFIED_SINCE              "If-Modified-Since"
#define  HTTPs_STR_HDR_FIELD_IF_MATCH                       "If-Match"
#define  HTTPs_STR_HDR_FIELD_IF_NONE_MATCH                  "If-None-Match"
#define  HTTPs_STR_HDR_FIELD_IF_RANGE                       "If-Range"
#define  HTTPs_STR_HDR_FIELD_IF_UNMODIFIED_SINCE            "If-Unmodified-Since"
#define  HTTPs_STR_HDR_FIELD_LAST_MODIFIED                  "Last-Modified"
#define  HTTPs_STR_HDR_FIELD_PUBLIC                         "Public"
#define  HTTPs_STR_HDR_FIELD_RANGE                          "Range"
#define  HTTPs_STR_HDR_FIELD_REFERER                        "Referer"
#define  HTTPs_STR_HDR_FIELD_RETRY_AFTER                    "Retry-After"
#define  HTTPs_STR_HDR_FIELD_SERVER                         "Server"
#define  HTTPs_STR_HDR_FIELD_SET_COOKIE                     "Set-Cookie"
#define  HTTPs_STR_HDR_FIELD_SET_COOKIE2                    "Set-Cookie2"
#define  HTTPs_STR_HDR_FIELD_TE                             "TE"
#define  HTTPs_STR_HDR_FIELD_TRAILER                        "Trailer"
#define  HTTPs_STR_HDR_FIELD_UPGRATE                        "Upgrade"
#define  HTTPs_STR_HDR_FIELD_USER_AGENT                     "User-Agent"
#define  HTTPs_STR_HDR_FIELD_VARY                           "Vary"
#define  HTTPs_STR_HDR_FIELD_VIA                            "Via"
#define  HTTPs_STR_HDR_FIELD_WARNING                        "Warning"
#define  HTTPs_STR_HDR_FIELD_WWW_AUTHENTICATE               "WWW-Authenticate"
#endif

                                                                /* Len of Hdr Field string names in Request messages     */
#define  HTTPs_STR_HDR_FIELD_CONTENT_TYPE_LEN               (sizeof(HTTPs_STR_HDR_FIELD_CONTENT_TYPE)        - 1)
#define  HTTPs_STR_HDR_FIELD_CONTENT_LEN_LEN                (sizeof(HTTPs_STR_HDR_FIELD_CONTENT_LEN)         - 1)
#define  HTTPs_STR_HDR_FIELD_HOST_LEN                       (sizeof(HTTPs_STR_HDR_FIELD_HOST)                - 1)
#if (HTTPs_CFG_HDR_EN == DEF_ENABLED)
#define  HTTPs_STR_HDR_FIELD_ACCEPT_LEN                     (sizeof(HTTPs_STR_HDR_FIELD_ACCEPT)              - 1)
#define  HTTPs_STR_HDR_FIELD_ACCEPT_CHARSET_LEN             (sizeof(HTTPs_STR_HDR_FIELD_ACCEPT_CHARSET)      - 1)
#define  HTTPs_STR_HDR_FIELD_ACCEPT_ENCODING_LEN            (sizeof(HTTPs_STR_HDR_FIELD_ACCEPT_ENCODING)     - 1)
#define  HTTPs_STR_HDR_FIELD_ACCEPT_LANGUAGE_LEN            (sizeof(HTTPs_STR_HDR_FIELD_ACCEPT_LANGUAGE)     - 1)
#define  HTTPs_STR_HDR_FIELD_AUTHORIZATION_LEN              (sizeof(HTTPs_STR_HDR_FIELD_AUTHORIZATION)       - 1)
#define  HTTPs_STR_HDR_FIELD_CLIENT_IP_LEN                  (sizeof(HTTPs_STR_HDR_FIELD_CLIENT_IP)           - 1)
#define  HTTPs_STR_HDR_FIELD_COOKIE_LEN                     (sizeof(HTTPs_STR_HDR_FIELD_COOKIE)              - 1)
#define  HTTPs_STR_HDR_FIELD_COOKIE2_LEN                    (sizeof(HTTPs_STR_HDR_FIELD_COOKIE2)             - 1)
#define  HTTPs_STR_HDR_FIELD_DATE_LEN                       (sizeof(HTTPs_STR_HDR_FIELD_DATE)                - 1)
#define  HTTPs_STR_HDR_FIELD_EXPECT_LEN                     (sizeof(HTTPs_STR_HDR_FIELD_EXPECT)              - 1)
#define  HTTPs_STR_HDR_FIELD_FROM_LEN                       (sizeof(HTTPs_STR_HDR_FIELD_FROM)                - 1)
#define  HTTPs_STR_HDR_FIELD_IF_MODIFIED_SINCE_LEN          (sizeof(HTTPs_STR_HDR_FIELD_IF_MODIFIED_SINCE)   - 1)
#define  HTTPs_STR_HDR_FIELD_IF_MATCH_LEN                   (sizeof(HTTPs_STR_HDR_FIELD_IF_MATCH)            - 1)
#define  HTTPs_STR_HDR_FIELD_IF_NONE_MATCH_LEN              (sizeof(HTTPs_STR_HDR_FIELD_IF_NONE_MATCH)       - 1)
#define  HTTPs_STR_HDR_FIELD_IF_RANGE_LEN                   (sizeof(HTTPs_STR_HDR_FIELD_IF_RANGE)            - 1)
#define  HTTPs_STR_HDR_FIELD_IF_UNMODIFIED_SINCE_LEN        (sizeof(HTTPs_STR_HDR_FIELD_IF_UNMODIFIED_SINCE) - 1)
#define  HTTPs_STR_HDR_FIELD_RANGE_LEN                      (sizeof(HTTPs_STR_HDR_FIELD_RANGE)               - 1)
#define  HTTPs_STR_HDR_FIELD_REFERER_LEN                    (sizeof(HTTPs_STR_HDR_FIELD_REFERER)             - 1)
#define  HTTPs_STR_HDR_FIELD_TE_LEN                         (sizeof(HTTPs_STR_HDR_FIELD_TE)                  - 1)
#define  HTTPs_STR_HDR_FIELD_UPGRATE_LEN                    (sizeof(HTTPs_STR_HDR_FIELD_UPGRATE)             - 1)
#define  HTTPs_STR_HDR_FIELD_USER_AGENT_LEN                 (sizeof(HTTPs_STR_HDR_FIELD_USER_AGENT)          - 1)
#define  HTTPs_STR_HDR_FIELD_VIA_LEN                        (sizeof(HTTPs_STR_HDR_FIELD_VIA)                 - 1)
#endif

static  const  HTTPs_DICTIONARY  HTTPs_DictionaryHdrFields[] = {
    { HTTPs_HDR_FIELD_CONTENT_TYPE,         HTTPs_STR_HDR_FIELD_CONTENT_TYPE,         (sizeof(HTTPs_STR_HDR_FIELD_CONTENT_TYPE)        - 1) },
    { HTTPs_HDR_FIELD_CONTENT_LEN,          HTTPs_STR_HDR_FIELD_CONTENT_LEN,          (sizeof(HTTPs_STR_HDR_FIELD_CONTENT_LEN)         - 1) },
    { HTTPs_HDR_FIELD_HOST,                 HTTPs_STR_HDR_FIELD_HOST,                 (sizeof(HTTPs_STR_HDR_FIELD_HOST)                - 1) },
    { HTTPs_HDR_FIELD_LOCATION,             HTTPs_STR_HDR_FIELD_LOCATION,             (sizeof(HTTPs_STR_HDR_FIELD_LOCATION)            - 1) },
    { HTTPs_HDR_FIELD_CONN,                 HTTPs_STR_HDR_FIELD_CONN,                 (sizeof(HTTPs_STR_HDR_FIELD_CONN)                - 1) },
    { HTTPs_HDR_FIELD_TRANSFER_ENCODING,    HTTPs_STR_HDR_FIELD_TRANSFER_ENCODING,    (sizeof(HTTPs_STR_HDR_FIELD_TRANSFER_ENCODING)   - 1) },
#if (HTTPs_CFG_HDR_EN == DEF_ENABLED)
    { HTTPs_HDR_FIELD_ACCEPT,               HTTPs_STR_HDR_FIELD_ACCEPT,               (sizeof(HTTPs_STR_HDR_FIELD_ACCEPT)              - 1) },
    { HTTPs_HDR_FIELD_ACCEPT_CHARSET,       HTTPs_STR_HDR_FIELD_ACCEPT_CHARSET,       (sizeof(HTTPs_STR_HDR_FIELD_ACCEPT_CHARSET)      - 1) },
    { HTTPs_HDR_FIELD_ACCEPT_ENCODING,      HTTPs_STR_HDR_FIELD_ACCEPT_ENCODING,      (sizeof(HTTPs_STR_HDR_FIELD_ACCEPT_ENCODING)     - 1) },
    { HTTPs_HDR_FIELD_ACCEPT_LANGUAGE,      HTTPs_STR_HDR_FIELD_ACCEPT_LANGUAGE,      (sizeof(HTTPs_STR_HDR_FIELD_ACCEPT_LANGUAGE)     - 1) },
    { HTTPs_HDR_FIELD_ACCEPT_RANGES,        HTTPs_STR_HDR_FIELD_ACCEPT_RANGES,        (sizeof(HTTPs_STR_HDR_FIELD_ACCEPT_RANGES)       - 1) },
    { HTTPs_HDR_FIELD_AGE,                  HTTPs_STR_HDR_FIELD_AGE,                  (sizeof(HTTPs_STR_HDR_FIELD_AGE)                 - 1) },
    { HTTPs_HDR_FIELD_ALLOW,                HTTPs_STR_HDR_FIELD_ALLOW,                (sizeof(HTTPs_STR_HDR_FIELD_ALLOW)               - 1) },
    { HTTPs_HDR_FIELD_AUTHORIZATION,        HTTPs_STR_HDR_FIELD_AUTHORIZATION,        (sizeof(HTTPs_STR_HDR_FIELD_AUTHORIZATION)       - 1) },
    { HTTPs_HDR_FIELD_CLIENT_IP,            HTTPs_STR_HDR_FIELD_CLIENT_IP,            (sizeof(HTTPs_STR_HDR_FIELD_CLIENT_IP)           - 1) },
    { HTTPs_HDR_FIELD_CONTENT_BASE,         HTTPs_STR_HDR_FIELD_CONTENT_BASE,         (sizeof(HTTPs_STR_HDR_FIELD_CONTENT_BASE)        - 1) },
    { HTTPs_HDR_FIELD_CONTENT_ENCODING,     HTTPs_STR_HDR_FIELD_CONTENT_ENCODING,     (sizeof(HTTPs_STR_HDR_FIELD_CONTENT_ENCODING)    - 1) },
    { HTTPs_HDR_FIELD_CONTENT_LANGUAGE,     HTTPs_STR_HDR_FIELD_CONTENT_LANGUAGE,     (sizeof(HTTPs_STR_HDR_FIELD_CONTENT_LANGUAGE)    - 1) },
    { HTTPs_HDR_FIELD_CONTENT_LOCATION,     HTTPs_STR_HDR_FIELD_CONTENT_LOCATION,     (sizeof(HTTPs_STR_HDR_FIELD_CONTENT_LOCATION)    - 1) },
    { HTTPs_HDR_FIELD_CONTENT_MD5,          HTTPs_STR_HDR_FIELD_CONTENT_MD5,          (sizeof(HTTPs_STR_HDR_FIELD_CONTENT_MD5)         - 1) },
    { HTTPs_HDR_FIELD_CONTENT_RANGE,        HTTPs_STR_HDR_FIELD_CONTENT_RANGE,        (sizeof(HTTPs_STR_HDR_FIELD_CONTENT_RANGE)       - 1) },
    { HTTPs_HDR_FIELD_COOKIE,               HTTPs_STR_HDR_FIELD_COOKIE,               (sizeof(HTTPs_STR_HDR_FIELD_COOKIE)              - 1) },
    { HTTPs_HDR_FIELD_COOKIE2,              HTTPs_STR_HDR_FIELD_COOKIE2,              (sizeof(HTTPs_STR_HDR_FIELD_COOKIE2)             - 1) },
    { HTTPs_HDR_FIELD_DATE,                 HTTPs_STR_HDR_FIELD_DATE,                 (sizeof(HTTPs_STR_HDR_FIELD_DATE)                - 1) },
    { HTTPs_HDR_FIELD_ETAG,                 HTTPs_STR_HDR_FIELD_ETAG,                 (sizeof(HTTPs_STR_HDR_FIELD_ETAG)                - 1) },
    { HTTPs_HDR_FIELD_EXPECT,               HTTPs_STR_HDR_FIELD_EXPECT,               (sizeof(HTTPs_STR_HDR_FIELD_EXPECT)              - 1) },
    { HTTPs_HDR_FIELD_EXPIRES,              HTTPs_STR_HDR_FIELD_EXPIRES,              (sizeof(HTTPs_STR_HDR_FIELD_EXPIRES)             - 1) },
    { HTTPs_HDR_FIELD_FROM,                 HTTPs_STR_HDR_FIELD_FROM,                 (sizeof(HTTPs_STR_HDR_FIELD_FROM)                - 1) },
    { HTTPs_HDR_FIELD_IF_MODIFIED_SINCE,    HTTPs_STR_HDR_FIELD_IF_MODIFIED_SINCE,    (sizeof(HTTPs_STR_HDR_FIELD_IF_MODIFIED_SINCE)   - 1) },
    { HTTPs_HDR_FIELD_IF_MATCH,             HTTPs_STR_HDR_FIELD_IF_MATCH,             (sizeof(HTTPs_STR_HDR_FIELD_IF_MATCH)            - 1) },
    { HTTPs_HDR_FIELD_IF_NONE_MATCH,        HTTPs_STR_HDR_FIELD_IF_NONE_MATCH,        (sizeof(HTTPs_STR_HDR_FIELD_IF_NONE_MATCH)       - 1) },
    { HTTPs_HDR_FIELD_IF_RANGE,             HTTPs_STR_HDR_FIELD_IF_RANGE,             (sizeof(HTTPs_STR_HDR_FIELD_IF_RANGE)            - 1) },
    { HTTPs_HDR_FIELD_IF_UNMODIFIED_SINCE,  HTTPs_STR_HDR_FIELD_IF_UNMODIFIED_SINCE,  (sizeof(HTTPs_STR_HDR_FIELD_IF_UNMODIFIED_SINCE) - 1) },
    { HTTPs_HDR_FIELD_LAST_MODIFIED,        HTTPs_STR_HDR_FIELD_LAST_MODIFIED,        (sizeof(HTTPs_STR_HDR_FIELD_LAST_MODIFIED)       - 1) },
    { HTTPs_HDR_FIELD_PUBLIC,               HTTPs_STR_HDR_FIELD_PUBLIC,               (sizeof(HTTPs_STR_HDR_FIELD_PUBLIC)              - 1) },
    { HTTPs_HDR_FIELD_RANGE,                HTTPs_STR_HDR_FIELD_RANGE,                (sizeof(HTTPs_STR_HDR_FIELD_RANGE)               - 1) },
    { HTTPs_HDR_FIELD_REFERER,              HTTPs_STR_HDR_FIELD_REFERER,              (sizeof(HTTPs_STR_HDR_FIELD_REFERER)             - 1) },
    { HTTPs_HDR_FIELD_RETRY_AFTER,          HTTPs_STR_HDR_FIELD_RETRY_AFTER,          (sizeof(HTTPs_STR_HDR_FIELD_RETRY_AFTER)         - 1) },
    { HTTPs_HDR_FIELD_SERVER,               HTTPs_STR_HDR_FIELD_SERVER,               (sizeof(HTTPs_STR_HDR_FIELD_SERVER)              - 1) },
    { HTTPs_HDR_FIELD_SET_COOKIE,           HTTPs_STR_HDR_FIELD_SET_COOKIE,           (sizeof(HTTPs_STR_HDR_FIELD_SET_COOKIE)          - 1) },
    { HTTPs_HDR_FIELD_SET_COOKIE2,          HTTPs_STR_HDR_FIELD_SET_COOKIE2,          (sizeof(HTTPs_STR_HDR_FIELD_SET_COOKIE2)         - 1) },
    { HTTPs_HDR_FIELD_TE,                   HTTPs_STR_HDR_FIELD_TE,                   (sizeof(HTTPs_STR_HDR_FIELD_TE)                  - 1) },
    { HTTPs_HDR_FIELD_TRAILER,              HTTPs_STR_HDR_FIELD_TRAILER,              (sizeof(HTTPs_STR_HDR_FIELD_TRAILER)             - 1) },
    { HTTPs_HDR_FIELD_UPGRATE,              HTTPs_STR_HDR_FIELD_UPGRATE,              (sizeof(HTTPs_STR_HDR_FIELD_UPGRATE)             - 1) },
    { HTTPs_HDR_FIELD_USER_AGENT,           HTTPs_STR_HDR_FIELD_USER_AGENT,           (sizeof(HTTPs_STR_HDR_FIELD_USER_AGENT)          - 1) },
    { HTTPs_HDR_FIELD_VARY,                 HTTPs_STR_HDR_FIELD_VARY,                 (sizeof(HTTPs_STR_HDR_FIELD_VARY)                - 1) },
    { HTTPs_HDR_FIELD_VIA,                  HTTPs_STR_HDR_FIELD_VIA,                  (sizeof(HTTPs_STR_HDR_FIELD_VIA)                 - 1) },
    { HTTPs_HDR_FIELD_WARNING,              HTTPs_STR_HDR_FIELD_WARNING,              (sizeof(HTTPs_STR_HDR_FIELD_WARNING)             - 1) },
    { HTTPs_HDR_FIELD_WWW_AUTHENTICATE,     HTTPs_STR_HDR_FIELD_WWW_AUTHENTICATE,     (sizeof(HTTPs_STR_HDR_FIELD_WWW_AUTHENTICATE)    - 1) },
#endif
};


/*
*********************************************************************************************************
*                                 HTTP HEADER FIELD CONNECTION VALUE
*********************************************************************************************************
*/

#define  HTTPs_STR_HDR_FIELD_CONN_CLOSE                     "Close"

static  const  HTTPs_DICTIONARY  HTTPs_DictionaryHdrFieldsConnValue[] = {
    { HTTPs_HDR_FIELD_CONN_CLOSE, HTTPs_STR_HDR_FIELD_CONN_CLOSE, (sizeof(HTTPs_STR_HDR_FIELD_CONN_CLOSE)   - 1) },
};


/*
*********************************************************************************************************
*                              HTTP HEADER FIELD TRANSFER ENCODING VALUE
*********************************************************************************************************
*/

typedef enum https_hdr_field_transfer_type {
    HTTPs_HDR_FIELD_TRANSFER_TYPE_CHUNCKED
} HTTPs_HDR_FIELD_TRANSFER_TYPE;

#define  HTTPs_STR_HDR_FIELD_CHUNKED                     "chunked"
#define  HTTPs_STR_BUF_TOP_SPACE_REQ_MIN                        6
#define  HTTPs_STR_BUF_END_SPACE_REQ_MIN                        2

static  const  HTTPs_DICTIONARY  HTTPs_DictionaryHdrFieldsTransferTypeValue[] = {
    { HTTPs_HDR_FIELD_TRANSFER_TYPE_CHUNCKED, HTTPs_STR_HDR_FIELD_CHUNKED, (sizeof(HTTPs_STR_HDR_FIELD_CHUNKED)   - 1) },
};


/*
*********************************************************************************************************
*                                  HTTP CGI MULTIPART CONTENT FIELD
*********************************************************************************************************
*/

typedef enum https_multipart_field {
    HTTPs_MULTIPART_FIELD_NAME,
    HTTPs_MULTIPART_FIELD_FILE_NAME,
    HTTPs_MULTIPART_FIELD_UNKNOWN
} HTTPs_MULTIPART_FIELD;

#define  HTTPs_STR_MULTIPART_FIELD_NAME                     "name"
#define  HTTPs_STR_MULTIPART_FIELD_FILE_NAME                "filename"

static  const  HTTPs_DICTIONARY  HTTPs_DictionaryMultipartFields[] = {
    { HTTPs_MULTIPART_FIELD_NAME,      HTTPs_STR_MULTIPART_FIELD_NAME,      (sizeof(HTTPs_STR_MULTIPART_FIELD_NAME)      - 1) },
    { HTTPs_MULTIPART_FIELD_FILE_NAME, HTTPs_STR_MULTIPART_FIELD_FILE_NAME, (sizeof(HTTPs_STR_MULTIPART_FIELD_FILE_NAME) - 1) },
};


/*
*********************************************************************************************************
*                                      HTTP INTERNAL TOKEN VALUE
*********************************************************************************************************
*/

typedef  enum  https_token_int {
    HTTPs_TOKEN_INTERNAL_STATUS_CODE,
    HTTPs_TOKEN_INTERNAL_REASON_PHRASE,
}HTTPs_TOKEN_INTERNAL;

#define  HTTPs_STR_TOKEN_INTERNAL_STATUS_CODE                "STATUS_CODE"
#define  HTTPs_STR_TOKEN_INTERNAL_REASON_PHRASE              "REASON_PHRASE"

static  const  HTTPs_DICTIONARY  HTTPs_DictionaryTokenInternal[] = {
    { HTTPs_TOKEN_INTERNAL_STATUS_CODE,   HTTPs_STR_TOKEN_INTERNAL_STATUS_CODE,   (sizeof(HTTPs_STR_TOKEN_INTERNAL_STATUS_CODE)   - 1) },
    { HTTPs_TOKEN_INTERNAL_REASON_PHRASE, HTTPs_STR_TOKEN_INTERNAL_REASON_PHRASE, (sizeof(HTTPs_STR_TOKEN_INTERNAL_REASON_PHRASE) - 1) },
};


/*
*********************************************************************************************************
*********************************************************************************************************
*                                               MACRO'S
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        HTTPs COUNTER MACRO'S
*
* Description : Functionality to set and increment statistic and error counter
*
* Argument(s) : Various HTTP server counter variable(s) & values.
*
* Return(s)   : none.
*
* Caller(s)   : various.
*
*               These macro's are INTERNAL HTTP server suite macro's & SHOULD NOT be called by
*               application function(s).
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (HTTPs_CFG_CTR_STAT_EN == DEF_ENABLED)
    #define  HTTPs_SET_PTR_STATS(p_ctr_stats, p_instance)   {                                           \
                                                                p_ctr_stats = &p_instance->StatsCtr;    \
                                                            }

    #define  HTTPs_STATS_INC(p_ctr)                         {                                           \
                                                                 p_ctr++;                               \
                                                            }

    #define  HTTPs_STATS_OCTET_INC(p_ctr, octet)            {                                           \
                                                                 p_ctr += octet;                        \
                                                            }

#else
                                                                /* Prevent 'variable unused' compiler warning.          */
    #define  HTTPs_SET_PTR_STATS(p_ctr_stats, p_instance)   {                                           \
                                                               (void)&p_ctr_stats;                      \
                                                            }

    #define  HTTPs_STATS_INC(p_ctr)

    #define  HTTPs_STATS_OCTET_INC(p_ctr, octet)
#endif



#if (HTTPs_CFG_CTR_ERR_EN == DEF_ENABLED)
    #define  HTTPs_SET_PTR_ERRS(p_ctr_err, p_instance)      {                                           \
                                                                 p_ctr_err = &p_instance->ErrsCtr;      \
                                                            }

    #define  HTTPs_ERR_INC(p_ctr)                           {                                           \
                                                                 p_ctr++;                               \
                                                            }

#else
                                                                 /* Prevent 'variable unused' compiler warning.          */
    #define  HTTPs_SET_PTR_ERRS(p_ctr_err, p_instance)      {                                           \
                                                               (void)&p_ctr_err;                        \
                                                            }

    #define  HTTPs_ERR_INC(p_ctr)
#endif



/*
*********************************************************************************************************
*********************************************************************************************************
*                                          FUNCTION PROTOTYPES
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                         APPLICATION PROGRAMING INTERFACE FUNCTION PROTOTYPES
*********************************************************************************************************
*/
HTTPs_ERR        HTTPs_Init        (void);

HTTPs_INSTANCE  *HTTPs_InstanceInit(const  HTTPs_CFG           *p_cfg,
                                    const  NET_FS_API          *p_fs_api,
                                           HTTPs_ERR           *p_err);

void            HTTPs_InstanceStart(       HTTPs_INSTANCE      *p_instance,
                                           HTTPs_ERR           *p_err);

void            HTTPs_InstanceStop (       HTTPs_INSTANCE      *p_instance,
                                           HTTPs_ERR           *p_err);


HTTPs_HDR_BLK  *HTTPs_RespHdrGet   (       HTTPs_INSTANCE      *p_instance,
                                           HTTPs_CONN          *p_conn,
                                           HTTPs_HDR_FIELD      hdr_field,
                                           HTTPs_HDR_VAL_TYPE   val_type,
                                           HTTPs_ERR           *p_err);


/*
*********************************************************************************************************
*                                     INTERNAL FUCNTION PROTOTYPE
*********************************************************************************************************
*/

void  HTTPs_InstanceTaskHandler(HTTPs_INSTANCE  *p_start_cfg);


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            CONFIGURATION
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                         CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef  HTTPs_CFG_ARG_CHK_EXT_EN
    #error  "HTTPs_CFG_ARG_CHK_EXT_EN                 not #define'd in 'http-s_cfg.h'"
    #error  "                             [MUST be  DEF_DISABLED]                    "
    #error  "                             [     ||  DEF_ENABLED ]                    "
#elif  ((HTTPs_CFG_ARG_CHK_EXT_EN != DEF_ENABLED ) && \
        (HTTPs_CFG_ARG_CHK_EXT_EN != DEF_DISABLED))
    #error  "HTTPs_CFG_ARG_CHK_EXT_EN           illegally #define'd in 'http-s_cfg.h'"
    #error  "                             [MUST be  DEF_DISABLED]                    "
    #error  "                             [     ||  DEF_ENABLED ]                    "
#endif


/*
*********************************************************************************************************
*                                    NETWORK CONFIGURATION ERRORS
*********************************************************************************************************
*/

#if     (NET_VERSION < 21302u)
#error  "NET_VERSION                       Invalid network protocol suite version    "
#error  "                                  [MUST be >= 2.13.02]                      "
#endif



#if     (NET_SOCK_CFG_SEL_EN != DEF_ENABLED)
#error  "NET_SOCK_CFG_SEL_EN               illegally #define'd in 'net_cfg.h'        "
#error  "                                  [MUST be  DEF_ENABLED]                    "
#endif


#if     (NET_TCP_NBR_CONN < (HTTPs_CFG_NBR_INSTANCE_MAX * 2))
#error  "NET_TCP_NBR_CONN              illegally #define'd in 'net_cfg.h'        "
#error  "                                  [MUST >= HTTPs_CFG_NBR_INSTANCE_MAX * 2]  "
#endif


/*
*********************************************************************************************************
*                                      LIB CONFIGURATION ERRORS
*********************************************************************************************************
*/

#if     (LIB_VERSION < 13700u)
#error  "LIB_VERSION                       Invalid library suite version"
#error  "                                  [MUST be >= 1.37.00]         "
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif                                                          /* End of HTTPs module include.                         */

