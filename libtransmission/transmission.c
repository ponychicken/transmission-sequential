/******************************************************************************
 * $Id$
 *
 * Copyright (c) 2005-2007 Transmission authors and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *****************************************************************************/

#include "transmission.h"
#include "shared.h"

/***********************************************************************
 * tr_init
 ***********************************************************************
 * Allocates a tr_handle_t structure and initializes a few things
 **********************************************************************/
tr_handle_t * tr_init()
{
    tr_handle_t * h;
    int           i, r;

    tr_msgInit();
    tr_netResolveThreadInit();

    h = calloc( sizeof( tr_handle_t ), 1 );

    /* Generate a peer id : "-TRxxyy-" + 12 random alphanumeric
       characters, where xx is the major version number and yy the
       minor version number (Azureus-style) */
    sprintf( h->id, "-TR%02d%02d-", VERSION_MAJOR, VERSION_MINOR );
    for( i = 8; i < 20; i++ )
    {
        r        = tr_rand( 36 );
        h->id[i] = ( r < 26 ) ? ( 'a' + r ) : ( '0' + r - 26 ) ;
    }

    /* Random key */
    for( i = 0; i < 20; i++ )
    {
        r         = tr_rand( 36 );
        h->key[i] = ( r < 26 ) ? ( 'a' + r ) : ( '0' + r - 26 ) ;
    }

    /* Don't exit when writing on a broken socket */
    signal( SIGPIPE, SIG_IGN );

    /* Initialize rate and file descripts controls */
    h->uploadLimit   = -1;
    h->downloadLimit = -1;
    
    tr_fdInit();
    h->shared = tr_sharedInit( h );

    return h;
}

/***********************************************************************
 * tr_setBindPort
 ***********************************************************************
 * 
 **********************************************************************/
void tr_setBindPort( tr_handle_t * h, int port )
{
    h->isPortSet = 1;
    tr_sharedSetPort( h->shared, port );
}

void tr_natTraversalEnable( tr_handle_t * h, int enable )
{
    tr_sharedTraversalEnable( h->shared, enable );
}

int tr_natTraversalStatus( tr_handle_t * h )
{
    return tr_sharedTraversalStatus( h->shared );
}

void tr_setGlobalUploadLimit( tr_handle_t * h, int limit )
{
    h->uploadLimit = limit;
    tr_sharedSetLimit( h->shared, limit );
}

void tr_setGlobalDownloadLimit( tr_handle_t * h, int limit )
{
    h->downloadLimit = limit;
}

void tr_torrentRates( tr_handle_t * h, float * dl, float * ul )
{
    tr_torrent_t * tor;

    *dl = 0.0;
    *ul = 0.0;
    for( tor = h->torrentList; tor; tor = tor->next )
    {
        tr_lockLock( &tor->lock );
        if( tor->status & TR_STATUS_DOWNLOAD )
            *dl += tr_rcRate( tor->download );
        *ul += tr_rcRate( tor->upload );
        tr_lockUnlock( &tor->lock );
    }
}

int tr_torrentCount( tr_handle_t * h )
{
    return h->torrentCount;
}

void tr_torrentIterate( tr_handle_t * h, tr_callback_t func, void * d )
{
    tr_torrent_t * tor, * next;

    for( tor = h->torrentList; tor; tor = next )
    {
        next = tor->next;
        func( tor, d );
    }
}

void tr_close( tr_handle_t * h )
{
    tr_sharedClose( h->shared );
    tr_fdClose();
    free( h );

    tr_netResolveThreadClose();
}

