// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2013 Elliot Glaysher
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
// -----------------------------------------------------------------------

#include "GL/glew.h"

#include "Systems/SDL/SDLColourFilter.hpp"

#include "Systems/Base/Colour.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/SDL/SDLUtils.hpp"
#include "Systems/SDL/Shaders.hpp"
#include "Systems/SDL/Texture.hpp"

SDLColourFilter::SDLColourFilter(const Rect& screen_rect)
    : screen_rect_(screen_rect),
      texture_width_(0),
      texture_height_(0),
      back_texture_id_(0) {
}

SDLColourFilter::~SDLColourFilter() {
  if (back_texture_id_)
    glDeleteTextures(1, &back_texture_id_);
}

void SDLColourFilter::Fill(const GraphicsObject& go,
                           const RGBAColour& colour) {
  if (GLEW_ARB_fragment_shader && GLEW_ARB_multitexture) {
    if (back_texture_id_ == 0) {
      glGenTextures(1, &back_texture_id_);
      glBindTexture(GL_TEXTURE_2D, back_texture_id_);
      DebugShowGLErrors();
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

      texture_width_ = SafeSize(screen_rect_.width());
      texture_height_ = SafeSize(screen_rect_.height());
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                   texture_width_, texture_height_,
                   0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
      DebugShowGLErrors();
    }

    // Copy the current value of the region where we're going to render
    // to a texture for input to the shader
    glBindTexture(GL_TEXTURE_2D, back_texture_id_);
    int ystart = int(Texture::ScreenHeight() - screen_rect_.y() -
                     screen_rect_.height());
    int idx1 = screen_rect_.x();
    glCopyTexSubImage2D(GL_TEXTURE_2D,
                        0,
                        0, 0,
                        idx1, ystart, texture_width_, texture_height_);
    DebugShowGLErrors();

    // Set up shader
    glUseProgramObjectARB(Shaders::getObjectProgram());
    glUniform1iARB(Shaders::getObjectUniformImage(), 0);
    // The alpha is 255 because we are replacing the image on the screen
    // instead of blending with it.
    glUniform1fARB(Shaders::getObjectUniformAlpha(), 1.0f);
    Shaders::loadObjectUniformFromGraphicsObject(go);

    float thisx1 = 0;
    float thisy1 = 0;
    float thisx2 = float(screen_rect_.width()) / texture_width_;
    float thisy2 = float(screen_rect_.height()) / texture_height_;

    // Don't blend with the background. Blit on top of it.
    glDisable(GL_BLEND);

    glBegin(GL_QUADS);
    {
      glTexCoord2f(thisx1, thisy2);
      glVertex2f(screen_rect_.x(), screen_rect_.y());
      glTexCoord2f(thisx1, thisy1);
      glVertex2f(screen_rect_.x(), screen_rect_.y() + screen_rect_.height());
      glTexCoord2f(thisx2, thisy1);
      glVertex2f(screen_rect_.x() + screen_rect_.width(),
                 screen_rect_.y() + screen_rect_.height());
      glTexCoord2f(thisx2, thisy2);
      glVertex2f(screen_rect_.x() + screen_rect_.width(), screen_rect_.y());
    }
    glEnd();

    glEnable(GL_BLEND);
    glUseProgramObjectARB(0);
  }
}

