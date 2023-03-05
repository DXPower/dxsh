#include "core/AST.hpp"
#include "core/Environment.hpp"
#include "core/Error.hpp"

namespace dxsh {
    namespace core {
        namespace AstMethods {
            Value  Evaluate(const Expr& expr, Environment& env, ErrorContext& errors);
        }
    }
}