#include "Operation.h"
#include "Visitor.h"

#include <iostream>
#include <iomanip>


void OpTreePrinter::Visit(const SoundInput& op)
{
    std::cout << "SoundInput: " << op.GetName();
}

void OpTreePrinter::Visit(const TriggerInput& op)
{
    std::cout << "TriggerInput: " << op.GetName();
}

void OpTreePrinter::Visit(const AmplitudeModulator& op)
{
    depth++;
    std::cout << "AmplitudeModulator: " << op.GetName() << std::endl;
    std::cout << std::setw(depth * 4) << " ";
    op.GetOp1()->Accept(*this);
    std::cout << std::endl;
    std::cout << std::setw(depth * 4) << " ";
    op.GetOp2()->Accept(*this);
    depth--;
}
