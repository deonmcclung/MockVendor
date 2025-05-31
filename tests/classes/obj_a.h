#pragma once

#include <string>

class ObjA
{
public: // Methods
    
    // Constraint: Constructor and destructor must be implemented
    // in the implementation file (no inline definitions).
    ObjA();

    virtual ~ObjA();

    void open(const std::string& filename);

    void close();

    void obj_a_func();

private:
    // Private methods don't require mocking or a sub function,
    // unless they are virtual.
    void _privateMethod();
};
