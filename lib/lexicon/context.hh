#ifndef __LEXICON_CONTEXT_HH__
#define __LEXICON_CONTEXT_HH__

#include <lexicon/token.hh>

namespace lexicon {
    struct Context {
        Context(): line_start(1), col_start(1), line_end(1), col_end(1), last_token(nullptr) {}

        int line_start;
        int line_end;
        int col_start;
        int col_end;

        Token *last_token;
    };
};

#endif // __LEXICON_CONTEXT_HH__
