// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#include "Systems/Base/SelectionElement.hpp"

#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/SoundSystem.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/Rect.hpp"

using boost::shared_ptr;

// -----------------------------------------------------------------------
// SelectionElement
// -----------------------------------------------------------------------
SelectionElement::SelectionElement(
    System& system,
    const boost::shared_ptr<Surface>& normal_image,
    const boost::shared_ptr<Surface>& highlighted_image,
    const boost::function<void(int)>& selection_callback,
    int id, const Point& pos)
    : is_highlighted_(false), id_(id), pos_(pos),
      normal_image_(normal_image),
      highlighted_image_(highlighted_image),
      selection_callback_(selection_callback),
      system_(system) {
}

SelectionElement::~SelectionElement() {
}

void SelectionElement::setSelectionCallback(
  const boost::function<void(int)>& func) {
  selection_callback_ = func;
}

bool SelectionElement::isHighlighted(const Point& p) {
  return Rect(pos_, normal_image_->size()).contains(p);
}

void SelectionElement::setMousePosition(const Point& pos) {
  bool start_value = is_highlighted_;
  is_highlighted_ = isHighlighted(pos);

  if (start_value != is_highlighted_) {
    system_.graphics().markScreenAsDirty(GUT_TEXTSYS);
    if (is_highlighted_)
      system_.sound().playSe(0);
  }
}

bool SelectionElement::handleMouseClick(const Point& pos, bool pressed) {
  if (pressed == false && isHighlighted(pos)) {
    // Released within the button
    if (selection_callback_)
      selection_callback_(id_);

    return true;
  } else {
    return false;
  }
}

void SelectionElement::render() {
  shared_ptr<Surface> target;

  if (is_highlighted_)
    target = highlighted_image_;
  else
    target = normal_image_;

  Size s = target->size();

  target->renderToScreen(Rect(Point(0, 0), s), Rect(pos_, s), 255);
}
