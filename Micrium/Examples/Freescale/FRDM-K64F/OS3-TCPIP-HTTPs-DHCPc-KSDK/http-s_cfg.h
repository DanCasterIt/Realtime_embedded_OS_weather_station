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
*                                   HTTP SERVER CONFIGURATION FILE
*
*                                              TEMPLATE
*
* Filename      : http-s_cfg.h
* Version       : V2.10.00
* Programmer(s) : AA
*                 MM
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                  HTTP ARGUMENT CHECK CONFIGURATION
*
* Note(s) : (1) Configure HTTPs_CFG_ARG_CHK_EXT_EN to enable/disable the HTTP server external argument
*               check feature :
*
*               (a) When ENABLED,  ALL arguments received from any port interface provided by the developer
*                   are checked/validated.
*
*               (b) When DISABLED, NO  arguments received from any port interface provided by the developer
*                   are checked/validated.
*
*           (2) Configure HTTPs_CFG_DBG_INFO_EN to enable/disable network protocol suite debug status
*               variables.
*********************************************************************************************************
*/

                                                                /* Configure external argument check feature ...        */
#define  HTTPs_CFG_ARG_CHK_EXT_EN                 DEF_ENABLED
                                                                /*   DEF_DISABLED     External argument check DISABLED  */
                                                                /*   DEF_ENABLED      External argument check ENABLED   */


                                                                /* Configure debug information feature (see Note #2) :  */
#define  HTTPs_CFG_DBG_INFO_EN                    DEF_ENABLED
                                                                /*   DEF_DISABLED  Debug information DISABLED           */
                                                                /*   DEF_ENABLED   Debug information ENABLED            */


/*
*********************************************************************************************************
*                                HTTP COUNTER MANAGEMENT CONFIGURATION
*
* Note(s) : (1) Configure HTTPs_CFG_CTR_STAT_EN to enable/disable HTTP server suite statistics counters.
*
*           (2) Configure HTTPs_CFG_CTR_ERR_EN  to enable/disable HTTP server suite error      counters.
*********************************************************************************************************
*/

                                                                /* Configure statistics counter feature (see Note #1) : */
#define  HTTPs_CFG_CTR_STAT_EN                    DEF_ENABLED
                                                                /*   DEF_DISABLED     Stat  counters DISABLED           */
                                                                /*   DEF_ENABLED      Stat  counters ENABLED            */

                                                                /* Configure error      counter feature (see Note #2) : */
#define  HTTPs_CFG_CTR_ERR_EN                     DEF_ENABLED
                                                                /*   DEF_DISABLED     Error counters DISABLED           */
                                                                /*   DEF_ENABLED      Error counters ENABLED            */

/*
*********************************************************************************************************
*                            HTTP DYNAMIC TOKEN REPLACEMENT CONFIGURATION
*
* Note(s) : (1) Configure HTTPs_CFG_TOKEN_PARSE_EN to enable/disable dynamic token replacement source code.
*********************************************************************************************************
*/

                                                                /* Configure Dynamic token replacement feature ...      */
                                                                /* ... (see Note #1):                                   */
#define  HTTPs_CFG_TOKEN_PARSE_EN                 DEF_ENABLED
                                                                /*   DEF_DISABLED   Dynamic token replacement DISABLED  */
                                                                /*   DEF_ENABLED    Dynamic token replacement ENABLED   */


/*
*********************************************************************************************************
*                                     HTTP CGI FORM CONFIGURATION
*
* Note(s) : (1) Configure HTTPs_CFG_CGI_EN             to enable/disable           Common Gateway Interface (CGI)
*                                                                                     form processing source code.
*
*           (2) Configure HTTPs_CFG_CGI_PEND_EN        to enable/disable           CGI pending until CGI data has
*                                                                                   been processed by the application.
*
*           (3) Configure HTTPs_CFG_CGI_MULTIPART_EN   to enable/disable multipart CGI form processing source code.
*
*           (4) Configure HTTPs_CFG_CGI_FILE_UPLOAD_EN to enable/disable file upload        processing source code.
*********************************************************************************************************
*/

                                                                /* Configure CGI processing feature (see Note #1):      */
#define  HTTPs_CFG_CGI_EN                         DEF_ENABLED
                                                                /*   DEF_DISABLED   CGI processing DISABLED             */
                                                                /*   DEF_ENABLED    CGI processing ENABLED              */

                                                                /* Configure CGI post pending feature (see Note #2):    */
#define  HTTPs_CFG_CGI_POLL_EN                    DEF_ENABLED
                                                                /*   DEF_DISABLED   CGI post pending DISABLED           */
                                                                /*   DEF_ENABLED    CGI post pending ENABLED            */

                                                                /* Configure Multipart CGI processing feature ...       */
                                                                /* ... (see Note #3):                                   */
#define  HTTPs_CFG_CGI_MULTIPART_EN               DEF_ENABLED
                                                                /*   DEF_DISABLED   Mutlipart CGI processing DISABLED   */
                                                                /*   DEF_ENABLED    Mutlipart CGI processing ENABLED    */

                                                                /* Configure File upload processing feature ...         */
                                                                /* ... (see Note #4):                                   */
#define  HTTPs_CFG_CGI_FILE_UPLOAD_EN             DEF_ENABLED
                                                                /*   DEF_DISABLED   File upload processing DISABLED     */
                                                                /*   DEF_ENABLED    File upload processing ENABLED      */


/*
*********************************************************************************************************
*                                      HTTP HEADER FIELD FEATURE
*
* Note(s) : (1) Configure HTTPs_CFG_HDR_EN to enable/disable header field processing for the message header of
*                                                                                        requests and responses.
*********************************************************************************************************
*/

                                                                /* Configure Header fields feature (see Note #1):       */
#define  HTTPs_CFG_HDR_EN                         DEF_ENABLED
                                                                /*   DEF_DISABLED   Header processing DISABLED          */
                                                                /*   DEF_ENABLED    Header processing ENABLED           */


/*
*********************************************************************************************************
*                                      HTTP PROXY CONFIGURATION
*
* Note(s) : (1) Configure HTTPs_CFG_ABSOLUTE_URI_EN to enable/disable support of absolute Uniform Resource
*                                                                                         Indentifier (URI).
*********************************************************************************************************
*/

                                                                /* Configure absolute URI support feature ...           */
                                                                /* ... (see Note #1):                                   */
#define  HTTPs_CFG_ABSOLUTE_URI_EN                DEF_ENABLED
                                                                /*   DEF_DISABLED   Absolute URI support DISABLED       */
                                                                /*   DEF_ENABLED    Absolute URI support ENABLED        */


/*
*********************************************************************************************************
*                           DEFAULT FATAL ERROR HTML DOCUMENT CONFIGURATION
*
* Note(s) : (1) Dynamic token replacement is also supported in default error HTML document.
*********************************************************************************************************
*/

                                                                /* Default HTML document returned error.                */
#define  HTTPs_CFG_HTML_DFLT_ERR_PAGE                                                                       \
                                                  "<HTML>\r\n"                                              \
                                                  "<BODY>\r\n"                                              \
                                                  "<HEAD><TITLE>SYSTEM ERROR</TITLE></HEAD>\r\n"            \
                                                  "<H1>#{STATUS_CODE}: #{REASON_PHRASE}</H1>\r\n"           \
                                                  "The operation cannot be completed.\r\n"                  \
                                                  "</BODY>\r\n"                                             \
                                                  "</HTML>\r\n"

