#pragma once

// EPOS Handler Utility Declarations

class Handler
{
public:
    // A handler function
    typedef void (Function)();

public:
    Handler() {}
    virtual ~Handler() {}

    virtual void operator()() = 0;
};

class Function_Handler: public Handler
{
public:
    Function_Handler(Function * h): _handler(h) {}
    ~Function_Handler() {}

    void operator()() { _handler(); }

private:
    Function * _handler;
};

template<typename T>
class Functor_Handler: public Handler
{
public:
    typedef void (Functor)(T *);

public:
    Functor_Handler(Functor * h, T * p): _handler(h), _ptr(p) {}
    ~Functor_Handler() {}

    void operator()() { _handler(_ptr); }

private:
    Functor * _handler;
    T * _ptr;
};
