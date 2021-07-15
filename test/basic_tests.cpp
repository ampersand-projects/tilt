#include "tilt/builder/tilder.h"
#include "tilt/codegen/printer.h"

#include "gtest/gtest.h"

#include <memory>
#include <string>

using namespace tilt;
using namespace tilt::tilder;
using namespace std;

TEST(BasicTests, BasicAssertions) {
	auto sym = _sym("test", Type(types::INT32, _iter("test")));
	auto str = IRPrinter::Build(sym);
	ASSERT_STREQ(str.c_str(), "~test");
}