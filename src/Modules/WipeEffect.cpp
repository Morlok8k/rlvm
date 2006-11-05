// This file is part of RLVM, a RealLive virutal machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 El Riot
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

/**
 * @file   WipeEffect.cpp
 * @author Elliot Glaysher
 * @date   Thu Nov  2 20:33:21 2006
 * 
 * @brief  Implements #SEL transition style #10, Wipe.
 * 
 * 
 */


#include "WipeEffect.hpp"
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"

#include <iostream>
#include <cmath>

using namespace std;

// -----------------------------------------------------------------------
// WipeEffect base class
// -----------------------------------------------------------------------

bool WipeEffect::blitOriginalImage() const
{
  return true; 
}

// -----------------------------------------------------------------------

/** 
 * Calculates the size of the interpolation and main polygons.
 *
 * There are 3 possible stages:
 * - [0, m_interpolationInPixels) - Draw only the
 *   transition. (sizeOfMainPolygon == 0, sizeOfInterpolation ==
 *   amountVisible)
 * - [m_interpolationInPixels, sizeOfScreen) - Draw both
 *   polygons. (sizeOfMainPolygon == amountVisible -
 *   m_interpolationInPixels, sizeOfInterpolation == amountVisible)
 * - [height, height + m_interpolationInPixels) - Draw both
 *   polygons, flooring the height of the transition to 
 * 
 * @param sizeOfInterpolation 
 * @param sizeOfMainPolygon 
 * @param sizeOfScreen Size of the area we are rendering to. Name is
 *                     intentionally ambigous so we can apply this to
 *                     both width and height.
 */
void WipeEffect::calculateSizes(int currentTime,
                                int& sizeOfInterpolation, 
                                int& sizeOfMainPolygon,
                                int sizeOfScreen)
{
  int amountVisible = (float(currentTime) / duration()) * 
    (sizeOfScreen + m_interpolationInPixels);
  if(amountVisible < m_interpolationInPixels)
  {
    sizeOfInterpolation = amountVisible;
    sizeOfMainPolygon = 0;
  }
  else if(amountVisible < sizeOfScreen)
  {
    sizeOfInterpolation = m_interpolationInPixels;
    sizeOfMainPolygon = amountVisible - m_interpolationInPixels;
  }
  else if(amountVisible < sizeOfScreen + m_interpolationInPixels)
  {
    sizeOfMainPolygon = amountVisible - m_interpolationInPixels;
    sizeOfInterpolation = sizeOfScreen - sizeOfMainPolygon;
  }
}

// -----------------------------------------------------------------------

WipeEffect::WipeEffect(RLMachine& machine, int x, int y, int width, 
                       int height, int dx, int dy, int time, 
                       int interpolation)
  : Effect(machine, x, y, width, height, dx, dy, time), 
    m_interpolation(interpolation),
    m_interpolationInPixels(0)
{
  if(m_interpolation)
    m_interpolationInPixels = pow(float(2), interpolation) * 2.5;
}

// -----------------------------------------------------------------------
// WipeTopToBottomEffect
// -----------------------------------------------------------------------

WipeTopToBottomEffect::WipeTopToBottomEffect(
  RLMachine& machine, int x, int y, int width, int height, int dx, 
  int dy, int time, int interpolation)
  : WipeEffect(machine, x, y, width, height, dx, dy, time, interpolation)
{}

// -----------------------------------------------------------------------

void WipeTopToBottomEffect::performEffectForTime(RLMachine& machine, 
                                                 int currentTime)
{
  GraphicsSystem& graphics = machine.system().graphics();
  int sizeOfInterpolation, sizeOfMainPolygon;
  calculateSizes(currentTime, sizeOfInterpolation, sizeOfMainPolygon, height());

  if(sizeOfMainPolygon)
  {
    graphics.getDC(1).
      renderToScreen(x(), y() + height() - sizeOfMainPolygon - sizeOfInterpolation, 
                     x() + width(), y() + height() - sizeOfInterpolation, 
                     dx(), dy(), dx() + width(), dy() + sizeOfMainPolygon,
                     255);
  }

  if(sizeOfInterpolation)
  {
    int opacity[4] = {255, 255, 0, 0};

    graphics.getDC(1).
      renderToScreen(x(), y() + height() - sizeOfInterpolation, 
                     x() + width(), y() + height(), 
                     dx(), dy() + sizeOfMainPolygon, dx() + width(), 
                     dy() + sizeOfMainPolygon + sizeOfInterpolation,
                     opacity);    
  }
}

// -----------------------------------------------------------------------
// WipeBottomToTopEffect
// -----------------------------------------------------------------------

WipeBottomToTopEffect::WipeBottomToTopEffect(
  RLMachine& machine, int x, int y, int width, int height, int dx, 
  int dy, int time, int interpolation)
  : WipeEffect(machine, x, y, width, height, dx, dy, time,
               interpolation)
{}

// -----------------------------------------------------------------------

void WipeBottomToTopEffect::performEffectForTime(RLMachine& machine,
                                                 int currentTime)
{
  GraphicsSystem& graphics = machine.system().graphics();

  int sizeOfInterpolation, sizeOfMainPolygon;
  calculateSizes(currentTime, sizeOfInterpolation, 
                 sizeOfMainPolygon, height());

  // Render the sliding on frame
  if(sizeOfMainPolygon)
  {
    graphics.getDC(1).
      renderToScreen(x(), y() + sizeOfInterpolation, 
                     x() + width(), y() + sizeOfInterpolation + sizeOfMainPolygon, 
                     dx(), dy() + height() - sizeOfMainPolygon, 
                     dx() + width(), dy() + height(),
                     255);
  }

  if(sizeOfInterpolation)
  {
    int opacity[4] = {0, 0, 255, 255};
    graphics.getDC(1).
      renderToScreen(x(), y(),
                     x() + width(), y() + sizeOfInterpolation,
                     dx(), dy() + height() - sizeOfMainPolygon - sizeOfInterpolation,
                     dx() + width(), dy() + height() - sizeOfMainPolygon,
                     opacity);
  }
}

// -----------------------------------------------------------------------
// WipeFromLeftToRightEffect
// -----------------------------------------------------------------------

WipeLeftToRightEffect::WipeLeftToRightEffect(
  RLMachine& machine, int x, int y, int width, int height, int dx, 
  int dy, int time, int interpolation)
  : WipeEffect(machine, x,y, width, height, dx, dy, time, 
               interpolation)
{}

// -----------------------------------------------------------------------

void WipeLeftToRightEffect::performEffectForTime(RLMachine& machine,
                                                  int currentTime)
{
  GraphicsSystem& graphics = machine.system().graphics();
  int sizeOfInterpolation, sizeOfMainPolygon;
  calculateSizes(currentTime, sizeOfInterpolation, sizeOfMainPolygon, width());

  if(sizeOfMainPolygon)
  {
    graphics.getDC(1).
      renderToScreen(x() + width() - sizeOfInterpolation - sizeOfMainPolygon, y(), 
                     x() + width() - sizeOfInterpolation, y() + height(), 
                     dx(), dy(), dx() + sizeOfMainPolygon, dy() + height(),
                     255);
  }

  if(sizeOfInterpolation)
  {
    int opacity[4] = {255, 0, 0, 255};
    graphics.getDC(1).
      renderToScreen(x() + width() - sizeOfInterpolation, y(), 
                     x() + width(), y() + height(), 
                     dx() + sizeOfMainPolygon, dy(), 
                     dx() + sizeOfMainPolygon + sizeOfInterpolation, dy() + height(),
                     opacity);
  }
}

// -----------------------------------------------------------------------
// WipeFromRightToLeftEffect
// -----------------------------------------------------------------------

WipeRightToLeftEffect::WipeRightToLeftEffect(
  RLMachine& machine, int x, int y, int width, int height, int dx, 
  int dy, int time, int interpolation)
  : WipeEffect(machine, x, y, width, height, dx, dy, time, 
               interpolation)
{}

// -----------------------------------------------------------------------

void WipeRightToLeftEffect::performEffectForTime(RLMachine& machine,
                                                 int currentTime)
{
  GraphicsSystem& graphics = machine.system().graphics();
  int sizeOfInterpolation, sizeOfMainPolygon;
  calculateSizes(currentTime, sizeOfInterpolation, sizeOfMainPolygon, width());

  if(sizeOfMainPolygon)
  {
    // broken
    graphics.getDC(1).
      renderToScreen(x() + sizeOfInterpolation, y(), 
                     x() + sizeOfInterpolation + sizeOfMainPolygon, y() + height(), 
                     dx() + width() - sizeOfMainPolygon, dy(),
                     dx() + width(), dy() + height(),
                     255);
  }

  if(sizeOfInterpolation)
  {
    int opacity[4] = {0, 255, 255, 0};
    graphics.getDC(1).
      renderToScreen(x(), y(), 
                     x() + sizeOfInterpolation, y() + height(), 
                     dx() + width() - sizeOfInterpolation - sizeOfMainPolygon, dy(),
                     dx() + width() - sizeOfMainPolygon, dy() + height(),
                     opacity);
  }
}
