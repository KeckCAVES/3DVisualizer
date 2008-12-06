***********************************************************************
Noise - Class for Perlin noise arrays with spline evaluation.
Copyright (c) 2000-2007 Oliver Kreylos

This file is part of the 3D Data Visualizer (Visualizer).

The 3D Data Visualizer is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

The 3D Data Visualizer is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the 3D Data Visualizer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <Math/Math.h>
#include <Math/Random.h>

#include <Concrete/Noise.h>

namespace Visualization {

namespace Concrete {

/******************************
Methods of class InfiniteArray:
******************************/

InfiniteArray::InfiniteArray(int sSizeBits,int sDegree)
	:sizeBits(sSizeBits),size(1<<sizeBits),sizeMask((1<<sizeBits)-1),
	 degree(sDegree),
	 domainSize(float(1<<sizeBits)),
	 array(new unsigned char[size*size*size]),
	 deBoorArray(new float[(degree+1)*(degree+1)*(degree+1)*(degree+1)])
	{
	}

InfiniteArray::InfiniteArray(const InfiniteArray& other)
	:sizeBits(other.sizeBits),size(other.size),sizeMask(other.sizeMask),
	 degree(other.degree),
	 domainSize(other.domainSize),
	 array(new unsigned char[size*size*size]),
	 deBoorArray(new float[(degree+1)*(degree+1)*(degree+1)*(degree+1)])
	{
	memcpy(array,other.array,size*size*size*sizeof(unsigned char));
	}

InfiniteArray::~InfiniteArray(void)
	{
	delete[] array;
	delete[] deBoorArray;
	}

float InfiniteArray::operator()(const InfiniteArray::Point& p) const
	{
	int index[3];
	float d[3];
	for(int i=0;i<3;++i)
		{
		d[i]=Math::mod(p[i],domainSize);
		index[i]=int(Math::floor(d[i]));
		d[i]-=float(index[i]);
		}
	if(degree==1)
		{
		float i0=float(operator()(index[0],index[1],index[2]))*(1.0f-d[0])+float(operator()(index[0]+1,index[1],index[2]))*d[0];
		float i1=float(operator()(index[0],index[1]+1,index[2]))*(1.0f-d[0])+float(operator()(index[0]+1,index[1]+1,index[2]))*d[0];
		float i2=float(operator()(index[0],index[1],index[2]+1))*(1.0f-d[0])+float(operator()(index[0]+1,index[1],index[2]+1))*d[0];
		float i3=float(operator()(index[0],index[1]+1,index[2]+1))*(1.0f-d[0])+float(operator()(index[0]+1,index[1]+1,index[2]+1))*d[0];
		
		float i4=i0*(1.0f-d[1])+i1*d[1];
		float i5=i2*(1.0f-d[1])+i3*d[1];
		
		return (i4*(1.0-d[2])+i5*d[2])/255.0f;
		}
	else
		{
		int ys=degree+1;
		int xs=ys*(degree+1);
		int ss=xs*(degree+1);
		float* arrayPtr=deBoorArray;
		int i,j,k;
		for(i=0;i<=degree;++i)
			for(j=0;j<=degree;++j)
				for(k=0;k<=degree;++k,++arrayPtr)
					*arrayPtr=float(operator()(index[0]+i,index[1]+j,index[2]+k));
		for(int step=0;step<degree;++step)
			{
			int sd=degree-step;
			for(i=0;i<sd;++i)
				{
				float xw1=((float)(i+1)-d[0])/(float)sd;
				float xw2=1.0f-xw1;
				for(j=0;j<sd;++j)
					{
					float yw1=((float)(j+1)-d[1])/(float)sd;
					float yw2=1.0f-yw1;
					for(k=0;k<sd;++k)
						{
						float zw1=((float)(k+1)-d[2])/(float)sd;
						float zw2=1.0f-zw1;
						int base=step*ss+i*xs+j*ys+k;
						float i1=deBoorArray[base]*xw1+deBoorArray[base+xs]*xw2;
						float i2=deBoorArray[base+ys]*xw1+deBoorArray[base+xs+ys]*xw2;
						float i3=deBoorArray[base+1]*xw1+deBoorArray[base+xs+1]*xw2;
						float i4=deBoorArray[base+ys+1]*xw1+deBoorArray[base+xs+ys+1]*xw2;
						float i5=i1*yw1+i2*yw2;
						float i6=i3*yw1+i4*yw2;
						deBoorArray[base+ss]=i5*zw1+i6*zw2;
						}
					}
				}
			}
		return deBoorArray[degree*ss]/255.0f;
		}
	}

/**********************
Methods of class Noise:
**********************/

Noise::Noise(int sSizeBits,int sDegree)
	:noiseArray(sSizeBits,sDegree)
	{
	#if 1
	int i,j,k;
	int size=1<<sSizeBits;
	for(i=0;i<size;i++)
		for(j=0;j<size;j++)
			for(k=0;k<size;k++)
				noiseArray.set(i,j,k,(unsigned char)Math::randUniformCO(0,256));
	#endif
	}

Noise::Noise(const Noise& source)
	:noiseArray(source.noiseArray)
	{
	}

inline float fpos(float x)
	{
	return x>0.0f?x:0.0f;
	}

float Noise::calcTurbulence(const Noise::Point& p,int depth) const
	{
	float result=0.0f;
	float weight=1.0f;
	Point ps=p;
	for(int i=0;i<depth;i++)
		{
		result+=(noiseArray(ps)-0.5f)*weight;
		weight*=1.0f/3.0f;
		for(int j=0;j<3;++j)
			ps[j]*=Math::sqrt(3.0f);
		}
	return result;
	}

}

}
