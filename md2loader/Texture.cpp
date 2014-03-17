/* -*- c++ -*- */
/////////////////////////////////////////////////////////////////////////////
//
// Texture.cpp -- Copyright (c) 2006-2007 David Henry
// last modification: july 13, 2007
//
// This code is licenced under the MIT license.
//
// This software is provided "as is" without express or implied
// warranties. You may freely copy and compile this source into
// applications you distribute provided that the copyright text
// below is included in the resulting source code.
//
// Implementation of an OpenGL texture classes.
//
/////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#define	WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif // _WIN32

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <iostream>
#include <stdexcept>

#include "Texture.h"
#include "Image.h"

using std::cout;
using std::cerr;
using std::endl;


/////////////////////////////////////////////////////////////////////////////
//
// class Texture implementation.
//
/////////////////////////////////////////////////////////////////////////////

// --------------------------------------------------------------------------
// Texture::Texture
//
// Constructor.
// --------------------------------------------------------------------------

Texture::Texture ()
  : _handle (0), _flags (kDefault),
    _standardCoordSystem (true), _fail (true)
{
  // Inhibit possible previous OpenGL error
  GLenum err = glGetError ();
  if (GL_NO_ERROR != err)
    cerr << "OpenGL Error: " << gluErrorString (err)
	 << " [Texture::Texture]" << endl;
}


// --------------------------------------------------------------------------
// Texture::~Texture
//
// Destructor.  Delete texture object.
// --------------------------------------------------------------------------

Texture::~Texture ()
{
  // Delete texture object
  if (glIsTexture(_handle))
    glDeleteTextures (1, &_handle);
}


// --------------------------------------------------------------------------
// Texture::bind
//
// Bind texture to the active texture unit.
// --------------------------------------------------------------------------

void
Texture::bind () const
{
  glBindTexture (target (), _handle);
}


// --------------------------------------------------------------------------
// Texture::getCompressionFormat
//
// Return the corresponding format for a compressed image given
// image's internal format (pixel components).
// --------------------------------------------------------------------------

GLint
Texture::getCompressionFormat (GLint internalFormat)
{
  if (!GLEW_EXT_texture_compression_s3tc ||
      !GLEW_ARB_texture_compression)
    // No compression possible on this target machine
    return internalFormat;

  switch (internalFormat)
    {
    case 1:
      return GL_COMPRESSED_LUMINANCE;

    case 2:
      return GL_COMPRESSED_LUMINANCE_ALPHA;

    case 3:
      return GL_COMPRESSED_RGB;

    case 4:
      return GL_COMPRESSED_RGBA;

    default:
      // Error!
      throw std::invalid_argument ("Texture::getCompressionFormat: "
				   "Bad internal format");
    }
}


// --------------------------------------------------------------------------
// Texture::getInternalFormat
//
// Return texture's internal format depending to whether compression
// is used or not.
// --------------------------------------------------------------------------

GLint
Texture::getInternalFormat (GLint components)
{
  if (_flags & kCompress)
    return getCompressionFormat (components);
  else
    return components;
}


/////////////////////////////////////////////////////////////////////////////
//
// class Texture2D implementation.
//
/////////////////////////////////////////////////////////////////////////////

// --------------------------------------------------------------------------
// Texture2D::Texture2D
//
// Constructors.  Try to load a texture 2D.  Don't throw any exception
// if it fails to create the texture; the program can still run whithout
// textures.
// --------------------------------------------------------------------------

Texture2D::Texture2D ()
  : Texture ()
{
}


Texture2D::Texture2D (const string &filename, TextureFlags flags)
{
  try
    {
      // Load image file into a buffer
      ImageBuffer ibuff (filename);
      auto_ptr<Image> img (ImageFactory::createImage (ibuff));

      // Create texture from image buffer
      create (img.get (), flags);
    }
  catch (std::exception &err)
    {
      cerr << "Error: Couldn't create texture 2D from "
	   << filename << endl;
      cerr << "Reason: " << err.what () << endl;
    }
}


Texture2D::Texture2D (const Image *img, TextureFlags flags)
{
  try
    {
      // Create texture from image buffer
      create (img, flags);
    }
  catch (std::exception &err)
    {
      cerr << "Error: Couldn't create texture 2D from "
	   << img->name () << endl;
      cerr << "Reason: " << err.what () << endl;
    }
}


// --------------------------------------------------------------------------
// Texture2D::create
//
// Create a texture 2D from an image.
// --------------------------------------------------------------------------

void
Texture2D::create (const Image *img, TextureFlags flags)
{
  if (!img->pixels ())
    throw std::runtime_error ("Invalid image data");

  // Fill texture's vars
  _name = img->name ();
  _flags = flags;
  _standardCoordSystem = img->stdCoordSystem ();

  // Generate a texture name
  glGenTextures (1, &_handle);
  glBindTexture (target (), _handle);

  // Setup texture filters
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // Build the texture and generate mipmaps
  if (GLEW_SGIS_generate_mipmap && img->isPowerOfTwo ())
    {
      // Hardware mipmap generation
      glTexParameteri (GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
      glHint (GL_GENERATE_MIPMAP_HINT_SGIS, GL_NICEST);

      glTexImage2D (GL_TEXTURE_2D, 0, getInternalFormat (img->components ()),
		    img->width (), img->height (), 0, img->format (),
		    GL_UNSIGNED_BYTE, img->pixels ());
    }
  else
    {
      // No hardware mipmap generation support, fall back to the
      // good old gluBuild2DMipmaps function
      gluBuild2DMipmaps (GL_TEXTURE_2D, getInternalFormat (img->components ()),
			 img->width (), img->height (), img->format (),
			 GL_UNSIGNED_BYTE, img->pixels ());
    }

  // Does texture creation succeeded?
  GLenum err = glGetError ();
  if (GL_NO_ERROR == err)
     _fail = false;
  else
      cerr << "OpenGL Error: " << gluErrorString (err)
	   << " [Texture2D::create ()]" << endl;
}
