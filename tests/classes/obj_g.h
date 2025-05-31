#pragma once

#include "obj_a.h"
#include "obj_d.h"
#include "obj_f.h"

class ObjG : public ObjA, public ObjD, public ObjF
{
public: // Methods
    
    // Constraint: Constructor and destructor must be implemented
    // in the implementation file (no inline definitions).
    ObjG();

    virtual ~ObjG();

    void obj_g_func();
};
