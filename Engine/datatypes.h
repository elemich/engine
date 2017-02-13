#ifndef DATATYPES_H
#define DATATYPES_H


#include "t_datatypes.h"
#include "interfaces.h"

struct IntNumber : NumberInterface
{
};

struct FloatNumber : NumberInterface
{
};

struct IntSize : TVector<int>
{
	int& x;
	int& y;

	IntSize():TVector(2),x(operator[](0)),y(operator[](1)){}
	IntSize(int inputX,int inputY):TVector(2),x(operator[](0)),y(operator[](1)){x=inputX,y=inputY;}
};


#endif //DATATYPES_H



