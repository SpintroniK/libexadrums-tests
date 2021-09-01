#include "Operation.h"


UnaryOp::UnaryOp(const std::string& name, std::stack<Operation*>& opStack) 
: Operation(name)
{
    child = opStack.top();
    opStack.pop();
}


void Input::Accept(OpVisitor& v) 
{ 
    v.Visit(*this); 
}

void UnaryOp::Accept(OpVisitor& v)
{
    v.Visit(*this);
}

void BinaryOp::Accept(OpVisitor& v)
{
    v.Visit(*this);
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
