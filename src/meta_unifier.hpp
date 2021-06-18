#pragma once

struct MetaUnifier {

void* comp;

void register_dot_target(int idx) { return; }
void unify(int idx1, int idx2) { return; }
int eval(int idx) { return idx; }
int make_const_node() { return 0; }
int make_var_node() { return 0; }
int make_dot_node() { return 0; }

};
