/******************************************************************************
 **
 ** Filename    : commom.h
 ** Abstract    :
 ** Settings    :
 ** Contents    :
 **     Public:
 **     Private:
 ** Author      : bruno
 ** Http        :
 ** Mail        :
 ** Create on   : 16 de Outubro de 2009, 10:01
 **
 ******************************************************************************/
#ifndef _COMMOM_H_
#define _COMMOM_H_

#ifdef	__cplusplus
extern "C" {
#endif
/******************************************************************************
 **
 ** 1   INCLUDE FILES
 ** 1.1 Standart include files
 **
 ******************************************************************************/
/******************************************************************************
 ** 1.2 Application include files
 ******************************************************************************/
/******************************************************************************
 **
 ** 2   DECLARATIONS
 ** 2.1 Global constants
 **
 ******************************************************************************/
#define COMMOM_DELAY_1MS    (1 / portTICK_RATE_MS)
#define COMMOM_DELAY_15MS   (15 / portTICK_RATE_MS)

#define PACKED __attribute__((packed))
/******************************************************************************
 ** 2.2 Global type definitions
 ******************************************************************************/
typedef char BYTE; // 1-byte definition
typedef short WORD; // 2-bytes definition
typedef int DWORD; // 4-bytes definition
typedef long QWORD; // 8-bytes definition

typedef unsigned char   u08;
typedef unsigned short  u16;
typedef unsigned long   u32;

typedef signed char     s08;
typedef signed short    s16;
typedef signed long     s32;

/**
 ** Common pointers.
 **/
typedef union {
    PACKED BYTE * b;
    PACKED WORD * w;
    PACKED DWORD * dw;
    PACKED QWORD * qw;
} CommomPointer_t;

/**
 ** Common nibbles.
 **/
typedef union {

    struct {
        BYTE byteL : 4, byteH : 4;
    };

    struct {
        WORD wordL : 8, wordH : 8;
    };

    struct {
        DWORD dwordL : 16, dwordH : 16;
    };
} PACKED CommomNibbles_t;

/******************************************************************************
 ** 2.3 Global macros
 ******************************************************************************/
/******************************************************************************
 ** 2.4 Global variables
 ******************************************************************************/
/******************************************************************************
 ** 2.5 Public function prototypes
 ******************************************************************************/

/******************************************************************************
 **
 ** END OF FILE
 **
 ******************************************************************************/
#ifdef	__cplusplus
}
#endif
#endif /* _COMMOM_H_ */
