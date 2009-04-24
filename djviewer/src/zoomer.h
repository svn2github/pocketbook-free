/* 
 * File:   zoomer.h
 * Author: alexander
 *
 */

#ifndef _ZOOMER_H
#define	_ZOOMER_H

typedef struct tagZoomerParameters
{
    int zoom;
    int leftMarginForEvenPage, rightMarginForEvenPage;
    int leftMarginForOddPage, rightMarginForOddPage;

    ddjvu_document_t* doc;
    int cpage;
    int orient;
} ZoomerParameters;

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*CloseHandler)(ZoomerParameters* params);

// show new zoomer
void ShowZoomer(ZoomerParameters* params, CloseHandler closeHandler);

#ifdef __cplusplus
}
#endif

#endif	/* _ZOOMER_H */

