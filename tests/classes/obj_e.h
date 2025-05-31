#pragma once

#include "obj_a.h"
#include "obj_d.h"

class ObjE : public ObjA, public ObjD
{
public: // Methods
    
    // Constraint: Constructor and destructor must be implemented
    // in the implementation file (no inline definitions).
    ObjE();

    virtual ~ObjE();

    void obj_e_func();
};
