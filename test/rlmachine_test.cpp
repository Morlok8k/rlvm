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

#include "gtest/gtest.h"

#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <utility>
#include <string>
#include <vector>

#include "MachineBase/Memory.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/Serialization.hpp"
#include "Modules/Module_Str.hpp"
#include "Utilities/Exception.hpp"
#include "libReallive/intmemref.h"
#include "testUtils.hpp"

using namespace std;
using namespace libReallive;
using boost::lexical_cast;
using boost::assign::list_of;

class RLMachineTest : public FullSystemTest {
 protected:
  void setIntMemoryCountingFrom(RLMachine& saveMachine,
                                const vector<pair<int, char> >& banks,
                                int count) {
    for (vector<pair<int, char> >::const_iterator it = banks.begin();
         it != banks.end(); ++it) {
      for (int i = 0; i < SIZE_OF_MEM_BANK; ++i) {
        saveMachine.setIntValue(IntMemRef(it->second, i), count);
        count++;
      }
    }
  }

  void setStrMemoryCountingFrom(RLMachine& saveMachine, int type, int count) {
    for (int i = 0; i < SIZE_OF_MEM_BANK; ++i) {
      saveMachine.setStringValue(type, i, lexical_cast<string>(count));
      count++;
    }
  }

  void verifyIntMemoryCountingFrom(RLMachine& loadMachine,
                                   const vector<pair<int, char> >& banks,
                                   int count) {
    for (vector<pair<int, char> >::const_iterator it = banks.begin();
         it != banks.end(); ++it) {
      for (int i = 0; i < SIZE_OF_MEM_BANK; ++i) {
        EXPECT_EQ(count, loadMachine.getIntValue(IntMemRef(it->second, i)));
        count++;
      }
    }
  }

  void verifyStrMemoryCountingFrom(RLMachine& loadMachine,
                                   int type,
                                   int count) {
    for (int i = 0; i < SIZE_OF_MEM_BANK; ++i) {
      EXPECT_EQ(lexical_cast<string>(count),
                loadMachine.getStringValue(type, i));
      count++;
    }
  }
};

TEST_F(RLMachineTest, RejectsDoubleAttachs) {
  rlmachine.attachModule(new StrModule);
  EXPECT_THROW({rlmachine.attachModule(new StrModule); },
               rlvm::Exception);
}

TEST_F(RLMachineTest, ReturnFromFarcallMismatch) {
  EXPECT_THROW({rlmachine.returnFromFarcall(); },
               rlvm::Exception);
}

TEST_F(RLMachineTest, ReturnFromGosubMismatch) {
  EXPECT_THROW({rlmachine.returnFromGosub(); },
               rlvm::Exception);
}

TEST_F(RLMachineTest, Halts) {
  EXPECT_TRUE(!rlmachine.halted()) << "Machine does not start halted.";
  rlmachine.halt();
  EXPECT_TRUE(rlmachine.halted()) << "Machine is halted.";
}

TEST_F(RLMachineTest, RegisterStore) {
  for (int i = 0; i < 10; ++i) {
    rlmachine.setStoreRegister(i);
    EXPECT_EQ(i, rlmachine.getStoreRegisterValue());
  }
}

// Test valid string memory access.
TEST_F(RLMachineTest, StringMemory) {
  vector<int> types = list_of(STRK_LOCATION)(STRM_LOCATION)(STRS_LOCATION);

  for (vector<int>::const_iterator it = types.begin(); it != types.end();
       ++it) {
    const string str = "Stored at " + lexical_cast<string>(*it);
    rlmachine.setStringValue(*it, 0, str);
    EXPECT_EQ(str, rlmachine.getStringValue(*it, 0));
  }
}

// Test error-inducing, string memory access.
TEST_F(RLMachineTest, StringMemoryErrors) {
  EXPECT_THROW({rlmachine.setStringValue(STRK_LOCATION, 3, "Blah");},
               rlvm::Exception);
  EXPECT_THROW({rlmachine.setStringValue(STRM_LOCATION, 2000, "Blah");},
               rlvm::Exception);
  EXPECT_THROW({rlmachine.getStringValue(STRK_LOCATION, 3);},
               rlvm::Exception);
  EXPECT_THROW({rlmachine.getStringValue(STRM_LOCATION, 2000);},
               rlvm::Exception);
}

// Test valid integer access of all types.
//
// For reference to understand the following
// signed 32-bit integer: 10281 (0010 1000 0010 1001b)
//        8-bit integers: 0,0,40,41
//
// For    8-bit integers: 38,39,40,41
TEST_F(RLMachineTest, IntegerMemory) {
  vector<char> banks = list_of('A')('B')('C')('D')('E')('F')('G')('L')('Z');

  const int in8b[] = {38,  39, 40, 41};
  const int base = (in8b[0] << 24) | (in8b[1] << 16) | (in8b[2] << 8) | in8b[3];

  const int rc = 8;
  const int final = (rc << 24) | (rc << 16) | (rc << 8) | rc;

  for (vector<char>::const_iterator it = banks.begin(); it != banks.end();
       ++it) {
    IntMemRef wordRef(*it, 0);
    rlmachine.setIntValue(wordRef, base);
    EXPECT_EQ(base, rlmachine.getIntValue(wordRef))
        << "Didn't record full value";

    for (int i = 0; i < 4; ++i) {
      IntMemRef comp(*it, "8b", i);
      EXPECT_EQ(in8b[3 - i], rlmachine.getIntValue(comp))
          << "Could get partial value";

      rlmachine.setIntValue(comp, rc);
    }

    EXPECT_EQ(final, rlmachine.getIntValue(wordRef))
        << "Changing the components didn't change the full value!";
  }
}

TEST_F(RLMachineTest, IntegerMemoryErrors) {
  EXPECT_THROW({rlmachine.getIntValue(IntMemRef(10, 0, 0));},
               rlvm::Exception);
  EXPECT_NO_THROW({rlmachine.getIntValue(IntMemRef('A', 1999));});  // NOLINT
  EXPECT_THROW({rlmachine.getIntValue(IntMemRef('A', 2000));},
               rlvm::Exception);
}

TEST_F(RLMachineTest, CheckNameLetterIndex) {
  EXPECT_EQ(0, Memory::ConvertLetterIndexToInt("A"));
  EXPECT_EQ(25, Memory::ConvertLetterIndexToInt("Z"));
  EXPECT_EQ(26, Memory::ConvertLetterIndexToInt("AA"));
  EXPECT_EQ(52, Memory::ConvertLetterIndexToInt("BA"));
  EXPECT_EQ(701, Memory::ConvertLetterIndexToInt("ZZ"));
}

TEST_F(RLMachineTest, NameStorage) {
  Memory& memory = rlmachine.memory();
  EXPECT_EQ("Bob", memory.getName(Memory::ConvertLetterIndexToInt("A")));
  EXPECT_EQ("Alice",
            memory.getLocalName(Memory::ConvertLetterIndexToInt("AB")));
}

TEST_F(RLMachineTest, Serialization) {
  stringstream ss;
  libReallive::Archive arc(locateTestCase("Module_Str_SEEN/strcpy_0.TXT"));
  // Save data
  {
    RLMachine saveMachine(system, arc);
    setIntMemoryCountingFrom(saveMachine, GLOBAL_INTEGER_BANKS, 0);
    setStrMemoryCountingFrom(saveMachine, STRM_LOCATION, 0);

    Serialization::saveGlobalMemoryTo(ss, saveMachine);
  }

  // Load data
  {
    RLMachine loadMachine(system, arc);
    Serialization::loadGlobalMemoryFrom(ss, loadMachine);
    verifyIntMemoryCountingFrom(loadMachine, GLOBAL_INTEGER_BANKS, 0);
    verifyStrMemoryCountingFrom(loadMachine, STRM_LOCATION, 0);
  }
}

// Tests serialization of the kidoku table.
TEST_F(RLMachineTest, SerializationOfKidoku) {
  stringstream ss;
  libReallive::Archive arc(locateTestCase("Module_Str_SEEN/strcpy_0.TXT"));

  // Save data
  {
    RLMachine saveMachine(system, arc);

    for (int i = 0; i < 10; i += 2) {
      saveMachine.memory().recordKidoku(5, i);
    }

    Serialization::saveGlobalMemoryTo(ss, saveMachine);
  }

  // Load data
  {
    RLMachine loadMachine(system, arc);
    Serialization::loadGlobalMemoryFrom(ss, loadMachine);

    for (int i = 0; i < 10; i++) {
      EXPECT_EQ(!(i % 2), loadMachine.memory().hasBeenRead(5, i))
          << "Didn't save kidoku table correctly!";
    }
  }
}

TEST_F(RLMachineTest, SerializationOfSavepointValues) {
  stringstream ss;
  libReallive::Archive arc(locateTestCase("Module_Str_SEEN/strcpy_0.TXT"));
  // Save data
  {
    RLMachine saveMachine(system, arc);

    // Write the values we're going to check for.
    setIntMemoryCountingFrom(saveMachine, LOCAL_INTEGER_BANKS, 0);
    setStrMemoryCountingFrom(saveMachine, STRS_LOCATION, 0);
    saveMachine.markSavepoint();

    // Verify that those values are written.
    verifyIntMemoryCountingFrom(saveMachine, LOCAL_INTEGER_BANKS, 0);
    verifyStrMemoryCountingFrom(saveMachine, STRS_LOCATION, 0);

    // Scribble different values on top, immediately check to make sure we can
    // still read them, but don't commit them.
    setIntMemoryCountingFrom(saveMachine, LOCAL_INTEGER_BANKS, 5);
    setStrMemoryCountingFrom(saveMachine, STRS_LOCATION, 5);
    verifyIntMemoryCountingFrom(saveMachine, LOCAL_INTEGER_BANKS, 5);
    verifyStrMemoryCountingFrom(saveMachine, STRS_LOCATION, 5);

    Serialization::saveGameTo(ss, saveMachine);
  }

  // Load data. Assure that we only have the committed values.
  {
    RLMachine loadMachine(system, arc);
    Serialization::loadGameFrom(ss, loadMachine);
    verifyIntMemoryCountingFrom(loadMachine, LOCAL_INTEGER_BANKS, 0);
    verifyStrMemoryCountingFrom(loadMachine, STRS_LOCATION, 0);
  }
}
