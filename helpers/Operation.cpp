#include "Operation.h"


UnaryOp::UnaryOp(const std::string& name, std::stack<Operation*>& opStack) 
: Operation(name)
{
    child = opStack.top();
    opStack.pop();
}


void SoundInput::Accept(OpVisitor& v)
{
    v.Visit(*this);
}

void TriggerInput::Accept(OpVisitor& v)
{
    v.Visit(*this);
}

void AmplitudeModulator::Accept(OpVisitor& v)
{
    v.Visit(*this);
}
