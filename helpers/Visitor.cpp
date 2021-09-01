#include "Operation.h"
#include "Visitor.h"

#include <iostream>

void OpTreePrinter::Visit(const Input& input)
{
    std::cout << "Input: " << input.GetName() << std::endl;
}

void OpTreePrinter::Visit(const UnaryOp& op)
{
    std::cout << "Unary Op: " << op.GetName() << std::endl;
}

void OpTreePrinter::Visit(const BinaryOp& op)
{
    std::cout << "Binary Op: " << op.GetName() << std::endl;
}

void OpTreePrinter::Visit(const SoundInput& op)
{
    std::cout << "SoundInput: " << op.GetName() << std::endl;
}

void OpTreePrinter::Visit(const TriggerInput& op)
{
    std::cout << "TriggerInput: " << op.GetName() << std::endl;
}

void OpTreePrinter::Visit(const AmplitudeModulator& op)
{
    std::cout << "AmplitudeModulator: " << op.GetName() << std::endl;
    op.GetOp1()->Accept(*this);
    op.GetOp2()->Accept(*this);
}
