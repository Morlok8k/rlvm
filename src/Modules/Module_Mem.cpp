// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
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

#include <vector>

#include "Modules/Module_Mem.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLOperation/Argc_T.hpp"
#include "MachineBase/RLOperation/Complex_T.hpp"
#include "MachineBase/RLOperation/RLOp_Store.hpp"
#include "MachineBase/RLOperation/References.hpp"

#include <cmath>
#include <algorithm>
#include <numeric>

// For copy_n, which isn't part of the C++ standard and doesn't come on
// OSX.
#include <boost/multi_array/algorithm.hpp>

using namespace std;
using namespace boost;

// -----------------------------------------------------------------------

namespace {

// Implement op<1:Mem:00000, 0>, fun setarray(int, intC+).
//
// Sets a block of integers, starting with origin, to the given values. values
// is an arbitrary number of integer expressions, each of which is assigned in
// turn to the next variable.
struct setarray : public RLOp_Void_2<IntReference_T,
                                         Argc_T<IntConstant_T> > {
  void operator()(RLMachine& machine, IntReferenceIterator origin,
                  vector<int> values) {
    copy(values.begin(), values.end(), origin);
  }
};

// Implement op<1:Mem:00001, 0>, fun setrng(int, int).
//
// Set block of integers to zero.
struct setrng_0 : public RLOp_Void_2< IntReference_T, IntReference_T > {
  void operator()(RLMachine& machine, IntReferenceIterator first,
                  IntReferenceIterator last) {
    ++last;  // RealLive ranges are inclusive
    fill(first, last, 0);
  }
};

// Implement op<1:Mem:00001, 1>, fun setrng(int, int, intC).
//
// Set block of integers to the constant passed in.
struct setrng_1 : public RLOp_Void_3< IntReference_T, IntReference_T,
                                          IntConstant_T > {
  void operator()(RLMachine& machine, IntReferenceIterator first,
                  IntReferenceIterator last, int value) {
    ++last;  // RealLive ranges are inclusive
    fill(first, last, value);
  }
};

// Implement op<1:Mem:00002, 0>, fun(int, intC, intC).
//
// Copies a block of values of length count from source to dest. The
// function appears to succeed even if the ranges overlap.
//
// @note copy_n is not part of the C++ standard, and while it's part of
// STL on the machines at work, it doesn't exist on OSX's implementation,
// so grab a copy that boost includes.
struct cpyrng : public RLOp_Void_3< IntReference_T, IntReference_T,
                                        IntConstant_T > {
  void operator()(RLMachine& machine, IntReferenceIterator source,
                  IntReferenceIterator dest, int count) {
    vector<int> tmpCopy;
    boost::detail::multi_array::copy_n(source, count, back_inserter(tmpCopy));
    std::copy(tmpCopy.begin(), tmpCopy.end(), dest);
  }
};

// Implement op<1:Mem:00003, 0>, fun setarray_stepped(int, int, intC+).
//
// Sets every stepth memory block starting at origin with the sequence of
// passed in values.
struct setarray_stepped
  : public RLOp_Void_3< IntReference_T, IntConstant_T,
                        Argc_T<IntConstant_T > > {
  void operator()(RLMachine& machine, IntReferenceIterator origin,
                  int step, vector<int> values) {
    // Sigh. No more simple STL statements
    for (vector<int>::iterator it = values.begin(); it != values.end(); ++it) {
      *origin = *it;
      advance(origin, step);
    }
  }
};

// Implement op<1:Mem:00004, 0>, fun setrng_stepped(int, intC, intC).
//
// Sets count number of memory locations to zero, starting at origin and going
// forward step.
struct setrng_stepped_0
  : public RLOp_Void_3< IntReference_T, IntConstant_T, IntConstant_T > {
  void operator()(RLMachine& machine, IntReferenceIterator origin,
                  int step, int count) {
    for (int i = 0; i < count; ++i) {
      *origin = 0;
      advance(origin, step);
    }
  }
};

// Implement op<1:Mem:00004, 1>, fun setrng_stepped(int, intC, intC, intC).
//
// Sets count number of memory locations to the passed in constant, starting at
// origin and going forward step.
struct setrng_stepped_1
  : public RLOp_Void_4< IntReference_T, IntConstant_T, IntConstant_T,
                        IntConstant_T > {
  void operator()(RLMachine& machine, IntReferenceIterator origin,
                  int step, int count, int value) {
    for (int i = 0; i < count; ++i) {
      *origin = value;
      advance(origin, step);
    }
  }
};

// Implement op<1:Mem:00006, 0>, fun cpyvars(int, intC, int+).
//
// I'm not even going to try for this one. See RLDev.
struct cpyvars : public RLOp_Void_3< IntReference_T, IntConstant_T,
                                         Argc_T< IntReference_T > > {
  void operator()(RLMachine& machine, IntReferenceIterator origin,
                  int offset, vector<IntReferenceIterator> values) {
    for (vector<IntReferenceIterator>::iterator it = values.begin();
        it != values.end(); ++it) {
      IntReferenceIterator irIt = *it;
      advance(irIt, offset);
      *origin++ = *irIt;
    }
  }
};

// Implement op<1:Mem:00100, 0>, fun sum(int, int).
//
// Returns the sum of all the numbers in the given memory range.
struct sum : public RLOp_Store_2< IntReference_T, IntReference_T > {
  int operator()(RLMachine& machine, IntReferenceIterator first,
                  IntReferenceIterator last) {
    last++;
    return accumulate(first, last, 0);
  }
};

// Implement op<1:Mem:00101, 0>, fun sums((int, int)+).
//
// Returns the sum of all the numbers in all the given memory ranges.
struct sums : public RLOp_Store_1< Argc_T< Complex2_T< IntReference_T,
                                                         IntReference_T > > > {
  int operator()(
      RLMachine& machine,
      vector<tuple<IntReferenceIterator, IntReferenceIterator> > ranges) {
    int total = 0;
    for (vector<tuple<IntReferenceIterator, IntReferenceIterator> >::iterator
             it = ranges.begin(); it != ranges.end(); ++it) {
      IntReferenceIterator last = it->get<1>();
      ++last;
      total += accumulate(it->get<0>(), last, 0);
    }
    return total;
  }
};

}  // namespace

// -----------------------------------------------------------------------

MemModule::MemModule()
  : RLModule("Mem", 1, 11) {
  addOpcode(0, 0, "setarray", new setarray);
  addOpcode(1, 0, "setrng", new setrng_0);
  addOpcode(1, 1, "setrng", new setrng_1);
  addOpcode(2, 0, "cpyrng", new cpyrng);
  addOpcode(3, 0, "setarray_stepped", new setarray_stepped);
  addOpcode(4, 0, "setrng_stepped", new setrng_stepped_0);
  addOpcode(4, 1, "setrng_stepped", new setrng_stepped_1);
  // implement op<1:Mem:00005, 0>
  addOpcode(6, 0, "cpyvars", new cpyvars);
  addOpcode(100, 0, "sum", new sum);
  addOpcode(101, 0, "sums", new sums);
}
