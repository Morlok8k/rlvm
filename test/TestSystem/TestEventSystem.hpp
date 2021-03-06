// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2008 Elliot Glaysher
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
//
// -----------------------------------------------------------------------

#ifndef TEST_TESTSYSTEM_TESTEVENTSYSTEM_HPP_
#define TEST_TESTSYSTEM_TESTEVENTSYSTEM_HPP_

#include "Systems/Base/EventSystem.hpp"

#include <boost/shared_ptr.hpp>

// Provides behaviour for the TestEventSystem.
class EventSystemMockHandler {
 public:
  EventSystemMockHandler() : counter_(0) {}
  virtual ~EventSystemMockHandler() {}

  virtual bool shiftPressed() const { return false; }
  virtual bool ctrlPressed() const { return false; }
  virtual unsigned int getTicks() const { return counter_++; }

 private:
  mutable int counter_;
};

// Mock enabled event system. Returned values are controlled by
// EventSystemMockHandler.
class TestEventSystem : public EventSystem {
 public:
  explicit TestEventSystem(Gameexe& gexe);
  void setMockHandler(const boost::shared_ptr<EventSystemMockHandler>& handler);

  // Implementation of EventSystem:
  virtual void executeEventSystem(RLMachine& machine);
  virtual unsigned int getTicks() const;
  virtual void wait(unsigned int milliseconds) const;
  virtual bool shiftPressed() const;
  virtual bool ctrlPressed() const;
  virtual Point getCursorPos() { return Point(0, 0); }
  virtual void getCursorPos(Point& position, int& button1, int& button2) {}
  virtual void flushMouseClicks() {}
  virtual unsigned int timeOfLastMouseMove() { return 0; }
  virtual void injectMouseMovement(RLMachine& machine, const Point& loc) {}
  virtual void injectMouseDown(RLMachine& machine) {}
  virtual void injectMouseUp(RLMachine& machine) {}

 private:
  // Defines test specific behaviour for the TestEventSystem
  boost::shared_ptr<EventSystemMockHandler> event_system_mock_;
};

#endif  // TEST_TESTSYSTEM_TESTEVENTSYSTEM_HPP_
