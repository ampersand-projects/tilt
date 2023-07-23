#ifndef PYTHON_EXEC_INCLUDE_PYENG_H_
#define PYTHON_EXEC_INCLUDE_PYENG_H_

#include <string>
#include <vector>

#include "pyreg.h"

#include "tilt/base/ctype.h"
#include "tilt/engine/engine.h"
#include "tilt/ir/op.h"

class PyEng {
public:
    PyEng(void);

    intptr_t compile(tilt::Op query_op, std::string query_name);
    void execute(intptr_t addr, ts_t t_start, ts_t t_end,
                 PyReg* out_reg, std::vector<PyReg*> in_reg_vec);

private:
    tilt::ExecEngine* jit;
};

#endif  // PYTHON_EXEC_INCLUDE_PYENG_H_
