/***************************************************************************
 *                                                                         *
 *  Copyright (C) 1997-1999 Drago Entertainment Corp. All rights reserved  *
 *                                                                         * 
 *  File:       JERROR.H                                                   *
 *  Purpose:    Header file for error handling                             *
 *                                                                         * 
 *  Project:    ACTIVATION!                                                *
 *                                                                         *
 *  Author:     Jan Jasinski                                               *
 *  Modified:   99.09.23                                                   *
 *  Notes:                                                                 *
 *                                                                         * 
 ***************************************************************************/

/*—————————————————————————————————————————————————————————————————————————*
 * Preprocessor                                                            *
 *—————————————————————————————————————————————————————————————————————————*/
#ifndef __JERROR_H__
#define __JERROR_H__

/*—————————————————————————————————————————————————————————————————————————*
 * Defines                                                                 *
 *—————————————————————————————————————————————————————————————————————————*/
#define CHECK_DX(x) {if(FAILED(hResult=(x))){ DUMP_F("%s in %s: %d",GetDirectXStatusString(hResult),__FILE__,__LINE__);} }

/*—————————————————————————————————————————————————————————————————————————*
 *                                                                         *
 * Structures                                                              *      
 *                                                                         *
 *—————————————————————————————————————————————————————————————————————————*/
/*—————————————————————————————————————————————————————————————————————————*
 * Struct ERRLIST                                                          *
 *—————————————————————————————————————————————————————————————————————————*/
typedef struct tag_ERRLIST
{
    HRESULT hResult;
    LPSTR   lpszError;
} ERRLIST;
typedef ERRLIST*    LPERRLIST;

/*—————————————————————————————————————————————————————————————————————————*
 *                                                                         *
 *  Function Prototypes                                                    *
 *                                                                         *
 *—————————————————————————————————————————————————————————————————————————*/
int   OpenDebugLogFile(LPSTR lpszFileName);
int   WriteDebugLogFile(LPSTR lpszComment);
void  CloseDebugLogFile( void );
void  DUMP_O( char *fmt, ... ); // Dump to output device
void  DUMP_F( char *fmt, ... ); // Dump to file
void  TRY_DX( HRESULT hResult );
LPSTR GetDirectXStatusString(HRESULT DDStatus);

/*—————————————————————————————————————————————————————————————————————————*
 * Globals                                                                 *
 *—————————————————————————————————————————————————————————————————————————*/
extern BOOL g_bLogFile;

/*—————————————————————————————————————————————————————————————————————————*
 * End of code                                                             *
 *—————————————————————————————————————————————————————————————————————————*/
#endif//__JERROR_H__
/***************************************************************************
 * End of file JDX_INIT.H                                                  * 
 ***************************************************************************/



