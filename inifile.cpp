#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "inifile.h"

/* DONT_HAVE_STRUPR is set when INI_REMOVE_CR is defined */

#ifdef DONT_HAVE_STRUPR
void strupr( char *str )
{
    // We dont check the ptr because the original also dont do it.
    while (*str != 0)
    {
        if ( islower( *str ) )
        {
		     *str = toupper( *str );
        }
        str++;
	}
}
#endif

TIniFile::TIniFile (void)
{
	m_pEntry      = NULL;
	m_pCurEntry   = NULL;
	m_result [0]  = 0;
	m_pIniFile    = NULL;
}

TIniFile::~TIniFile (void)
{
	// FreeAllMem ();
    Close();
}
CTuint TIniFile::GetVersion (void)
{
	return 0x0030;
}

CTbool TIniFile::Open(const char * FileName)
{
	char   Str [255];
	char   *pStr;
	struct IENTRY *pEntry;

	FreeAllMem ();

	if (FileName == NULL)                             { return CT_boolFALSE; }
	if( !strcmp( FileName, "" ) )				      { return CT_boolFALSE; }
	if ((m_pIniFile = fopen (FileName, "r")) == NULL) { return CT_boolFALSE; }

	while (fgets (Str, 255, m_pIniFile) != NULL)
	{
		pStr = strchr (Str, '\n');
		if (pStr != NULL) { *pStr = 0; }
		pEntry = MakeNewEntry ();
		if (pEntry == NULL) { return CT_boolFALSE; }

		#ifdef INI_REMOVE_CR
		int Len = strlen(Str);
		if ( Len > 0 )
		{
			if ( Str[Len-1] == '\r' )
			{
				Str[Len-1] = '\0';
			}
		}
		#endif

//		pEntry->pText = (char *)malloc (strlen (Str)+1);
		pEntry->pText = new char[strlen(Str)+1];
		if (pEntry->pText == NULL)
		{
			FreeAllMem ();
			return CT_boolFALSE;
		}
		strcpy (pEntry->pText, Str);
		pStr = strchr (Str,';');
		if (pStr != NULL) { *pStr = 0; } /* Cut all comments */
		pStr = strchr (Str,'#');
		if (pStr != NULL) { *pStr = 0; } /* Cut all comments */
		pStr = strchr (Str,'/');
		if (pStr != NULL && pStr[1] == '/' )
			{ *pStr = 0; } /* Cut all comments */

		if ( (strstr (Str, "[") > 0) && (strstr (Str, "]") > 0) ) /* Is Section */
		{
			pEntry->Type = tpSECTION;

			pStr = strchr (Str, ']');  //zsw add 
			if( pStr != NULL ) *pStr = 0 ;
			pStr = strchr( Str, '[' );
			if( pStr ) strcpy( pEntry->pText, pStr+1);
			alltrim(pEntry->pText);
		}
		else
		{
			if (strstr (Str, "=") > 0)
			{
				pEntry->Type = tpKEYVALUE;
			}
			else
			{
				pEntry->Type = tpCOMMENT;
			}
		}
		m_pCurEntry = pEntry;
	}
	fclose (m_pIniFile);
	m_pIniFile = NULL;
	return CT_boolTRUE;
}

void TIniFile::Close(void)
{
	FreeAllMem ();
	if (m_pIniFile != NULL)
	{
		fclose (m_pIniFile);
		m_pIniFile = NULL;
	}
}


CTbool TIniFile::WriteIniFile (const char *pFileName)
{
	struct IENTRY *pEntry = m_pEntry;
	if (m_pIniFile != NULL)
	{
		fclose (m_pIniFile);
	}
	if ((m_pIniFile = fopen (pFileName, "wb")) == NULL)
	{
		FreeAllMem ();
		return CT_boolFALSE;
	}

	while (pEntry != NULL)
	{
		if (pEntry->Type != tpNULL)
		{
#ifdef INI_REMOVE_CR
            /* ?? */
            if ( pEntry->Type == tpSECTION )
            {
                fprintf (m_pIniFile, "[%s]\n", pEntry->pText );
            }
            else
            {
			    fprintf (m_pIniFile, "%s\n", pEntry->pText);
            };
#else
			
            if ( pEntry->Type == tpSECTION )
            {
                fprintf (m_pIniFile, "[%s]\r\n", pEntry->pText );
            }
            else
            {
			    fprintf (m_pIniFile, "%s\r\n", pEntry->pText);
            };
#endif
		}
		pEntry = pEntry->pNext;
	}

	fclose (m_pIniFile);
	m_pIniFile = NULL;
	return CT_boolTRUE;
}

void TIniFile::WriteString (const char *pSection, const char *pKey, const char *pValue)
{
	EFIND List;
	char  Str [255];

	if (ArePtrValid (pSection, pKey, pValue) == CT_boolFALSE) { return; }
	if (FindKey  (pSection, pKey, &List) == CT_boolTRUE)
	{
		sprintf (Str, "%s=%s%s", List.KeyText, pValue, List.Comment);
		FreeMem (List.pKey->pText);
//		List.pKey->pText = (char *)malloc (strlen (Str)+1);
		List.pKey->pText = new char[strlen (Str)+1];
		strcpy (List.pKey->pText, Str);
	}
	else
	{
		if ((List.pSec != NULL) && (List.pKey == NULL)) /* section exist, Key not */
		{
			AddKey (List.pSec, pKey, pValue);
		}
		else
		{
			AddSectionAndKey (pSection, pKey, pValue);
		}
	}
}

void TIniFile::WriteBool (const char *pSection, const char *pKey, CTbool Value)
{
	char Val [2] = {'0',0};
	if (Value != 0) { Val [0] = '1'; }
	WriteString(pSection, pKey, Val);
}
void TIniFile::WriteInt (const char *pSection, const char *pKey, int Value)
{
	char Val [12]; /* 32bit maximum + sign + \0 */
	sprintf (Val, "%d", Value);
	WriteString(pSection, pKey, Val);
}

void TIniFile::WriteDouble (const char *pSection, const char *pKey, double Value)
{
	char Val [32]; /* DDDDDDDDDDDDDDD+E308\0 */
	sprintf (Val, "%1.10lE", Value);
	WriteString(pSection, pKey, Val);
}

const char *TIniFile::ReadString (const char *pSection, const char *pKey, const char *pDefault)
{
	EFIND List;
	if (ArePtrValid (pSection, pKey, pDefault) == CT_boolFALSE) { return pDefault; }

	if (FindKey  (pSection, pKey, &List) == CT_boolTRUE)
	{
		strcpy (m_result, List.ValText);
		if ( *m_result != '\0')
			return m_result;
	}
	return pDefault;
}

CTbool TIniFile::ReadBool (const char *pSection, const char *pKey, CTbool Default)
{
	char Val [2] = {"0"};
	if (Default != 0) { Val [0] = '1'; }
	return (atoi (ReadString (pSection, pKey, Val))?1:0); /* Only allow 0 or 1 */
}

int TIniFile::ReadInt (const char *pSection, const char *pKey, int Default)
{
	char Val [12];
	sprintf (Val,"%d", Default);
       int     ret(0);
      char    *val_str    = (char*)ReadString(pSection, pKey, Val);
      if ( strlen(val_str) > 2 && val_str[0] == '0' && val_str[1] == 'x' )
     {
        //strupr( (char*)(val_str + 2) );
        sscanf(val_str, "0x%x", &ret );
     }
     else
    {
        ret = atoi( val_str );
    };

    return ret;
}

double TIniFile::ReadDouble (const char *pSection, const char *pKey, double Default)
{
	double Val;
	sprintf (m_result, "%1.10lE", Default);
	sscanf (ReadString (pSection, pKey, m_result), "%lE", &Val);
	return Val;
}
CTbool TIniFile::DeleteKey (const char *pSection, const char *pKey)
{
	EFIND         List;
	struct IENTRY *pPrev;
	struct IENTRY *pNext;

	if (FindKey (pSection, pKey, &List) == CT_boolTRUE)
	{
		pPrev = List.pKey->pPrev;
		pNext = List.pKey->pNext;
		if (pPrev)
		{
			pPrev->pNext=pNext;
		}
		if (pNext)
		{ 
			pNext->pPrev=pPrev;
		}
		FreeMem (List.pKey->pText);
		FreeMem (List.pKey);
		return CT_boolTRUE;
	}
	return CT_boolFALSE;
}


void TIniFile::FreeMem (void *pPtr)
{
	if (pPtr != NULL) { delete (pPtr); }
}

void TIniFile::FreeAllMem (void)
{
	struct IENTRY *pEntry;
	struct IENTRY *pNextEntry;
	pEntry = m_pEntry;
	while (1)
	{
		if (pEntry == NULL) { break; }
		pNextEntry = pEntry->pNext;
		FreeMem (pEntry->pText); /* Frees the pointer if not NULL */
		FreeMem (pEntry);
		pEntry = pNextEntry;
	}
	m_pEntry    = NULL;
	m_pCurEntry = NULL;
}

struct IENTRY *TIniFile::FindSection (const char *pSection)
{
	char Sec  [130];
	char iSec [130];
	struct IENTRY *pEntry;
	strcpy( Sec, pSection); 
	strupr  (Sec);
	pEntry = m_pEntry; 
	while (pEntry != NULL)
	{
		if (pEntry->Type == tpSECTION)
		{
			strcpy  (iSec, pEntry->pText);
			strupr  (iSec);
			if (strcmp (Sec, iSec) == 0)
			{
				return pEntry;
			}
		}
		pEntry = pEntry->pNext;
	}
	return NULL;
}


CTbool	TIniFile::DeleteKeyOfSection (const char *pSection)
{
	char Sec  [130];
	char iSec [130];
	struct IENTRY *pEntry;
	strcpy( Sec, pSection); 
	strupr  (Sec);
	pEntry = m_pEntry; /* Get a pointer to the first Entry */
	while (pEntry != NULL)
	{				
		if (pEntry->Type == tpSECTION)//Found the section,break
		{
			strcpy  (iSec, pEntry->pText);
			strupr  (iSec);
			if (strcmp (Sec, iSec) == 0)
			{
				break;
			}
		}
		pEntry = pEntry->pNext;
	}	
	if(pEntry!=NULL)//If the section existing
	{
		while((pEntry->pNext)!=NULL)//Delete the keyvalue of the section
		{
			if (((pEntry->pNext->Type) == tpSECTION) || 
					((pEntry->pNext->Type) == tpNULL   ))
			{
				return CT_boolFALSE;
			}
			else
			{			
				struct IENTRY *temp;
				temp=pEntry->pNext;			
				pEntry->pNext=temp->pNext;
				FreeMem (temp->pText);
				FreeMem (temp);	
			}
		}
	}
	return CT_boolFALSE;	
}

#define Min
CTbool TIniFile::FindKey  (const char *pSection, const char *pKey, EFIND *pList)
{
	char Search [130];
	char Found  [130];
	char Text   [255];
	char *pText, *pText2;
	struct IENTRY *pEntry;
	pList->pSec        = NULL;
	pList->pKey        = NULL;
	pEntry = FindSection (pSection);
	if (pEntry == NULL) { return CT_boolFALSE; }
	pList->pSec        = pEntry;
	pList->KeyText[0] = 0;
	pList->ValText[0] = 0;
	pList->Comment[0] = 0;
	pEntry = pEntry->pNext;
	if (pEntry == NULL) { return CT_boolFALSE; }
	sprintf (Search, "%s",pKey);
	strupr  (Search);
	while (pEntry != NULL)
	{
		if ((pEntry->Type == tpSECTION) || /* Stop after next section or EOF */
			(pEntry->Type == tpNULL   ))
		{
			return CT_boolFALSE;
		}
		if (pEntry->Type == tpKEYVALUE)
		{
			strcpy (Text, pEntry->pText);
			pText = strchr (Text, ';');
			if (pText == NULL)
				pText = strchr (Text, '#');
			else
			{
				pText2 = strchr (Text, '#');
				if (pText2 != NULL)
					pText = Min(pText, pText2);
			}
				
			if (pText != NULL)
			{
				strcpy (pList->Comment, pText);
				*pText = 0;
			}

			pText = strchr (Text, '=');
			if (pText != NULL)
			{
				*pText = 0;
				
				strcpy (pList->KeyText, Text);

				strcpy (Found, Text);
				alltrim(Found);  //zsw add
				*pText = '=';
				strupr (Found);
	
				if (strcmp (Found,Search) == 0)
				{
				   strcpy (pList->ValText, pText+1);
				   alltrim(pList->ValText) ;//zsw must add 
				   pList->pKey = pEntry;
				   return CT_boolTRUE;
				}
			}
		}
		pEntry = pEntry->pNext;
	}
	return CT_boolFALSE;
}

CTbool TIniFile::AddItem (char Type, const char *pText)
{
	struct IENTRY *pEntry = MakeNewEntry ();
	if (pEntry == NULL) { return CT_boolFALSE; }
	pEntry->Type = Type;
//	pEntry->pText = (char*)malloc (strlen (pText) +1);
	pEntry->pText = new char[strlen (pText) +1];
	if (pEntry->pText == NULL)
	{
//		free (pEntry);
		delete (pEntry);
		return CT_boolFALSE;
	}
	strcpy (pEntry->pText, pText);
	pEntry->pNext   = NULL;
	if (m_pCurEntry != NULL) { m_pCurEntry->pNext = pEntry; }
	m_pCurEntry    = pEntry;
	return CT_boolTRUE;
}

CTbool TIniFile::AddItemAt (struct IENTRY *pEntryAt, char Mode, const char *pText)
{
	struct IENTRY *pNewEntry;
	if (pEntryAt == NULL)  { return CT_boolFALSE; }
	pNewEntry = new IENTRY ;
	if (pNewEntry == NULL) { return CT_boolFALSE; }
	pNewEntry->pText = new char[strlen (pText)+1];
	if (pNewEntry->pText == NULL)
	{
		delete (pNewEntry);
		return CT_boolFALSE;
	}
	strcpy (pNewEntry->pText, pText);
	if (pEntryAt->pNext == NULL) /* No following nodes. */
	{
		pEntryAt->pNext   = pNewEntry;
		pNewEntry->pNext  = NULL;
	}
	else
	{
		pNewEntry->pNext = pEntryAt->pNext;
		pEntryAt->pNext  = pNewEntry;
	}
	pNewEntry->pPrev = pEntryAt;
	pNewEntry->Type  = Mode;
	return CT_boolTRUE;
}

CTbool TIniFile::AddSectionAndKey (const char *pSection, const char *pKey, const char *pValue)
{
	char Text [255];
        sprintf (Text, "%s", pSection);
	if (AddItem (tpSECTION, Text) == CT_boolFALSE) { return CT_boolFALSE; }
	sprintf (Text, "%s = %s", pKey, pValue);
	return AddItem (tpKEYVALUE, Text)? 1 : 0;
}

void TIniFile::AddKey (struct IENTRY *pSecEntry, const char *pKey, const char *pValue)
{
	char Text [255];
	sprintf (Text, "%s=%s", pKey, pValue);
	AddItemAt (pSecEntry, tpKEYVALUE, Text);
}

struct IENTRY *TIniFile::MakeNewEntry (void)
{
	struct IENTRY *pEntry;
	pEntry = new IENTRY ;
	if (pEntry == NULL)
	{
		FreeAllMem ();
		return NULL;
	}
	if (m_pEntry == NULL)
	{
		m_pEntry = pEntry;
	}
	pEntry->Type  = tpNULL;
	pEntry->pPrev = m_pCurEntry;
	pEntry->pNext = NULL;
	pEntry->pText = NULL;
	if (m_pCurEntry != NULL)
	{
		m_pCurEntry->pNext = pEntry;
	}
	return pEntry;
}

void TIniFile::alltrim(char *string) 
{
	if( !string || !string[0])
		return ;

	char *p =   string + strlen(string) -1 ;
    /*  trim tail.  */
	for(; p >= string && (*p == ' ' || *p == '\t') ; p-- )  // add trim of TAB.
		*p = 0 ;
    /*  trim head.  */
	for( p = string; *p == ' ' || *p == '\t' ; p++ );
	for(; *p; p++ )
		*string++ = *p ;
	*string = 0 ;
}

CTbool TIniFile::AddSectionEx(const char * pSection,const char *lpString)
{
	char Text [255];
	struct IENTRY *pEntry;
	pEntry = FindSection (pSection);//First found the section
	if (pEntry == NULL) //If the section did not exist,add new section and keyvalue
	{
		sprintf (Text, "%s", pSection);
		if (AddItem (tpSECTION, Text) == CT_boolFALSE) 
		{ 
			return CT_boolFALSE; 
		}
		sprintf (Text, "%s", lpString);
		if (AddItem (tpKEYVALUE, Text) == CT_boolFALSE) 
		{ 
			return CT_boolFALSE; 
		}
	}	
	else //If the section already exist,than replace the key of section with lpString
	{
		DeleteKeyOfSection (pSection);
		sprintf (Text, "%s", lpString);
		AddItemAt (pEntry,tpKEYVALUE, Text);
	}
	return 	CT_boolTRUE;
}

CTbool GetPrivateProfileBoolEx(const char *pSection, const char *pKey, CTbool   Default, const char *pFileName)
{
	TIniFile ini ;
	ini.Open(pFileName);
	CTbool ret = ini.ReadBool( pSection, pKey, Default);
	ini.Close();
	return ret;
}

int GetPrivateProfileIntEx(const char *pSection, const char *pKey, int    Default, const char *pFileName)
{
	TIniFile ini ;
	ini.Open(pFileName);
	int ret = ini.ReadInt( pSection, pKey, Default);
	ini.Close();
	return ret;
}

double GetPrivateProfileDoubleEx  (const char *pSection, const char *pKey, double Default, const char *pFileName)
{
	TIniFile ini ;
	ini.Open(pFileName);
	double ret = ini.ReadDouble( pSection, pKey, Default);
	ini.Close();
	return ret;
}

char    *GetPrivateProfileStringEx (const char *pSection, const char *pKey, const char  *pDefault, char *pReturnedString,  int nSize, const char *pFileName)
{
	TIniFile ini ;
	ini.Open(pFileName);
	if (pDefault == NULL)
		strncpy( pReturnedString, ini.ReadString( pSection, pKey, ""), nSize) ;
	else
		strncpy( pReturnedString, ini.ReadString( pSection, pKey, pDefault), nSize) ;

	ini.Close();
	return pReturnedString ;
}

void    WritePrivateProfileBoolEx   (const char *pSection, const char *pKey, CTbool   Value, const char *pFileName)
{
	TIniFile ini ;
	ini.Open(pFileName);
	ini.WriteBool( pSection, pKey, Value);
	ini.WriteIniFile (pFileName);
	ini.Close();
}

void    WritePrivateProfileIntEx    (const char *pSection, const char *pKey, int    Value, const char *pFileName)
{
	TIniFile ini ;
	ini.Open(pFileName);
	ini.WriteInt( pSection, pKey, Value);
	ini.WriteIniFile (pFileName);
	ini.Close();
}

void    WritePrivateProfileDoubleEx (const char *pSection, const char *pKey, double Value, const char *pFileName)
{
	TIniFile ini ;
	ini.Open(pFileName);
	ini.WriteDouble( pSection, pKey, Value);
	ini.WriteIniFile (pFileName);
	ini.Close();
}

void    WritePrivateProfileStringEx (const char *pSection, const char *pKey, const char  *pValue, const char *pFileName)
{
	TIniFile ini ;
	ini.Open(pFileName);
	ini.WriteString( pSection, pKey, pValue);
	ini.WriteIniFile (pFileName);
	ini.Close();
}


unsigned long GetPrivateProfileSectionNamesEx(char * strReturn,
			unsigned long nSize,const char * lpFileName)
{
	char   Str [255];
	//char *tempstr=(char *)malloc(nSize);
	char tempstr[255];
	FILE *m_pIniFile;
	char   *pStr;
	unsigned long reResult=0;
	if( !strcmp( lpFileName, "" ) )
	{ 
		return CT_boolFALSE; 
	}
	if ((m_pIniFile = fopen (lpFileName, "r")) == NULL) 
	{
		 return CT_boolFALSE; 
	}
	
	while (fgets (Str, 255, m_pIniFile) != NULL)
	{
		pStr = strchr (Str, '\n');
		if (pStr != NULL) { *pStr = 0; }
		
		pStr = strchr (Str,';');
		if (pStr != NULL) { *pStr = 0; } /* Cut all comments */
		pStr = strchr (Str,'#');
		if (pStr != NULL) { *pStr = 0; } /* Cut all comments */
		pStr = strchr (Str,'/');
		if (pStr != NULL && pStr[1] == '/' )
			{ *pStr = 0; } /* Cut all comments */
		if ( (strstr (Str, "[") > 0) && (strstr (Str, "]") > 0) ) /* Is Section */
		{
			pStr = strchr (Str, ']');  
			if( pStr != NULL ) *pStr = 0 ;
			pStr = strchr( Str, '[' );
			if( pStr ) 
			{
				strcpy( tempstr, pStr+1);
				//alltrim(tempstr);
				for(int i=0;tempstr[i]!=NULL;i++)
				{
					*strReturn++=tempstr[i];
					reResult++;	
				}
				*strReturn++='\0';
			}			
		}		
	}
	*strReturn++='\0';
	if(reResult>=nSize)
	{
		reResult=nSize;
	}
	fclose (m_pIniFile);
	m_pIniFile = NULL;
	//free(tempstr);
	return reResult;
}


int WritePrivateProfileSectionEx(const char * pSection,
				const char *lpString,const char * lpFileName)
{
	TIniFile ini ;
	ini.Open(lpFileName);
	ini.AddSectionEx(pSection,lpString);
	ini.WriteIniFile (lpFileName);
	ini.Close();
	return CT_boolTRUE;
}

