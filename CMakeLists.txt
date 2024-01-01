cmake_minimum_required(VERSION 3.26)
project(Kathryn)

set(CMAKE_CXX_STANDARD 17)


INCLUDE_DIRECTORIES( ./src )


add_executable(Kathryn main.cpp
        src/model/hwComponent/expression/expression.cpp
        src/model/hwComponent/expression/expression.h
        src/model/hwComponent/abstract/operation.h
        src/model/hwComponent/abstract/operable.h
        src/model/hwComponent/abstract/assignable.h
        src/model/hwComponent/abstract/identifiable.cpp
        src/model/hwComponent/abstract/identifiable.h
        src/model/hwComponent/abstract/operable.cpp
        src/model/hwComponent/register/register.cpp
        src/model/hwComponent/register/register.h
        src/model/hwComponent/wire/wire.cpp
        src/model/hwComponent/wire/wire.h
        src/model/hwComponent/value/value.cpp
        src/model/hwComponent/value/value.h
        src/model/hwComponent/module/module.cpp
        src/model/hwComponent/module/module.h
        src/model/controller/controller.cpp
        src/model/controller/controller.h
        src/model/controller/conInterf/controllerItf.cpp
        src/model/controller/conInterf/controllerItf.h
        src/model/hwComponent/abstract/makeComponent.h
        src/model/hwComponent/abstract/slicable.h
        src/model/hwComponent/abstract/Slice.h
        src/model/FlowBlock/abstract/flowBlock_Base.cpp
        src/model/FlowBlock/abstract/flowBlock_Base.h
        src/model/controller/flowController.cpp
        src/model/controller/hwCompController.cpp
        src/model/FlowBlock/par/par.cpp
        src/model/FlowBlock/par/par.h
        src/model/FlowBlock/abstract/loopStMacro.h
        src/model/FlowBlock/abstract/stateReg.h
        src/model/FlowBlock/abstract/stateReg.cpp
        src/model/FlowBlock/seq/seq.cpp
        src/model/FlowBlock/seq/seq.h
        src/model/FlowBlock/loop/cwhile.cpp
        src/model/FlowBlock/loop/cwhile.h
        src/model/FlowBlock/cond/if.cpp
        src/model/FlowBlock/cond/if.h
        src/model/FlowBlock/cond/elif.cpp
        src/model/FlowBlock/cond/elif.h
        src/application/visualizer/vis.cpp
        src/application/visualizer/vis.h
        src/model/hwComponent/abstract/hwDebuggable.h
        src/model/hwComponent/abstract/logicComp.h
        src/model/util/StateRegUtil.h
        src/test/test.cpp
        src/test/test.h
        src/test/modelTest1.cpp
        src/test/modelTest1.h
        src/model/hwComponent/abstract/operation.cpp
        src/model/util/stateRegUtil.cpp
        src/model/FlowBlock/abstract/node.cpp
        src/model/FlowBlock/abstract/node.h
        src/test/modelTest2.h
        src/util/logger/logger.cpp
        src/util/logger/logger.h
        src/model/logicRep/valRep.cpp
        src/model/logicRep/valRep.h
        src/model/FlowBlock/abstract/nodeWrap.cpp
        src/model/FlowBlock/abstract/nodeWrap.h
)
