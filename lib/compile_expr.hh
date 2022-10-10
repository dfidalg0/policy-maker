#ifndef __COMPILE_EXPR_HH__
#define __COMPILE_EXPR_HH__

#include <vector>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <syntax/nodes.hh>

#define SECCOMP_DATA(name) offsetof(seccomp_data, name)

std::vector<sock_filter> * compile_expr(Expr * expr);

#endif // __COMPILE_EXPR_HH__
