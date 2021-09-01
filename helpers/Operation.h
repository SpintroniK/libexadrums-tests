#pragma once

#include "Visitor.h"

#include <string>
#include <memory>
#include <stack>

using namespace std::string_literals;

class Operation
{
public:
    Operation(const std::string& name) : name{name} {};
    virtual ~Operation() = default;

    virtual bool IsInput() const  = 0;
    virtual std::string Compute() = 0;
    virtual std::string GetName() const { return name; }
    virtual void Accept(OpVisitor& v) = 0;

private:

    std::string name;

};

class Input : public Operation
{
public:
    Input(const std::string& name) : Operation(name) {}
    virtual ~Input() = default;
    

    virtual bool IsInput() const final { return true; }
    virtual void Accept(OpVisitor& v) override;
};


class UnaryOp : public Operation
{
public:

    UnaryOp(const std::string& name, std::stack<Operation*>& opStack);

    UnaryOp(Operation* op) : Operation(op->GetName()), child{op} {}

    virtual ~UnaryOp() 
    { 
         delete child; 
    }

    void SetChild(Operation* node)
    {
        child = node;
    }

    virtual bool IsInput() const final { return false; }
    virtual void Accept(OpVisitor& v) override;

    Operation* GetChild() const { return child; }


protected:

    Operation* child;

};



class BinaryOp : public Operation
{
public:

    BinaryOp(const std::string& name, std::stack<Operation*>& opStack) : Operation(name)
    {
        op1 = opStack.top();
        opStack.pop();
        op2 = opStack.top();
        opStack.pop();
    }

    BinaryOp(const std::string& name, Operation* op1, Operation* op2) : Operation(name) {}

    virtual ~BinaryOp()
    {
        delete op1;
        delete op2;
    }

    virtual bool IsInput() const final { return false; }
    virtual void Accept(OpVisitor& v) override;

    auto GetOp1() const { return op1; }
    auto GetOp2() const { return op2; }


protected:

    Operation* op1{nullptr};
    Operation* op2{nullptr};

};


class SoundInput final : public Input
{
public:
    SoundInput(const std::string& name) : Input(name) 
    {
    }

    virtual ~SoundInput() = default;

    virtual std::string Compute() final { return "Sound"; }
    virtual void Accept(OpVisitor& v) override;

private:

    std::string fileLocation;
    std::string soundType;

};


class TriggerInput final : public Input
{
public:
    TriggerInput(const std::string& name) : Input(name)
    {

    }

    virtual ~TriggerInput() = default;

    virtual std::string Compute() final { return "Trigger"; }
    virtual void Accept(OpVisitor& v) override;

private:

};


class AmplitudeModulator final : public BinaryOp
{
public:

    AmplitudeModulator(const std::string& name, std::stack<Operation*>& opStack) : BinaryOp(name, opStack)
    {

    }

    virtual ~AmplitudeModulator() = default;

    virtual std::string Compute() final { return "AmplitudeModulator \n[\n\t"s + op1->Compute() + " | "s + op2->Compute() + "\n]"s; }

    virtual void Accept(OpVisitor& v) override;
};


using OperationPtr = std::shared_ptr<Operation>;