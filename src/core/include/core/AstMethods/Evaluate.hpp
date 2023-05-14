#include "core/AST.hpp"

namespace dxsh {
    namespace core {
        class Interpreter;

        namespace AstMethods {
            Value Evaluate(const Expr& expr, Interpreter& interpreter);
        }
    }
}