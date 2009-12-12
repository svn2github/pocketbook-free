/*
 *   Copyright (C) 2009 Yury P. Fedorchenko
 *   yuryfdr@users.sf.net
 *   Copyright (C) 2002-2003 Andi Peredri                                  
 *   andi@ukr.net                                                          
 *   Copyright (C) 2004-2005 Artur Wiebe                                   
 *   wibix@gmx.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#ifndef RCHECKERS_H
#define RCHECKERS_H

#include "checkers.h"
//#include "pdn.h"


class RCheckers:public Checkers
{

public:
    virtual bool go1(int,int);
    virtual bool go2_human(int,int);


    virtual int type() const { return RUSSIAN; }

    virtual bool checkCapture1() const;
    virtual bool checkCapture2() const;

protected:
    virtual bool checkCapture1(int) const;

private:

    void kingMove2(int,int &);

    bool manCapture1(int,int,bool &);
    bool kingCapture1(int,int,bool &);

    bool manCapture2(int,int &);
    bool kingCapture2(int,int,int &);
    // for 2 player game
    bool manCapture2(int,int,bool &);
    bool kingCapture2(int,int,bool &);
};

#endif


