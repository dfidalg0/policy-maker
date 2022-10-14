#ifndef __LEXICON_CONTEXT_HH__
#define __LEXICON_CONTEXT_HH__

namespace lexicon {
    struct Context {
        Context(): line_start(1), col_start(1), line_end(1), col_end(1) {}

        int line_start;
        int line_end;
        int col_start;
        int col_end;
    };
};

#endif // __LEXICON_CONTEXT_HH__
