/**************************************************************************
* Copyright (c) 2004,ShenXun Information Technology
* All rights reserved.
* File name:	iniFile.h     
* Author: 	yueyuanchong	
* Version: 	1.0  
* Date:		2004.02.24  
* Description:	
* History:	
*	   Date:
*	   Author:
*	   Modification:
******************************************************************************/

#ifndef __INIFILE__H
#define __INIFILE__H

#include "stdio.h"  
#include "typedef.h"
#ifndef WIN32 
#define INI_REMOVE_CR
#define DONT_HAVE_STRUPR
#endif

#define tpNULL       0
#define tpSECTION    1
#define tpKEYVALUE   2
#define tpCOMMENT    3



struct IENTRY
{
   char   Type;
   char  *pText;
   struct IENTRY *pPrev;
   struct IENTRY *pNext;
};

typedef struct
{
   struct IENTRY *pSec;
   struct IENTRY *pKey;
   char          KeyText [128];
   char          ValText [128];
   char          Comment [255];
} EFIND;

/* Macros */
#define ArePtrValid(Sec,Key,Val) ((Sec!=NULL)&&(Key!=NULL)&&(Val!=NULL))

#ifdef _WINDLL
class __declspec(dllexport) TIniFile
#else
class TIniFile
#endif
{
public:
            TIniFile    (void);
            ~TIniFile   (void);
    CTuint  GetVersion  (void);
    CTbool  Open(const char *pFileName);
    CTbool  ReadBool    (const char *pSection, const char *pKey, CTbool   Default);
    int     ReadInt     (const char *pSection, const char *pKey, int    Default);
    double  ReadDouble  (const char *pSection, const char *pKey, double Default);
    const char    *ReadString (const char *pSection, const char *pKey, const char  *pDefault);

    void    WriteBool   (const char *pSection, const char *pKey, CTbool   Value);
    void    WriteInt    (const char *pSection, const char *pKey, int    Value);
    void    WriteDouble (const char *pSection, const char *pKey, double Value);
    void    WriteString (const char *pSection, const char *pKey, const char  *pValue);

    void    Close();
    CTbool  WriteIniFile (const char *pFileName);
    CTbool	DeleteKey (const char *pSection, const char *pKey);
    CTbool	DeleteKeyOfSection (const char *pSection);//Add by c.s.wang
    CTbool	AddSectionEx(const char * pSection,const char *lpString); //Add by c.s.wang

protected:
	struct  IENTRY *m_pEntry;
	struct  IENTRY *m_pCurEntry;
	char    m_result [255];
	FILE    *m_pIniFile;
	void    AddKey     (struct IENTRY *pEntry, const char *pKey, const char *pValue);
	CTbool  AddItem    (char Type, const char *pText);
	CTbool  AddItemAt (struct IENTRY *pEntryAt, char Mode, const char *pText);
	void    FreeMem    (void *pPtr);
	void    FreeAllMem (void);
	CTbool  FindKey    (const char *pSection, const char *pKey, EFIND *pList);
	CTbool  AddSectionAndKey (const char *pSection, const char*pKey, const char*pValue);
	struct  IENTRY *MakeNewEntry (void);
	struct  IENTRY *FindSection (const char *pSection);

private:
	void    alltrim(char *string); 

};

////////////////////////////////////////////////////////////////////////

#ifdef _WINDLL
__declspec(dllexport) CTbool    GetPrivateProfileBoolEx    (const char *pSection, const char *pKey, CTbool   Default, const char *pFileName);
__declspec(dllexport) int       GetPrivateProfileIntEx     (const char *pSection, const char *pKey, int    Default, const char *pFileName);
__declspec(dllexport) double    GetPrivateProfileDoubleEx  (const char *pSection, const char *pKey, double Default, const char *pFileName);
__declspec(dllexport)  char *   GetPrivateProfileStringEx (const char *pSection, const char *pKey, const char  *pDefault, char *pReturnedString,  int nSize, const char *pFileName);
__declspec(dllexport) void    WritePrivateProfileBoolEx   (const char *pSection, const char *pKey, CTbool   Value, const char *pFileName);
__declspec(dllexport) void     WritePrivateProfileIntEx    (const char *pSection, const char *pKey, int    Value, const char *pFileName);
__declspec(dllexport) void  WritePrivateProfileDoubleEx (const char *pSection, const char *pKey, double Value, const char *pFileName);
__declspec(dllexport) void    WritePrivateProfileStringEx (const char *pSection, const char *pKey, const char  *pValue, const char *pFileName);
__declspec(dllexport) unsigned long  GetPrivateProfileSectionNamesEx(char * strReturn,
				unsigned long nSize,const char * lpFileName);
__declspec(dllexport) int  WritePrivateProfileSectionEx(const char * pSection,	const char *lpString,const char * lpFileName);//Add by c.s.wang
#else
CTbool  GetPrivateProfileBoolEx    (const char *pSection, const char *pKey, CTbool   Default, const char *pFileName);
int     GetPrivateProfileIntEx     (const char *pSection, const char *pKey, int    Default, const char *pFileName);
double  GetPrivateProfileDoubleEx  (const char *pSection, const char *pKey, double Default, const char *pFileName);
char    *GetPrivateProfileStringEx (const char *pSection, const char *pKey, const char  *pDefault, char *pReturnedString,  int nSize, const char *pFileName);
void    WritePrivateProfileBoolEx   (const char *pSection, const char *pKey, CTbool   Value, const char *pFileName);
void    WritePrivateProfileIntEx    (const char *pSection, const char *pKey, int    Value, const char *pFileName);
void    WritePrivateProfileDoubleEx (const char *pSection, const char *pKey, double Value, const char *pFileName);
void    WritePrivateProfileStringEx (const char *pSection, const char *pKey, const char  *pValue, const char *pFileName);
unsigned long GetPrivateProfileSectionNamesEx(char * strReturn,
				unsigned long nSize,const char * lpFileName);
int WritePrivateProfileSectionEx(const char * pSection,	const char *lpString,const char * lpFileName);//Add by c.s.wang
#endif


#endif


