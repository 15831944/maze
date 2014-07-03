#ifndef __GLTexture__glt__h
#define __GLTexture__glt__h

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif

#ifdef linux
#include <cstdlib>
#endif
#ifdef __APPLE__
#include <unistd.h>
#endif
#include "OpenGLTools.h"
#include<memory>

struct __GLTexture__glt
{

    ////////////////////////////////////////////////////////////////////
    // Allocate memory and load targa bits. Returns pointer to new buffer,
    // height, and width of texture, and the OpenGL format of data.
    // Call free() on buffer when finished!
    // This only works on pretty vanilla targas... 8, 24, or 32 bit color
    // only, no palettes, no RLE encoding.
    static inline std::shared_ptr<GLbyte> readTGABits(const char *szFileName, GLint *iWidth, GLint *iHeight, GLint *iComponents, GLenum *eFormat)
        {
        /***************************************************************************/
        #pragma pack(1)
        typedef struct
        {
            GLbyte	identsize;                      // Size of ID field that follows header (0)
            GLbyte	colorMapType;                   // 0 = None, 1 = paletted
            GLbyte	imageType;                      // 0 = none, 1 = indexed, 2 = rgb, 3 = grey, +8=rle
            unsigned short	colorMapStart;          // First colour map entry
            unsigned short	colorMapLength;         // Number of colors
            unsigned char 	colorMapBits;           // bits per palette entry
            unsigned short	xstart;                 // image x origin
            unsigned short	ystart;                 // image y origin
            unsigned short	width;                  // width in pixels
            unsigned short	height;                 // height in pixels
            GLbyte	bits;                           // bits per pixel (8 16, 24, 32)
            GLbyte	descriptor;                     // image descriptor
        } TGAHEADER;
        #pragma pack()
        /**************************************************************************/
        FILE *pFile;			// File pointer
        TGAHEADER tgaHeader;		// TGA file header
        unsigned long lImageSize;		// Size in bytes of image
        short sDepth;			// Pixel depth;
        GLbyte *pBits = NULL;          // Pointer to bits

        // Default/Failed values
        *iWidth = 0;
        *iHeight = 0;
        *eFormat = GL_RGB;
        *iComponents = GL_RGB;

        // Attempt to open the file
        fopen_s(&pFile,szFileName, "rb");
        if(pFile == NULL)
        {
            cout<<szFileName<<" can not found!"<<endl;
            return NULL;
        }

        // Read in header (binary)
        fread(&tgaHeader, 18/* sizeof(TGAHEADER)*/, 1, pFile);

        // Do byte swap for big vs little endian
    #ifdef __APPLE__
        LITTLE_ENDIAN_WORD(&tgaHeader.colorMapStart);
        LITTLE_ENDIAN_WORD(&tgaHeader.colorMapLength);
        LITTLE_ENDIAN_WORD(&tgaHeader.xstart);
        LITTLE_ENDIAN_WORD(&tgaHeader.ystart);
        LITTLE_ENDIAN_WORD(&tgaHeader.width);
        LITTLE_ENDIAN_WORD(&tgaHeader.height);
    #endif


        // Get width, height, and depth of texture
        *iWidth = tgaHeader.width;
        *iHeight = tgaHeader.height;
        sDepth = tgaHeader.bits / 8;

        // Put some validity checks here. Very simply, I only understand
        // or care about 8, 24, or 32 bit targa's.
        if(tgaHeader.bits != 8 && tgaHeader.bits != 24 && tgaHeader.bits != 32)
        {   cout<<szFileName<<" bits error!"<<endl;
            return NULL;
        }

        // Calculate size of image buffer
        lImageSize = tgaHeader.width * tgaHeader.height * sDepth;

        // Allocate memory and check for success
        pBits = (GLbyte*)malloc(lImageSize * sizeof(GLbyte));
        if(pBits == NULL)
            return NULL;

        // Read in the bits
        // Check for read error. This should catch RLE or other
        // weird formats that I don't want to recognize
        if(fread(pBits, lImageSize, 1, pFile) != 1)
            {
            free(pBits);
            return NULL;
            }

        // Set OpenGL format expected
        switch(sDepth)
            {
    #ifndef OPENGL_ES
            case 3:     // Most likely case
                *eFormat = GL_BGR;
                *iComponents = GL_RGB;
                break;
    #endif
            case 4:
                *eFormat = GL_BGRA;
                *iComponents = GL_RGBA;
                break;
            case 1:
                *eFormat = GL_LUMINANCE;
                *iComponents = GL_LUMINANCE;
                break;
            default:        // RGB
                // If on the iPhone, TGA's are BGR, and the iPhone does not
                // support BGR without alpha, but it does support RGB,
                // so a simple swizzle of the red and blue bytes will suffice.
                // For faster iPhone loads however, save your TGA's with an Alpha!
    #ifdef OPENGL_ES
        for(int i = 0; i < lImageSize; i+=3)
            {
            GLbyte temp = pBits[i];
            pBits[i] = pBits[i+2];
            pBits[i+2] = temp;
            }
    #endif
            break;
            }



        // Done with File
        fclose(pFile);

        // Return pointer to image data
        return std::shared_ptr<GLbyte>(
                    pBits,
                    [](GLbyte * p){free(p);}
        );
   }


    struct TGAFormat
    {
        std::string fileName;
        GLint width;
        GLint height;
        GLint components;
        GLenum format;
        std::shared_ptr<GLbyte> data;
        bool read(const std::string & name)
        {
            fileName=name;
            data = readTGABits(fileName.c_str(),&width,&height,&components,&format);
#ifdef _DEBUG
            if(data.get()==0)
            cout<<"read tga error"<<endl;
#endif
            return(data.get()!=0);
        }
        void clear()
        {
            data=std::shared_ptr<GLbyte>();
        }
    };

    /**************************************************************/

    ////////////////////////////////////////////////////////////////////
    // Capture the current viewport and save it as a targa file.
    // Be sure and call SwapBuffers for double buffered contexts or
    // glFinish for single buffered contexts before calling this function.
    // Returns 0 if an error occurs, or 1 on success.
    // Does not work on the iPhone
    #ifndef OPENGL_ES
    static inline GLint grabScreenTGA(const char *szFileName)
        {
        /***************************************************************************/
#pragma pack(1)
        struct  TGAHEADER
        {
            /*1*/GLbyte	identsize;                      // Size of ID field that follows header (0)
            /*1*/GLbyte	colorMapType;                   // 0 = None, 1 = paletted
            /*1*/GLbyte	imageType;                      // 0 = none, 1 = indexed, 2 = rgb, 3 = grey, +8=rle
            /*2*/unsigned short	colorMapStart;          // First colour map entry
            /*2*/unsigned short	colorMapLength;         // Number of colors
            /*2*/unsigned char 	colorMapBits;           // bits per palette entry
            /*2*/unsigned short	xstart;                 // image x origin
            /*2*/unsigned short	ystart;                 // image y origin
            /*2*/unsigned short	width;                  // width in pixels
            /*2*/unsigned short	height;                 // height in pixels
            /*1*/GLbyte	bits;                           // bits per pixel (8 16, 24, 32)
            /*1*/GLbyte	descriptor;                     // image descriptor
        } ;
#pragma pack()
        /**************************************************************************/
        FILE *pFile;                // File pointer
        TGAHEADER tgaHeader;		// TGA file header
        unsigned long lImageSize;   // Size in bytes of image
        GLbyte	*pBits = NULL;      // Pointer to bits
        GLint iViewport[4];         // Viewport in pixels
        GLenum lastBuffer;          // Storage for the current read buffer setting

        // Get the viewport dimensions
        glGetIntegerv(GL_VIEWPORT, iViewport);

        // How big is the image going to be (targas are tightly packed)
        lImageSize = iViewport[2] * 3 * iViewport[3];

        // Allocate block. If this doesn't work, go home
        pBits = (GLbyte *)malloc(lImageSize);
        if(pBits == NULL)
            return 0;

        // Read bits from color buffer
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_PACK_ROW_LENGTH, 0);
        glPixelStorei(GL_PACK_SKIP_ROWS, 0);
        glPixelStorei(GL_PACK_SKIP_PIXELS, 0);

        // Get the current read buffer setting and save it. Switch to
        // the front buffer and do the read operation. Finally, restore
        // the read buffer state
        glGetIntegerv(GL_READ_BUFFER, (GLint *)&lastBuffer);
        glReadBuffer(GL_FRONT);
        glReadPixels(0, 0, iViewport[2], iViewport[3], GL_BGR_EXT, GL_UNSIGNED_BYTE, pBits);
        glReadBuffer(lastBuffer);

        // Initialize the Targa header
        tgaHeader.identsize = 0;
        tgaHeader.colorMapType = 0;
        tgaHeader.imageType = 2;
        tgaHeader.colorMapStart = 0;
        tgaHeader.colorMapLength = 0;
        tgaHeader.colorMapBits = 0;
        tgaHeader.xstart = 0;
        tgaHeader.ystart = 0;
        tgaHeader.width = iViewport[2];
        tgaHeader.height = iViewport[3];
        tgaHeader.bits = 24;
        tgaHeader.descriptor = 0;

        // Do byte swap for big vs little endian
    #ifdef __APPLE__
        LITTLE_ENDIAN_WORD(&tgaHeader.colorMapStart);
        LITTLE_ENDIAN_WORD(&tgaHeader.colorMapLength);
        LITTLE_ENDIAN_WORD(&tgaHeader.xstart);
        LITTLE_ENDIAN_WORD(&tgaHeader.ystart);
        LITTLE_ENDIAN_WORD(&tgaHeader.width);
        LITTLE_ENDIAN_WORD(&tgaHeader.height);
    #endif

        // Attempt to open the file
        fopen_s(&pFile,szFileName, "wb");
        if(pFile == NULL)
         {
            free(pBits);    // Free buffer and return error
            return 0;
        }

        // Write the header
        fwrite(&tgaHeader, sizeof(TGAHEADER), 1, pFile);

        // Write the image data
        fwrite(pBits, lImageSize, 1, pFile);

        // Free temporary buffer and close the file
        free(pBits);
        fclose(pFile);

        // Success!
        return 1;
        }
    #endif



};

#endif
