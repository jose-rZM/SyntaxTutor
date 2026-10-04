BUILD_DIR   = build
APP_BUILD   = $(BUILD_DIR)/app
TEST_BUILD  = $(BUILD_DIR)/tests
CORE_BUILD  = $(BUILD_DIR)/core
QMAKE       = qmake6
JOBS        = $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
CXX        ?= g++

# Locate gtest via pkg-config, then common brew/system paths
GTEST_CFLAGS := $(shell pkg-config --cflags gtest 2>/dev/null)
GTEST_LIBS   := $(shell pkg-config --libs   gtest gtest_main 2>/dev/null)
ifeq ($(GTEST_LIBS),)
  GTEST_PREFIX := $(shell brew --prefix googletest 2>/dev/null)
  ifneq ($(GTEST_PREFIX),)
    GTEST_CFLAGS := -I$(GTEST_PREFIX)/include
    GTEST_LIBS   := -L$(GTEST_PREFIX)/lib -lgtest -lgtest_main
  else
    GTEST_CFLAGS := -I/usr/include
    GTEST_LIBS   := -lgtest -lgtest_main -lpthread
  endif
endif

BACKEND_SRCS = \
    src/backend/grammar.cpp \
    src/backend/grammar_factory.cpp \
    src/backend/grammar_parser.cpp \
    src/backend/ll1_parser.cpp \
    src/backend/lr0_item.cpp \
    src/backend/slr1_parser.cpp \
    src/backend/symbol_table.cpp \
    src/backend/tests.cpp

.PHONY: all app tests core check check-core check-ui clean

all: app tests core

app:
	mkdir -p $(APP_BUILD)
	cd $(APP_BUILD) && $(QMAKE) $(CURDIR)/SyntaxTutor.pro CONFIG+=release
	$(MAKE) -C $(APP_BUILD) -j$(JOBS)

tests:
	mkdir -p $(TEST_BUILD)
	cd $(TEST_BUILD) && $(QMAKE) $(CURDIR)/tests/tests.pro
	$(MAKE) -C $(TEST_BUILD) -j$(JOBS)

core:
	mkdir -p $(CORE_BUILD)
	$(CXX) -std=gnu++2a -O2 -Isrc/backend \
	    $(GTEST_CFLAGS) \
	    $(BACKEND_SRCS) \
	    $(GTEST_LIBS) \
	    -o $(CORE_BUILD)/core_tests

check: check-core check-ui

check-core: core
	$(CORE_BUILD)/core_tests

check-ui: tests
	QT_QPA_PLATFORM=offscreen $(TEST_BUILD)/.bin/tutor_tests

clean:
	rm -rf $(BUILD_DIR)
