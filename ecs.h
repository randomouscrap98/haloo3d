// haloopdy - 2024
// A super simple, all macro-driven ecs implementation.
// Macros create a struct and associated functions to build
// a simple, lightweight ecs implementation
#ifndef __HALOO3D_ECS2_H
#define __HALOO3D_ECS2_H

// Note: you can get rid of string.h as a requirement if you
// redo that memset to be a loop (up to you)
#include <string.h>

#ifndef ECS_MAXENTITIES
#define ECS_MAXENTITIES 1024
#endif

// We reserve one slot to automatically point back to the ecs system.
// This is the unusable 64th ID = 63 (your ids should start from 0)
#define ECS_MAXCTYPES 63
// The self flag is an ID that will get you back a component that is
// the ecs system itself. It is ALSO used to identify if an entity is
// valid, as ALL entities will implicitly have the ECS component
#define ECS_SELFFLAG (1ULL << ECS_MAXCTYPES)

typedef unsigned long long ecs_cid;
typedef int ecs_eid;

#define ECS_START(name)                                                        \
  typedef struct name name;                                                    \
  struct name {                                                                \
    ecs_cid entities[ECS_MAXENTITIES];                                         \
    ecs_eid entitytop;                                                         \
    name *c_##name[ECS_MAXENTITIES];

#define ECS_END(name)                                                          \
  }                                                                            \
  ;                                                                            \
  /* Always initialize the ecs system when you create a new one! */            \
  static void name##_init(name *_ecs) { memset(_ecs, 0, sizeof(name)); }       \
  /* Create an entity with the given base components (usually 0) */            \
  static ecs_eid name##_newentity(name *_ecs, ecs_cid basecomponents) {        \
    for (int i = 0; i < ECS_MAXENTITIES; i++) {                                \
      ecs_eid id = _ecs->entitytop;                                            \
      _ecs->entitytop = (_ecs->entitytop + 1) % ECS_MAXENTITIES;               \
      if (_ecs->entities[id] == 0) {                                           \
        _ecs->entities[id] = ECS_SELFFLAG | basecomponents;                    \
        _ecs->c_##name[id] = _ecs;                                             \
        return id;                                                             \
      }                                                                        \
    }                                                                          \
    return -1;                                                                 \
  }                                                                            \
  /* Delete an entity by eid from the given ecs sys */                         \
  static void name##_deleteentity(name *_ecs, ecs_eid eid) {                   \
    if (eid >= 0 && eid < ECS_MAXENTITIES)                                     \
      _ecs->entities[eid] = 0;                                                 \
  }                                                                            \
  /* Whether an entity matches a given list of components by flag */           \
  static int name##_match(name *_ecs, ecs_eid _eid, ecs_cid _comps) {          \
    ecs_cid _realcomps = ECS_SELFFLAG | _comps;                                \
    return (_ecs->entities[_eid] & _realcomps) == _realcomps;                  \
  }                                                                            \
  /* fill given list with eids of entities that match. careful with size */    \
  static int name##_query(name *_ecs, ecs_cid _comps, ecs_eid *out) {          \
    int count = 0;                                                             \
    for (int i = 0; i < ECS_MAXENTITIES; i++) {                                \
      if (name##_match(_ecs, i, _comps)) {                                     \
        out[count++] = i;                                                      \
      }                                                                        \
    }                                                                          \
    return count;                                                              \
  }                                                                            \
  /* Calculate the eid from a double ecs pointer. Useful to get eid in sys */  \
  static ecs_eid name##_eid(name **_ecs) {                                     \
    return (ecs_eid)(_ecs - (*_ecs)->c_##name);                                \
  }                                                                            \
  const ecs_cid name##_fl = ECS_SELFFLAG;

// Create an ECS component within an ECS struct
#define ECS_COMPONENT(type) type c_##type[ECS_MAXENTITIES];

#define ECS_GETCOMPONENT(_ecs, eid, type) (_ecs)->c_##type[eid]

// Define the CIDs necessary to access a component (each component MUST
// have a unique id and I can't do that inside a macro without requiring
// extensions)
#define ECS_CID(type, id)                                                      \
  const int type##_id = id;                                                    \
  const ecs_cid type##_fl = (1ULL << id);

// Set the given entity to have the given component (by type). Follow this
// macro immediately with the struct initializer
#define ECS_SETCOMPONENT(_ecs, eid, type)                                      \
  (_ecs)->entities[eid] |= type##_fl;                                          \
  (_ecs)->c_##type[eid] = (type)

// Add a system function which automatically calls your given function with
// pre-pulled items from the entity component arrays. The new function is
// named the same as the old one, just with _run appeneded
#define ECS_SYSTEM1(ecsname, fname, type1)                                     \
  void fname##_run(ecsname *_ecs, ecs_eid eid) {                               \
    ecs_cid _ecsflags = type1##_fl;                                            \
    if (ecsname##_match(_ecs, eid, _ecsflags)) {                               \
      fname(_ecs->c_##type1 + eid);                                            \
    }                                                                          \
  }

// Add a system function which automatically calls your given function with
// pre-pulled items from the entity component arrays. The new function is
// named the same as the old one, just with _run appeneded
#define ECS_SYSTEM2(ecsname, fname, type1, type2)                              \
  void fname##_run(ecsname *_ecs, ecs_eid eid) {                               \
    ecs_cid _ecsflags = type1##_fl | type2##_fl;                               \
    if (ecsname##_match(_ecs, eid, _ecsflags)) {                               \
      fname(_ecs->c_##type1 + eid, _ecs->c_##type2 + eid);                     \
    }                                                                          \
  }

// Add a system function which automatically calls your given function with
// pre-pulled items from the entity component arrays. The new function is
// named the same as the old one, just with _run appeneded
#define ECS_SYSTEM3(ecsname, fname, type1, type2, type3)                       \
  void fname##_run(ecsname *_ecs, ecs_eid eid) {                               \
    ecs_cid _ecsflags = type1##_fl | type2##_fl | type3##_fl;                  \
    if (ecsname##_match(_ecs, eid, _ecsflags)) {                               \
      fname(_ecs->c_##type1 + eid, _ecs->c_##type2 + eid,                      \
            _ecs->c_##type3 + eid);                                            \
    }                                                                          \
  }

// Add a system function which automatically calls your given function with
// pre-pulled items from the entity component arrays. The new function is
// named the same as the old one, just with _run appeneded
#define ECS_SYSTEM4(ecsname, fname, type1, type2, type3, type4)                \
  void fname##_run(ecsname *_ecs, ecs_eid eid) {                               \
    ecs_cid _ecsflags = type1##_fl | type2##_fl | type3##_fl | type4##_fl;     \
    if (ecsname##_match(_ecs, eid, _ecsflags)) {                               \
      fname(_ecs->c_##type1 + eid, _ecs->c_##type2 + eid,                      \
            _ecs->c_##type3 + eid, _ecs->c_##type4 + eid);                     \
    }                                                                          \
  }

// Add a system function which automatically calls your given function with
// pre-pulled items from the entity component arrays. The new function is
// named the same as the old one, just with _run appeneded
#define ECS_SYSTEM5(ecsname, fname, type1, type2, type3, type4, type5)         \
  void fname##_run(ecsname *_ecs, ecs_eid eid) {                               \
    ecs_cid _ecsflags =                                                        \
        type1##_fl | type2##_fl | type3##_fl | type4##_fl | type5##_fl;        \
    if (ecsname##_match(_ecs, eid, _ecsflags)) {                               \
      fname(_ecs->c_##type1 + eid, _ecs->c_##type2 + eid,                      \
            _ecs->c_##type3 + eid, _ecs->c_##type4 + eid,                      \
            _ecs->c_##type5 + eid);                                            \
    }                                                                          \
  }

// Add a system function which automatically calls your given function with
// pre-pulled items from the entity component arrays. The new function is
// named the same as the old one, just with _run appeneded
#define ECS_SYSTEM6(ecsname, fname, type1, type2, type3, type4, type5, type6)  \
  void fname##_run(ecsname *_ecs, ecs_eid eid) {                               \
    ecs_cid _ecsflags = type1##_fl | type2##_fl | type3##_fl | type4##_fl |    \
                        type5##_fl | type6##_fl;                               \
    if (ecsname##_match(_ecs, eid, _ecsflags)) {                               \
      fname(_ecs->c_##type1 + eid, _ecs->c_##type2 + eid,                      \
            _ecs->c_##type3 + eid, _ecs->c_##type4 + eid,                      \
            _ecs->c_##type5 + eid, _ecs->c_##type6 + eid);                     \
    }                                                                          \
  }

#endif
