#pragma once

#include "obj_b.h"
#include "obj_c.h"
#include "obj_d.h"

class ObjH : public ObjD, public ObjC, public ObjF
{
public: // Methods
    
    // Constraint: Constructor and destructor must be implemented
    // in the implementation file (no inline definitions).
    ObjH();

    virtual ~ObjH();

    void obj_h_func();
};
