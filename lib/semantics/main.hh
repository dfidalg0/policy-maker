#ifndef __SEMANTICS_MAIN_HH__
#define __SEMANTICS_MAIN_HH__

#include <syntax.hh>
#include <utility>
#include <vector>
#include <unordered_map>
#include <string>

typedef std::vector<std::pair<Expr *, Action *>> SyscallRules;
typedef std::unordered_map<std::string, SyscallRules *> PolicyRules;
typedef std::unordered_map<std::string, PolicyRules *> Policies;

Policies * compile (Program * prog);

#endif // __SEMANTICS_MAIN_HH__
