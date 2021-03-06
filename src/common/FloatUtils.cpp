/*********************                                                        */
/*! \file FloatUtils.cpp
 ** \verbatim
 ** Top contributors (to current version):
 **   Guy Katz, Duligur Ibeling
 ** This file is part of the Marabou project.
 ** Copyright (c) 2017-2019 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved. See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** [[ Add lengthier description here ]]

**/

#include "FloatUtils.h"
#include "Debug.h"
#include <iomanip>
#include <sstream>

// https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
// http://docs.oracle.com/cd/E19957-01/806-3568/ncg_goldberg.html

bool FloatUtils::areEqual( double x, double y, double epsilon )
{
    // Check if the numbers are really close -- needed
    // when comparing numbers near zero.
    double diff = fabs( x - y );
    if ( diff <= epsilon )
        return true;

    x = fabs( x );
    y = fabs( y );
    double largest = ( x > y ) ? x : y;

    if ( diff <= largest * DBL_EPSILON )
        return true;

    return false;
}

String FloatUtils::doubleToString( double x, unsigned precision )
{
    std::ostringstream strout;
    strout << std::fixed << std::setprecision(precision) << x;
    std::string str = strout.str();
    size_t end = str.find_last_not_of( '0' ) + 1;
    str.erase( end );

    if ( str[str.size() - 1] == '.' )
        str = str.substr(0, str.size() - 1);

    return str;
}

bool FloatUtils::isNan( double x )
{
    return isnan( x );
}

bool FloatUtils::isInf( double x )
{
    return isinf( x );
}

double FloatUtils::sigmoid( double x )
{
    return 1.0 / ( 1.0 + exp( -x ) );
}

double FloatUtils::sigmoidInverse( double x )
{
//    if ( gte(x, 1) )
//        return 100;
//    if ( lte(x, 0) )
//        return -100;
    return log ( x / ( 1.0 - x ) );
}

double FloatUtils::sigmoidDerivative( double x )
{
    return sigmoid( x ) * (1 - sigmoid( x ));
}





//
// Local Variables:
// compile-command: "make -C ../.. "
// tags-file-name: "../../TAGS"
// c-basic-offset: 4
// End:
//
