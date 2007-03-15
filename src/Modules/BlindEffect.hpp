// This file is part of RLVM, a RealLive virtual machine clone.
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
 * @file   BlindEffect.hpp
 * @author Elliot Glaysher
 * @date   Tue Feb 27 21:04:47 2007
 * 
 * @brief  Implements \#SEL transition style \#10, Blind.
 */

#ifndef __BlindEffect_hpp__
#define __BlindEffect_hpp__

#include "Modules/Effect.hpp"

/**
 * @ingroup TransitionEffects
 * 
 * @{
 */

class BlindEffect : public Effect
{
private:
  int m_blindSize;

  virtual bool blitOriginalImage() const;

protected:
  // We'll probably need some utility stuff here.

  const int blindSize() const { return m_blindSize; }

public:
  BlindEffect(RLMachine& machine, boost::shared_ptr<Surface> src,
             boost::shared_ptr<Surface> dst, 
             int width, int height, int time, int blindSize);

  ~BlindEffect();
};

// -----------------------------------------------------------------------

class BlindTopToBottomEffect : public BlindEffect
{
protected:
  virtual void performEffectForTime(RLMachine& machine, int currentTime);

public:
  BlindTopToBottomEffect(RLMachine& machine, boost::shared_ptr<Surface> src,
             boost::shared_ptr<Surface> dst, 
             int width, int height, int time, int blindSize);
};

// -----------------------------------------------------------------------

class BlindBottomToTopEffect : public BlindEffect
{
protected:
  virtual void performEffectForTime(RLMachine& machine, int currentTime);

public:
  BlindBottomToTopEffect(RLMachine& machine, boost::shared_ptr<Surface> src,
             boost::shared_ptr<Surface> dst, 
             int width, int height, int time, int blindSize);
};

// -----------------------------------------------------------------------

class BlindLeftToRightEffect : public BlindEffect
{
protected:
  virtual void performEffectForTime(RLMachine& machine, int currentTime);

public:
  BlindLeftToRightEffect(RLMachine& machine, boost::shared_ptr<Surface> src,
             boost::shared_ptr<Surface> dst, 
             int width, int height, int time, int blindSize);
};

// -----------------------------------------------------------------------

class BlindRightToLeftEffect : public BlindEffect
{
protected:
  virtual void performEffectForTime(RLMachine& machine, int currentTime);

public:
  BlindRightToLeftEffect(RLMachine& machine, boost::shared_ptr<Surface> src,
             boost::shared_ptr<Surface> dst, 
             int width, int height, int time, int blindSize);
};

/// @}

#endif