#include "core/Defer.hpp"

using namespace dxsh;
using namespace core;

void DeferManager::DeferExpr(const Expr& expr) {
    defers.insert_or_assign(&expr, std::nullopt);
}

void DeferManager::DeferExpr(const Expr& expr, Value&& value) {
    defers.insert_or_assign(&expr, std::move(value));
}

const Value* DeferManager::GetDeferValue(const Expr& expr) const {
    auto it = defers.find(&expr);

    if (it == defers.end() || not it->second.has_value())
        return nullptr;
    else
        return &it->second.value();
}

void DeferManager::CleanDeferValue(const Expr& expr) {
    defers.erase(&expr);
}

DeferGuard::~DeferGuard() {
    if (shouldClean) {
        DeferManager::Inst().CleanDeferValue(*expr);
    } else {
        DeferManager::Inst().DeferExpr(*expr, std::move(*value));
    }
}

DeferGuard::DeferGuard(DeferGuard&& move)
    : expr(std::exchange(move.expr, nullptr))
    , value(std::exchange(move.value, nullptr))
{ }

DeferGuard& DeferGuard::operator=(DeferGuard&& move) {
    std::swap(*this, move);
    return *this;
}