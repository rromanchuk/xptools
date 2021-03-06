/*
 * Copyright (c) 2004, Laminar Research.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#include "TexUtils.h"
#include "AssertUtils.h"
#include "BitmapUtils.h"

#if APL
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include "glew.h"
	#include <GL/glu.h>
#endif


struct  gl_info_t {
	int		gl_major_version;
	bool	has_tex_compression;
	bool	has_non_pots;
	bool	has_bgra;
	int		max_tex_size;
	bool	has_rgtc;
};

static gl_info_t gl_info = { 0 };

#define INIT_GL_INFO		if(gl_info.gl_major_version == 0) init_gl_info(&gl_info);

static void init_gl_info(gl_info_t * i)
{
	CHECK_GL_ERR
	const char * ver_str = (const char *)glGetString(GL_VERSION);      CHECK_GL_ERR
	const char * ext_str = (const char *)glGetString(GL_EXTENSIONS);   CHECK_GL_ERR

	sscanf(ver_str,"%d", &i->gl_major_version);
#if APL
/* Apple only gives by default a 2.1 compatible openGL contexts.
   BUT if openGL 3.2 or higher compatible contexts are requested by adding
   "NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core," just as the textbooks
   say - all openGl 1/2 direct drawing is disbled. i.e. most openGl functions used
   by WED are unavailable. So we can only test for openGL 2 core functionality available 
   here and pray glew gets us the rest ...
*/
	if(i->gl_major_version < 2)
	{
		LOG_MSG("OpenGL 2.0 or higher required. Found version '%s'\n", ver_str);
		AssertPrintf("OpenGL 2.0 or higher required. Found version '%s'\n", ver_str);
	}
#else
	if(i->gl_major_version < 3)
	{
		LOG_MSG("OpenGL 3.0 or higher required. Found version '%s'\n", ver_str);
		AssertPrintf("OpenGL 3.0 or higher required. Found version '%s'\n", ver_str);
	}
#endif
	i->has_tex_compression = strstr(ext_str,"GL_ARB_texture_compression") != NULL;
	i->has_non_pots = strstr(ext_str,"GL_ARB_texture_non_power_of_two") != NULL;
	i->has_bgra = strstr(ext_str,"GL_EXT_bgra") != NULL;
	i->has_rgtc = strstr(ext_str,"GL_ARB_texture_compression_rgtc") != NULL;

	glGetIntegerv(GL_MAX_TEXTURE_SIZE,&i->max_tex_size);
	// if(i->max_tex_size > 2*8192)	i->max_tex_size = 2*8192;
	if(i->has_tex_compression)	glHint(GL_TEXTURE_COMPRESSION_HINT,GL_NICEST); CHECK_GL_ERR
	LOG_MSG("OpenGL renderer  : %s\n", glGetString(GL_RENDERER));              CHECK_GL_ERR
	LOG_MSG("OpenGL Version   : %s\n", ver_str);
	LOG_MSG("Max texture size : %5d  DXT : %d   POT : %d  RGTC : %d\n", i->max_tex_size,
	                                        i->has_tex_compression, i->has_non_pots, i->has_rgtc);
	LOG_MSG("                          FBO : %d   VBO : %d  MSext : %d\n", strstr(ext_str, "GL_ARB_framebuffer_object") != nullptr,
											strstr(ext_str, "GL_ARB_vertex_buffer_object") != nullptr,
	                                        strstr(ext_str, "GL_EXT_framebuffer_multisample") != nullptr);
	LOG_FLUSH();
}

/*****************************************************************************************
 * UTILS
 *****************************************************************************************/


inline int NextPowerOf2(int a)
{
	GLint	maxDim = gl_info.max_tex_size;
	int rval = 2;
	while(rval < a && rval < maxDim)
		rval <<= 1;
	return rval;
}


void UnpadImage(ImageInfo * im)
{
	for (int y = 0; y < im->height; ++y)
	{
		unsigned char * src = im->data + (y * (im->width * im->channels + im->pad));
		unsigned char * dst = im->data + (y * (im->width * im->channels          ));
		if (src != dst)
			memmove(dst,src,im->width * im->channels);
	}
	im->pad = 0;
}

/*****************************************************************************************
 * LoadTextureFromFile
 *****************************************************************************************/
bool LoadTextureFromFile(
						const char * 	inFileName,
						int 			inTexNum,
						int 			flags,
						int * 			outWidth,
						int * 			outHeight,
						float *  		outS,
						float * 		outT)
{
	struct ImageInfo	im = { 0 };
	if (LoadBitmapFromAnyFile(inFileName, &im) == 0)
	{
		if (im.pad != 0)
			UnpadImage(&im);

		int res = LoadTextureFromImage(im, inTexNum, flags, outWidth, outHeight, outS, outT);
		DestroyBitmap(&im);
		return res;
	}
	return false;
}

/*****************************************************************************************
 * LoadTextureFromImage
 *****************************************************************************************/
bool LoadTextureFromImage(ImageInfo& im, int inTexNum, int inFlags, int * outWidth, int * outHeight, float * outS, float * outT)
{
	INIT_GL_INFO

	// Process alpha.  Then remove padding.  Finally, figure out the next biggest power of 2.  If we aren't
	// a power of 2, we may need to resize.  That will be done with rescaling if the user wants.  Also if the bitmap
	// is bigger than the max power of 2 supported by the HW, force rescaling.
	if (inFlags & tex_MagentaAlpha)	ConvertBitmapToAlpha(&im, true);
	if (im.pad != 0)
 		UnpadImage(&im);

	int non_pots = ((inFlags & tex_Always_Pad) == 0) && gl_info.has_non_pots;

	int		res_x = non_pots ? min((int) im.width, gl_info.max_tex_size) : NextPowerOf2(im.width);
	int		res_y = non_pots ? min((int) im.height,gl_info.max_tex_size) : NextPowerOf2(im.height);
	bool	resize = (res_x != im.width || res_y != im.height);
	bool	rescale = resize && (inFlags & tex_Rescale);
	if (im.width > res_x || im.height > res_y)	// Always rescale if the image is too big for the max tex!!
		rescale = true;

	ImageInfo *	useIt = &im;
	ImageInfo	rescaleBits;
	if (resize)
	{
		if (CreateNewBitmap(res_x, res_y, im.channels, &rescaleBits) != 0)
		{
			return false;
		}
		useIt = &rescaleBits;

		if (rescale) {
			CopyBitmapSection(&im, &rescaleBits, 0, 0, im.width, im.height, 0, 0, rescaleBits.width, rescaleBits.height);
			if (outS) *outS = 1.0;
			if (outT) *outT = 1.0;
		} else {
			CopyBitmapSectionDirect(im, rescaleBits, 0, 0, 0, 0, im.width, im.height);
			if (im.width < rescaleBits.width)
				CopyBitmapSectionDirect(im, rescaleBits, im.width-1, 0, im.width, 0, 1, im.height);
			if (im.height < rescaleBits.height)
				CopyBitmapSectionDirect(im, rescaleBits, 0, im.height-1, 0, im.height, im.width, 1);
			if (im.height < rescaleBits.height && im.width < rescaleBits.width)
				CopyBitmapSectionDirect(im, rescaleBits, im.width-1, im.height-1, im.width, im.height, 1, 1);

			if (outS) *outS = (float) im.width / (float) rescaleBits.width;
			if (outT) *outT = (float) im.height / (float) rescaleBits.height;
		}
	} else {
		if (outS) *outS = 1.0;
		if (outT) *outT = 1.0;
	}


	if (outWidth) *outWidth = useIt->width;
	if (outHeight) *outHeight = useIt->height;

	glBindTexture(GL_TEXTURE_2D, inTexNum);  CHECK_GL_ERR
	
	int	iformat, glformat;
	if (useIt->channels == 1)
	{
		iformat = glformat = GL_ALPHA;
	}
	else if(gl_info.has_bgra)
	{
		                            iformat = GL_RGB;  glformat = GL_BGR;
		if (useIt->channels == 4) { iformat = GL_RGBA; glformat = GL_BGRA; }
	}
	else
	{
		long cnt = useIt->width * useIt->height;
		unsigned char * p = useIt->data;
		while (cnt--)
		{
			swap(p[0], p[2]);						// Ben says: since we get BGR or BGRA, swap red and blue channesl to make RGB or RGBA.  Some day we could
			p += useIt->channels;					// use our brains and use GL_BGR_EXT and GL_BGRA_EXT; literally all GL cards from Radeon/GeForce on support this.
			}
													// Michael says: that day was the last day of 2018. Eight years, 4 months and EXA bytes swapped by CPUs later.
													// But, its just a drop into the ocean: ALL images except BMP are channel order swapped when loading in BitmapUtils,
													// the real architectural misfortune was choosing the ImageInfo data format to be BGR rather than RGB.
													
								  iformat = glformat = GL_RGB;
		if (useIt->channels == 4) iformat = glformat = GL_RGBA;
	}

	if(gl_info.has_tex_compression && (inFlags & tex_Compress_Ok))
	{
		switch (iformat) {
		case GL_RGB:	iformat = GL_COMPRESSED_RGB;	break;
		case GL_RGBA:	iformat = GL_COMPRESSED_RGBA;	break;
		}
	}

	if (inFlags & tex_Mipmap)
		gluBuild2DMipmaps(GL_TEXTURE_2D, iformat, useIt->width, useIt->height, glformat, GL_UNSIGNED_BYTE, useIt->data); CHECK_GL_ERR
	else
		glTexImage2D(GL_TEXTURE_2D, 0, iformat, useIt->width ,useIt->height, 0,	glformat, GL_UNSIGNED_BYTE, useIt->data); CHECK_GL_ERR

	if (resize)
		DestroyBitmap(&rescaleBits);

	// BAS note: for some reason on my WinXP system with GF-FX, if
	// I do not set these explicitly to linear, I get no drawing at all.
	// Who knows what default state the card is in. :-(
//	if(inFlags & tex_Nearest)
//	{
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	} else 
	if (inFlags & tex_Linear) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (inFlags & tex_Mipmap) ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR);
	} else {
		// If we have nearest-neighboring and we are down-sampling WITHOUT a mip-map we STILL use linear in an attempt to keep this thing from looking TOTALY blitzed, I guess?
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (inFlags & tex_Mipmap) ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR);
	}
	if(inFlags & tex_Wrap) {
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT );
	    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT );
	}
	else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	return true;
}

#if LOAD_DDS_DIRECT

struct DXT1Block {
    uint16_t colors[2];
    uint8_t  rows[4];
};

struct DXT3Block {
    DXT1Block	alphas;
    DXT1Block 	colors;
};

struct DXT5AlphaBlock {
    uint8_t 		alpha[2];
    uint8_t			idx[6];
};

struct DXT5Block {
    DXT5AlphaBlock	alphas;
    DXT1Block		colors;
};

inline void swap_12bit_idx(uint8_t * a)
{
	uint64_t src = *(uint64_t *) a;
	uint64_t dst = src & 0xFFFF000000000000ull;

	dst |= ((src & 0xFFFull) << 36) | ((src & 0xFFF000ull) << 12) | ((src & 0xFFF000000ull)) >> 12 | ((src & 0xFFF000000000ull) >>36);

	*(uint64_t *) a = dst;
}

static void swap_blocks(char *a, char *b, GLint type)
{
	if (type == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
	{
		DXT5Block * x = (DXT5Block *) a;
		DXT5Block * y = (DXT5Block *) b;
		swap(*x,*y);
		swap_12bit_idx(x->alphas.idx);
		swap_12bit_idx(y->alphas.idx);
		swap(x->colors.rows[0], x->colors.rows[3]);
		swap(x->colors.rows[1], x->colors.rows[2]);
		swap(y->colors.rows[0], y->colors.rows[3]);
		swap(y->colors.rows[1], y->colors.rows[2]);
	}
	else if (type == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT)
	{
		DXT3Block * x = (DXT3Block *) a;
		DXT3Block * y = (DXT3Block *) b;
		swap(*x,*y);
		swap(x->alphas.rows[0], x->alphas.rows[3]);
		swap(x->alphas.rows[1], x->alphas.rows[2]);
		swap(y->alphas.rows[0], y->alphas.rows[3]);
		swap(y->alphas.rows[1], y->alphas.rows[2]);
		swap(x->colors.rows[0], x->colors.rows[3]);
		swap(x->colors.rows[1], x->colors.rows[2]);
		swap(y->colors.rows[0], y->colors.rows[3]);
		swap(y->colors.rows[1], y->colors.rows[2]);
	}
	else if (type == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		DXT1Block * x = (DXT1Block *) a;
		DXT1Block * y = (DXT1Block *) b;
		swap(*x,*y);
		swap(x->rows[0], x->rows[3]);
		swap(x->rows[1], x->rows[2]);
		swap(y->rows[0], y->rows[3]);
		swap(y->rows[1], y->rows[2]);
	}
	else // BC4 or BC5
	{
		DXT5AlphaBlock * x = (DXT5AlphaBlock  *)a;
		DXT5AlphaBlock * y = (DXT5AlphaBlock  *)b;
		swap(*x, *y);
		swap_12bit_idx(x->idx);
		swap_12bit_idx(y->idx);
		if (type == GL_COMPRESSED_SIGNED_RG_RGTC2)
		{
			++x; ++y;
			swap(*x, *y);
			swap_12bit_idx(x->idx);
			swap_12bit_idx(y->idx);
		}
	}
}

static void swap_blocks(char *a, GLint type)
{
	if (type == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
	{
		DXT5Block * x = (DXT5Block *)a;
		swap_12bit_idx(x->alphas.idx);
		swap(x->colors.rows[0], x->colors.rows[3]);
		swap(x->colors.rows[1], x->colors.rows[2]);
	}
	else if (type == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT)
	{
		DXT3Block * x = (DXT3Block *)a;
		swap(x->alphas.rows[0], x->alphas.rows[3]);
		swap(x->alphas.rows[1], x->alphas.rows[2]);
		swap(x->colors.rows[0], x->colors.rows[3]);
		swap(x->colors.rows[1], x->colors.rows[2]);
	}
	else if (type == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	{
		DXT1Block * x = (DXT1Block *)a;
		swap(x->rows[0], x->rows[3]);
		swap(x->rows[1], x->rows[2]);
	}
	else // BC4 or BC5
	{
		DXT5AlphaBlock * x = (DXT5AlphaBlock  *)a;
		swap_12bit_idx(x->idx);
		if (type == GL_COMPRESSED_SIGNED_RG_RGTC2)
		{
			++x;
			swap_12bit_idx(x->idx);
		}
	}
}

bool	LoadTextureFromDDS(
				char *			mem_start,
				char *			mem_end,
				int				in_tex_num,
				int				inFlags,
				int *			outWidth,
				int *			outHeight)
{
	INIT_GL_INFO

	if((mem_end - mem_start) < sizeof(TEX_dds_desc)) return false;

	const TEX_dds_desc * desc = (const TEX_dds_desc *) mem_start;

	if (strncmp(desc->dwMagic, "DDS ", 4) != 0) return false;
	if((SWAP32(desc->dwSize)) != (sizeof(*desc) - sizeof(desc->dwMagic))) return false;

	GLenum glformat;
	int	dds_blocksize;

	if (strncmp(desc->ddpfPixelFormat.dwFourCC, "DXT", 3) == 0)
		switch (desc->ddpfPixelFormat.dwFourCC[3])
		{
		case '1':	dds_blocksize = 8;  glformat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;	break;
		case '3':	dds_blocksize = 16; glformat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;	break;
		case '5':	dds_blocksize = 16; glformat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;	break;
		default: return false;
		}
	else if (gl_info.has_rgtc && strncmp(desc->ddpfPixelFormat.dwFourCC, "ATI", 3) == 0)
		switch (desc->ddpfPixelFormat.dwFourCC[3])
		{
		case '1':	dds_blocksize = 8;  glformat = GL_COMPRESSED_RED_RGTC1;			break;
		case '2':	dds_blocksize = 16; glformat = GL_COMPRESSED_SIGNED_RG_RGTC2; break; // for normal maps, some day
		default: return false;
		}
	else
		return false;

	int mips = 0;
	if(inFlags & tex_Mipmap && (SWAP32(desc->dwFlags)) & DDSD_MIPMAPCOUNT)
		mips = SWAP32(desc->dwMipMapCount);
	int x = SWAP32(desc->dwWidth);
	int y = SWAP32(desc->dwHeight);

	if ((mips && y != NextPowerOf2(y)) || y % 8 != 0) return false;  // flipping code can only handle certain heights

	if (outWidth) *outWidth = x;
	if (outHeight) *outHeight = y;

	char * data = mem_start + sizeof(TEX_dds_desc);

	glBindTexture(GL_TEXTURE_2D, in_tex_num);

	for (int level = 0; level <= mips; ++level)
	{
		int data_len = max(1 , (x * y) / 16) *  dds_blocksize;
		if((data + data_len) > mem_end) return false;        // not enough data for mipmaps = broken dds !

		// lossless flip image in Y-direction to match orientation of all other textures in XPtools/X-plane
		// to match old MSFT DIB convention (0,0) == left bottom, but DXT starts at left top.
		{
			int blocks_per_line = max(1, x / 4);
			int line_len = blocks_per_line * dds_blocksize;
			int swap_count = y / 4 / 2;
			char * dds_line1 = data;
			char * dds_line2 = data + data_len - line_len;

//			printf("%dx%d %dx%d %d %d/%d\n", *outWidth, *outHeight, x, y, swap_count, level, mips);
			if(swap_count == 0)
				for (int i = 0; i < blocks_per_line; i++)
				{
					swap_blocks(dds_line1, glformat);
					dds_line1 += dds_blocksize;
				}
			while (swap_count--)
			{
				for (int i = 0; i < blocks_per_line; i++)
				{
					swap_blocks(dds_line1, dds_line2, glformat);
					dds_line1 += dds_blocksize;
					dds_line2 += dds_blocksize;
				}
				dds_line2 -= 2 * line_len;
			}
		}

		glCompressedTexImage2D( GL_TEXTURE_2D, level, glformat, x, y, 0, data_len, data); CHECK_GL_ERR

		x = max(1, x >> 1);
		y = max(1, y >> 1);
		data += data_len;
	}

	if (inFlags & tex_Linear)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (inFlags & tex_Mipmap) ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (inFlags & tex_Mipmap) ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR);
	}

	if (inFlags & tex_Wrap)
	{
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT );
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT );
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	}
	return true;
}
#endif
