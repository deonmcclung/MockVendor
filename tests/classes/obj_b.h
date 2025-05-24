#pragma once

#include "obj_a.h"

class ObjB : public ObjA
{
public: // Methods
    
    // Constraint: Constructor and destructor must be implemented
    // in the implementation file (no inline definitions).
    ObjB();

    virtual ~ObjB();

    int obj_b_func();

    virtual int obj_b_v_func();
};

