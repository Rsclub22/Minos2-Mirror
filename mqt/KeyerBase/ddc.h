/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef ddcH
#define ddcH 
/*============================================================================
 
	 ddc.h  -  Don Cross,  October 1992.
 
	 Generic ddclib stuff.
 
============================================================================*/


enum DDCRET
{
   DDC_SUCCESS,               // The operation succeded
   DDC_FAILURE,               // The operation failed for unspecified reasons
   DDC_OUT_OF_MEMORY,         // Operation failed due to running out of memory
   DDC_FILE_ERROR,            // Operation encountered file I/O error
   DDC_INVALID_CALL,          // Operation was called with invalid parameters
   DDC_USER_ABORT,            // Operation was aborted by the user
   DDC_INVALID_FILE       // File format does not match
};

#endif /* __DDC_DDC_H */

/*--- end of file ddc.h ---*/
