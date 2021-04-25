
// stdafx.cpp : source file that includes just the standard includes
// InSpector.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

char * string2char(CString str)
{
	wchar_t *w;
	char *s = NULL;
	int length = 0;
	w = str.GetBuffer(str.GetLength());   
	if( w ) {
		length = WideCharToMultiByte(CP_ACP, 0, w, -1, NULL, 0, NULL, NULL); 
		if( length > 0 ) {
			s = (char *)malloc(sizeof(char)*length);
			if( s != NULL ) {
				WideCharToMultiByte(CP_ACP, 0, w, -1, s, length, 0,0);	 
			}
		}
	}
	return s;
}


