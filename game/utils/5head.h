#undef FUNC
#undef VAR
#ifdef FHEAD
  #define FUNC(definition,content) \
    definition;
  
  #define VAR(definition,content) \
    extern definition;
#else
  #define FUNC(definition,content) \
    definition \
    content
  
  #define VAR(definition,content) \
    definition content;
#endif
