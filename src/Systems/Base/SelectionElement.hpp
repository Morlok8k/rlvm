// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//  
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//  
// -----------------------------------------------------------------------

#ifndef __SelectionElement_hpp__
#define __SelectionElement_hpp__

#include <boost/shared_ptr.hpp>

class RLMachine;
class Surface;

/**
 * Represents a clickable 
 *
 * @todo Later on, see if I can factor out common code from 
 *       @c TextWindowButton.
 */
class SelectionElement
{
private:
  bool m_isHighlighted;

  int m_id;

  // Location of the button
  int m_x, m_y;

  boost::shared_ptr<Surface> m_normalImage;
  boost::shared_ptr<Surface> m_highlightedImage;

  /// Callback function for when item is selected.
  boost::function<void(int)> m_selectionCallback;

  bool isHighlighted(int x, int y);

public:
  SelectionElement(const boost::shared_ptr<Surface>& normalImage, 
                   const boost::shared_ptr<Surface>& highlightedImage,
                   const boost::function<void(int)>& selectionCallback,
                   int id, int x, int y);
  ~SelectionElement();

  void setSelectionCallback(const boost::function<void(int)>& func);

  void setMousePosition(RLMachine& machine, int x, int y);
  bool handleMouseClick(RLMachine& machine, int x, int y, bool pressed);

  void render();
};

#endif
