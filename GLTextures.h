/***********************************************************************
GLTextures - Overloaded versions of the glTexImage... family of
functions for type-safe texture image handling and more flexible layout
of texture images in application memory.
Copyright (c) 1999-2006 Oliver Kreylos

This file is part of the 3D Data Visualization Library (3DVis).

The 3D Data Visualization Library is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The 3D Data Visualization Library is distributed in the hope that it
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the 3D Data Visualization Library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef GLTEXTURES_INCLUDED
#define GLTEXTURES_INCLUDED

#include <GL/gl.h>

/*******************************************
Overloaded versions of glTexImage functions:
*******************************************/

inline void glTexImage2D(GLenum target,GLint level,GLint internalFormat,GLsizei width,GLsizei height,GLint border,GLenum format,const GLubyte* pixels)
	{
	glTexImage2D(target,level,internalFormat,width,height,border,format,GL_UNSIGNED_BYTE,pixels);
	}

inline void glTexImage2D(GLenum target,GLint level,GLint internalFormat,GLsizei width,GLsizei height,GLint border,GLenum format,const GLbyte* pixels)
	{
	glTexImage2D(target,level,internalFormat,width,height,border,format,GL_BYTE,pixels);
	}

inline void glTexImage2D(GLenum target,GLint level,GLint internalFormat,GLsizei width,GLsizei height,GLint border,GLenum format,const GLushort* pixels)
	{
	glTexImage2D(target,level,internalFormat,width,height,border,format,GL_UNSIGNED_SHORT,pixels);
	}

inline void glTexImage2D(GLenum target,GLint level,GLint internalFormat,GLsizei width,GLsizei height,GLint border,GLenum format,const GLshort* pixels)
	{
	glTexImage2D(target,level,internalFormat,width,height,border,format,GL_SHORT,pixels);
	}

inline void glTexImage2D(GLenum target,GLint level,GLint internalFormat,GLsizei width,GLsizei height,GLint border,GLenum format,const GLuint* pixels)
	{
	glTexImage2D(target,level,internalFormat,width,height,border,format,GL_UNSIGNED_INT,pixels);
	}

inline void glTexImage2D(GLenum target,GLint level,GLint internalFormat,GLsizei width,GLsizei height,GLint border,GLenum format,const GLint* pixels)
	{
	glTexImage2D(target,level,internalFormat,width,height,border,format,GL_INT,pixels);
	}

inline void glTexImage2D(GLenum target,GLint level,GLint internalFormat,GLsizei width,GLsizei height,GLint border,GLenum format,const GLfloat* pixels)
	{
	glTexImage2D(target,level,internalFormat,width,height,border,format,GL_FLOAT,pixels);
	}

/****************************************************************************
Version of glTexSubImage2D with more flexible in-memory texture image layout:
****************************************************************************/

template <class TexelType>
inline
void
glTexSubImage2D(
	GLenum target,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLsizei width,
	GLsizei height,
	GLint columnStride, // Distance between adjacent texels in the same row (==1 if rows are stored consecutively)
	GLint rowStride, // Distance between adjacent texels in the same column (==width for compact images)
	GLenum format,
	GLenum type,
	const TexelType* pixels)
	{
	/* Set the common pixel pipeline parameters: */
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS,0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS,0);
	
	if(columnStride==1)
		{
		/* Upload the texture image directly: */
		glPixelStorei(GL_UNPACK_ROW_LENGTH,rowStride);
		glTexSubImage2D(target,level,xoffset,yoffset,width,height,format,type,pixels);
		}
	else
		{
		/* Copy the texture image to a contiguous buffer: */
		TexelType* tempPixels=new TexelType[width*height];
		TexelType* tempPixelPtr=tempPixels;
		const TexelType* rowPtr=pixels;
		for(int y=0;y<height;++y,rowPtr+=rowStride)
			{
			const TexelType* columnPtr=rowPtr;
			for(int x=0;x<width;++x,++tempPixelPtr,columnPtr+=columnStride)
				*tempPixelPtr=*columnPtr;
			}
		
		/* Upload the temporary texture image: */
		glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
		glTexSubImage2D(target,level,xoffset,yoffset,width,height,format,type,tempPixels);
		
		/* Delete the temporary texture image: */
		delete[] tempPixels;
		}
	}

#endif
