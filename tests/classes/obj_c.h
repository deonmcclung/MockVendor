#pragma once

#include "obj_b.h"

class ObjC : public ObjB
{
public: // Methods
    
    // Constraint: Constructor and destructor must be implemented
    // in the implementation file (no inline definitions).
    ObjC();

    virtual ~ObjC();

    void obj_c_func();

    virtual int obj_b_v_func() override;
};


