#include "tilt/ir/expr.h"
#include "tilt/codegen/printer.h"

#include "gtest/gtest.h"

#include <memory>
#include <string>

using namespace tilt;
using namespace std;

TEST(BasicTests, BasicAssertions) {
	auto sym = make_shared<Symbol>("test", Type(types::INT32, FreeIter("test")));
	auto str = IRPrinter::Build(sym);
	ASSERT_STREQ(str.c_str(), "~test");
}

