#pragma once

class Input;
class UnaryOp;
class BinaryOp;
class SoundInput;
class TriggerInput;
class AmplitudeModulator;


template <typename T, typename... Types>
class Visitable : public Visitable<T>, public Visitable<Types...>
{
public:
    using Visitable<T>::Accept;
    using Visitable<Types...>::Accept;
};

template <typename T>
class Visitable<T>
{
public:
    virtual void Accept(T&) = 0;
};

template <typename T, typename... Types>
class Visitor : public Visitor<T>, public Visitor<Types...>
{
public:
    using Visitor<T>::Visit;
    using Visitor<Types...>::Visit;
};

template <typename T>
class Visitor<T>
{
public:
    virtual void Visit(const T&) = 0;
};


using OpVisitor = Visitor<SoundInput, TriggerInput, AmplitudeModulator>;

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

class OpTreeEvaluator : public OpVisitor
{
public:
    OpTreeEvaluator() = default;
    virtual ~OpTreeEvaluator() = default;


    virtual void Visit(const SoundInput& op) override;
    virtual void Visit(const TriggerInput& op) override;
    virtual void Visit(const AmplitudeModulator& op) override;

    auto GetResult() const { return result; }

private:
    double result = 0;
};
