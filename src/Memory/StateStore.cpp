#include "StateStore.h"

#if FEATURE_MEMORY

#include <Preferences.h>
#include "../Utils/ParamRegistry.h"
#include "../Utils/Notify.h"

static Preferences prefs;

void state_store_init() {
    prefs.begin(ROBOT_STORAGE, false);
}

void state_store_load() {
    int loaded = 0;
    for (int i = 0; i < ParamRegistry::count(); i++) {
        const ParamEntry* e = ParamRegistry::at(i);
        if (!prefs.isKey(e->name)) continue;

        switch (e->type) {
            case ParamType::FLOAT: *static_cast<float*>(e->ptr)    = prefs.getFloat(e->name); break;
            case ParamType::U8:    *static_cast<uint8_t*>(e->ptr)  = prefs.getUChar(e->name); break;
            case ParamType::U16:   *static_cast<uint16_t*>(e->ptr) = prefs.getUShort(e->name); break;
            case ParamType::I16:   *static_cast<int16_t*>(e->ptr)  = prefs.getShort(e->name); break;
            case ParamType::U32:   *static_cast<uint32_t*>(e->ptr) = prefs.getULong(e->name); break;
            case ParamType::BOOL:  *static_cast<bool*>(e->ptr)     = prefs.getBool(e->name); break;
        }
        loaded++;
    }
    notify("loaded %d saved parameter(s)\n", loaded);
}

void state_store_save() {
    for (int i = 0; i < ParamRegistry::count(); i++) {
        const ParamEntry* e = ParamRegistry::at(i);

        switch (e->type) {
            case ParamType::FLOAT: prefs.putFloat(e->name,  *static_cast<float*>(e->ptr)); break;
            case ParamType::U8:    prefs.putUChar(e->name,  *static_cast<uint8_t*>(e->ptr)); break;
            case ParamType::U16:   prefs.putUShort(e->name, *static_cast<uint16_t*>(e->ptr)); break;
            case ParamType::I16:   prefs.putShort(e->name,  *static_cast<int16_t*>(e->ptr)); break;
            case ParamType::U32:   prefs.putULong(e->name,  *static_cast<uint32_t*>(e->ptr)); break;
            case ParamType::BOOL:  prefs.putBool(e->name,   *static_cast<bool*>(e->ptr)); break;
        }
    }
    notify("saved %d parameter(s)\n", ParamRegistry::count());
}

#endif
