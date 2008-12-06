/***********************************************************************
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

#ifndef VISUALIZATION_CONCRETE_NOISE_INCLUDED
#define VISUALIZATION_CONCRETE_NOISE_INCLUDED

#include <assert.h>
#include <Geometry/Point.h>

namespace Visualization {

namespace Concrete {

class InfiniteArray // Class for "infinite" three-dimensional arrays
	{
	/* Embedded classes: */
	public:
	typedef Geometry::Point<float,3> Point;
	
	/* Elements: */
	private:
	int sizeBits; // Zweierlogarithmus der Seitenl&auml;nge eines Elementarw&uuml;rfels
	int size;
	int sizeMask; // Seitenl&auml;nge eines Elementarw&uuml;rfels minus 1
	int degree; // Grad der Interpolation
	float domainSize; // Kantenl&auml;nge eines Elementarw&uuml;rfels
	unsigned char* array; // Array der Werte
	float* deBoorArray; // Hilfsarray f&uuml;r den deBoor-Interpolationsalgorithmus
	
	/* Constructors and destructors: */
	public:
	InfiniteArray(int sSizeBits,int sDegree); // Erstellt Array, keine Initialisierung
	InfiniteArray(const InfiniteArray& other); // Copykonstruktor
	~InfiniteArray(void);
	
	/* Methods: */
	void set(int x,int y,int z,unsigned char val)
		{
		array[(x*size+y)*size+z]=val;
		}
	unsigned char operator()(int x,int y,int z) const // Arrayzugriffsoperator
		{
		return array[((((x&sizeMask)<<sizeBits)|(y&sizeMask))<<sizeBits)|(z&sizeMask)];
		}
	unsigned char& operator()(int x,int y,int z) // Ditto
		{
		int index=((((x&sizeMask)<<sizeBits)|(y&sizeMask))<<sizeBits)|(z&sizeMask);
		assert(index>=0&&index<size*size*size);
		return array[index];
		}
	float operator()(const Point& p) const; // Arrayzugriff mit Interpolation
	};

class Noise // Class for Perlin noise
	{
	/* Embedded classes: */
	public:
	typedef InfiniteArray::Point Point;
	
	/* Elements: */
	private:
	InfiniteArray noiseArray; // Feld von Zufallszahlen
	
	/* Constructors and destructors: */
	public:
	Noise(int sSizeBits,int sDegree =1); // Erzeugt Zufallsfeld der Gr&ouml;&szlig;e 1<<sSizeBits mit Interpolationsgrad sDegree
	Noise(const Noise& source); // Copykonstruktor
	
	/* Zugriffsfunktionen: */
	float calcNoise(const Point& p) const // Wert des Zahlenfeldes an der Stelle p
		{
		return noiseArray(p);
		}
	float calcTurbulence(const Point& p,int depth) const; // Turbulenzfunktion
	};

}

}

#endif
