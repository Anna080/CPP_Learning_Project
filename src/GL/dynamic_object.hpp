#pragma once

#include <unordered_set>

namespace GL {

class DynamicObject
{
public:
    bool destroy = false;
    virtual ~DynamicObject() {}
    virtual bool move() = 0;
    virtual bool doDestroy() { return destroy; }
};

inline std::unordered_set<DynamicObject*> move_queue;

} // namespace GL
