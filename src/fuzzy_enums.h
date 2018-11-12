
#ifndef fuzzy_enums
#define fuzzy_enums fuzzy_enums

enum Inference_t { MAX_MIN,
                   MAX_PROD};

enum Defuzzy_t { DEFUZZY_COG,
                 DEFUZZY_MCOG,
                 DEFUZZY_ACOG,
                 DEFUZZY_MHR,
                 DEFUZZY_MHL,
                 DEFUZZY_LOM,
                 DEFUZZY_FOM };

enum FuzzyAnd_t { AND_MIN,
                  AND_BOUNDED_DIFF,
                  AND_ALGEBRAIC_SUM };

enum FuzzyOr_t { OR_MAX,
                 OR_BOUNDED_SUM,
                 OR_ALGEBRAIC_SUM};
#endif

