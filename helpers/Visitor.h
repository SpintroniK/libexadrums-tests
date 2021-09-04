#pragma once

class Input;
class UnaryOp;
class BinaryOp;
class SoundInput;
class TriggerInput;
class AmplitudeModulator;

class OpVisitor
{

public:

    OpVisitor() = default;
    virtual ~OpVisitor() = default;

    virtual void Visit(const SoundInput& op) = 0;
    virtual void Visit(const TriggerInput& op) = 0;
    virtual void Visit(const AmplitudeModulator& op) = 0;

};

class OpTreePrinter : public OpVisitor
{

public:

    OpTreePrinter() = default;
    virtual ~OpTreePrinter() = default;

    virtual void Visit(const SoundInput& op) override;
    virtual void Visit(const TriggerInput& op) override;
    virtual void Visit(const AmplitudeModulator& op) override;

private:
    int depth = 0;

};