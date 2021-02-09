# Programs

Within this repository, there are 3 different programs: the interpreter, the
test suite, and the playground. Respectively, these 3

- run jasper programs
- run tests, which can be found under the `tests/` directory
- show cst dumps and other useful debugging features

# Stages

The general stages of running a Jasper program go as follows:

 - Parsing -- The parser produces a `CST`. This stores information about the
 source location of the different syntactic constructs in a program.
 - AST Conversion -- We convert the `CST` into an `AST`, discarding source
 location information. We also de-sugar some constructs into simpler ones.
 - Symbol resolution -- We create a symbol table, and match every identifier to
 its declaration, while gathering references between global declarations.
 - Metatype inference -- validates that types and values are used in acceptable
 ways. (e.g. no saying that a variable has type 42) while eliminating certain
 ambiguities (e.g. is this identifier a type?).
 - Type evaluation -- We build data structures that represent types and
 relations between them.
 - Type inference -- For every declaration and expression, we infer a type using
 Hindley-Milner style unification. We collapse circular global declarations into
 letrec blocks.
 - Offset calculation -- In preparation for our stack-based interpreter, we
 assign a stack frame offset to every variable and temporary value.
 - Evaluation -- we walk over the AST, evaluating expressions, and following
 control flow. Variables and intermediate values are stored in a stack.

## Parsing

First, the input source file is tokenized by a hand written lexer. Then, a hand
written recursive descent parser builds a syntax tree.

We use Pratt parsing to do infix operator precedence parsing.

## Evaluation

Our interpreter works by walking over the AST, and storing values in a software
stack. The values are managed by our own garbage collector.

# Project structure

To build the project, we use a makefile that lives on the root of the repo. All
the other interesting files are within the `src/` directory.

The tree-walk interpreter lives in the `src/interpreter/` directory.

There are a few generally useful data structures in the `src/utils/` directory,
and some more project-specific ones in the `src/algorithms/` directory.

`src/test/` stores the implementation of the test runner, while `src/tests/`
stores the content of the tests themselves. You will need to touch these in
order to add new tests.
