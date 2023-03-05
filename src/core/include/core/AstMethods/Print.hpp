#include "core/AST.hpp"

namespace dxsh {
    namespace core {
        namespace AstMethods {
            std::string PrintRPN(const Expr& expr);
            std::string PrintInfix(const Expr& expr);
        }
    }
}