/* -*- c++ -*- */
/////////////////////////////////////////////////////////////////////////////
//
// Md2Player.h -- Copyright (c) 2006 David Henry
// last modification: feb. 10, 2006
//
// This code is licenced under the MIT license.
//
// This software is provided "as is" without express or implied
// warranties. You may freely copy and compile this source into
// applications you distribute provided that the copyright text
// below is included in the resulting source code.
//
// Definition of MD2 Player Class.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __MD2PLAYER_H__
#define __MD2PLAYER_H__

#include <vector>
#include <string>

#include "Md2Model.h"

using std::vector;
using std::string;
using std::auto_ptr;


/////////////////////////////////////////////////////////////////////////////
//
// class Md2Player -- MD2 Player Class.
//
/////////////////////////////////////////////////////////////////////////////

class Md2Player
{
public:
  // Constructors/destructor
  Md2Player (const string &dirname)
    throw (std::runtime_error);
  ~Md2Player ();

public:
  // Public interface
  void drawPlayerItp (bool animated, Md2Object::Md2RenderMode renderMode);
  void drawPlayerFrame (int frame, Md2Object::Md2RenderMode renderMode);
  void animate (float percent);

  // Setters and accessors
  void setScale (GLfloat scale);
  void setSkin (const string &name);
  void setAnim (const string &name);

  const string &name () const { return _name; }
  const string &currentSkin () const { return _currentSkin; }
  const string &currentAnim () const { return _currentAnim; }

  const Md2Model *playerMesh () const { return _playerMesh.get (); }
  const Md2Model *weaponMesh () const { return _weaponMesh.get (); }

private:
  // Member variables
  typedef auto_ptr<Md2Model> Md2ModelPtr;

  Md2ModelPtr _playerMesh;
  Md2ModelPtr _weaponMesh;

  Md2Object _playerObject;
  Md2Object _weaponObject;

  string _name;
  string _currentSkin;
  string _currentAnim;
};

#endif // __MD2PLAYER_H__
