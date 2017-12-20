/**************************************************************************
* Copyright (c) 2004,ShenXun Information Technology
* All rights reserved.
* File name:	typedef.h     
* Author: 	yueyuanchong	
* Version: 	1.0  
* Date:		2004.02.24  
* Description:	
* History:	
*	   Date:
*	   Author:
*	   Modification:
******************************************************************************/

#ifndef  __TYPEDEF__H
#define  __TYPEDEF__H

#include <time.h>

typedef  unsigned long        CTuint ;
typedef  unsigned char        CTbyte;
typedef  short  int               CTshort;
typedef  unsigned short int  CTushort;
typedef  signed long            CTint;
typedef  float                      CTfloat;
typedef  time_t                   CTtime;
typedef  double                  CTdouble;

typedef  unsigned long         CTstatus;
typedef  unsigned long         CTerror;
typedef  unsigned long        CTsymbol;
typedef  unsigned long        CTobj;
typedef  signed    long 		CTbool;

#define  CT_boolTRUE           1
#define  CT_boolFALSE         0



#define  IVR_Module     1
#define  CCS_Module     2
#define  CTI_Module     3
#define  CSTA_Module    4

// KV Set value type definitions
#define VALTYP_SCALAR_BASE        10
#define VALTYP_NONSCALAR_BASE     40
#define VALTYP_ARRAY_BASE         60

#define CTkvs_valtypNULL         VALTYP_SCALAR_BASE + 0
#define CTkvs_valtypBYTE         VALTYP_SCALAR_BASE + 1
#define CTkvs_valtypSHORT      VALTYP_SCALAR_BASE + 2
#define CTkvs_valtypINT           VALTYP_SCALAR_BASE + 3
#define CTkvs_valtypUINT         VALTYP_SCALAR_BASE + 4
#define CTkvs_valtypBOOL         VALTYP_SCALAR_BASE + 5
#define CTkvs_valtypFLOAT        VALTYP_SCALAR_BASE + 6
#define CTkvs_valtypERROR        VALTYP_SCALAR_BASE + 7
#define CTkvs_valtypSTATUS       VALTYP_SCALAR_BASE + 8
#define CTkvs_valtypSYMBOL       VALTYP_SCALAR_BASE + 9
#define CTkvs_valtypTIME           VALTYP_SCALAR_BASE + 10
#define CTkvs_valtypUSHORT       VALTYP_SCALAR_BASE + 11 

#define CTkvs_valtypIRANGE        VALTYP_NONSCALAR_BASE + 0
#define CTkvs_valtypFRANGE        VALTYP_NONSCALAR_BASE + 1
#define CTkvs_valtypSTRING         VALTYP_NONSCALAR_BASE + 2

#define CTkvs_valtypNULLARRAY       VALTYP_ARRAY_BASE + 0
#define CTkvs_valtypBYTEARRAY       VALTYP_ARRAY_BASE + 1
#define CTkvs_valtypSHORTARRAY    VALTYP_ARRAY_BASE + 2
#define CTkvs_valtypINTARRAY         VALTYP_ARRAY_BASE + 3
#define CTkvs_valtypUINTARRAY        VALTYP_ARRAY_BASE + 4
#define CTkvs_valtypBOOLARRAY        VALTYP_ARRAY_BASE + 5
#define CTkvs_valtypSYMBOLARRAY     VALTYP_ARRAY_BASE + 6
#define CTkvs_valtypFLOATARRAY      VALTYP_ARRAY_BASE + 7
#define CTkvs_valtypSTRINGARRAY     VALTYP_ARRAY_BASE + 8
#define CTkvs_valtypARRAYARRAY      VALTYP_ARRAY_BASE + 9
#define CTkvs_valtypERRORARRAY      VALTYP_ARRAY_BASE + 10
#define CTkvs_valtypSTATUSARRAY     VALTYP_ARRAY_BASE + 11
#define CTkvs_valtypFRANGEARRAY     VALTYP_ARRAY_BASE + 12
#define CTkvs_valtypIRANGEARRAY     VALTYP_ARRAY_BASE + 13
#define CTkvs_valtypUSHORTARRAY     VALTYP_ARRAY_BASE + 14


#define _delete(p) {delete p; p=0;}









#endif
