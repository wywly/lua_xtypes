#define LUA_LIB

#define LUAW_NO_CXX11
#include "luawrapper.hpp"
#include "luawrapperutil.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <stdint.h>


uint64_t power(uint64_t base, uint64_t exponent);

template<class TInt>
struct IntWrapper {
    TInt value;

    explicit IntWrapper() : value() {}
    explicit IntWrapper(TInt v) : value(v) {}
    IntWrapper(const IntWrapper& x) : value(x.value) {}

    IntWrapper  operator+(const IntWrapper& r) const {
        return IntWrapper(value + r.value);
    }
    IntWrapper  operator-(const IntWrapper& r) const {
        return IntWrapper(value - r.value);
    }
    IntWrapper  operator*(const IntWrapper& r) const {
        return IntWrapper(value * r.value);
    }
    IntWrapper  operator/(const IntWrapper& r) const {
        if (r.value == TInt())
            throw std::invalid_argument("division by zero");
        return IntWrapper(value / r.value);
    }
    IntWrapper  operator%(const IntWrapper& r) const {
        if (r.value == TInt())
            throw std::invalid_argument("division by zero");
        return IntWrapper(value % r.value);
    }
    bool        operator<(const IntWrapper& r) const {
        return value < r.value;
    }
    bool        operator<=(const IntWrapper& r) const {
        return value <= r.value;
    }
    bool        operator==(const IntWrapper& r) const {
        return value == r.value;
    }

    IntWrapper  operator-() const {
        return IntWrapper(-value);
    }

    IntWrapper  pow(const IntWrapper& r) const {
        return IntWrapper( power(value, r.value) );
    }

    std::string tostring() const {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }
};

template<class TInt>
struct IntWrapperRegistry {
    typedef IntWrapper<TInt> Wrapper;

    static Wrapper* __push_new_value(lua_State* L, TInt value) {
        Wrapper* w = new Wrapper( value );
        luaW_push<Wrapper>(L, w);
        luaW_hold<>(L, w);
        return w;
    }
    static Wrapper* __push_new_value(lua_State* L, const Wrapper& value) {
        Wrapper* w = new Wrapper( value );
        luaW_push<Wrapper>(L, w);
        luaW_hold<>(L, w);
        return w;
    }

    static Wrapper* __from_stack(lua_State* L, int index) {
        switch (lua_type(L, index)) {
            case LUA_TNUMBER: {
                return __push_new_value(L, luaU_check<TInt>(L, index) );
            }
            case LUA_TSTRING: {
                size_t len = 0;
                const uint8_t * str = (const uint8_t *)lua_tolstring(L, index, &len);
                if (len > 8) {
                    const char *msg = lua_pushfstring(L, "The string (length = %d) is too long to be argument", len);
                    luaL_argerror(L, index, msg);
                    return NULL;
                }

                uint64_t n64 = 0;
                for (size_t i = 0; i < len; i++) {
                    n64 |= (uint64_t)str[i] << (i*8);
                }
                return __push_new_value(L, static_cast<TInt>(n64) );
            }
            default:
                return luaW_check<Wrapper>(L, index);
        }
    }

#define add(a, b)   ((a)+(b))
#define sub(a, b)   ((a)-(b))
#define mul(a, b)   ((a)*(b))
#define div(a, b)   ((a)/(b))
#define mod(a, b)   ((a)%(b))
#define pow(a, b)   ((a).pow(b))
#define  eq(a, b)   ((a)==(b))
#define  le(a, b)   ((a)<=(b))
#define  lt(a, b)   ((a)< (b))
#define  lt(a, b)   ((a)< (b))
#define unm(x)      (-(x))
#define new(x)        (x)

#define binary_val(functor) \
    static int __##functor(lua_State* L) {\
        Wrapper* l = __from_stack(L, 1);\
        Wrapper* r = __from_stack(L, 2);\
        const char* err = NULL;\
        try { __push_new_value(L, functor(*l, *r)); return 1; }\
        catch(std::exception& e) { err = lua_pushfstring(L, e.what()); }\
        if (err) { return luaL_error(L, "%s", err); }\
    }

#define binary_bool(functor) \
    static int __##functor(lua_State* L) {\
        Wrapper* l = __from_stack(L, 1);\
        Wrapper* r = __from_stack(L, 2);\
        luaU_push<bool>(L, functor(*l, *r));\
        return 1;\
    }

#define unary(functor) \
    static int __##functor(lua_State* L) {\
        Wrapper* w = __from_stack(L, 1);\
        __push_new_value(L, functor(*w) );\
        return 1;\
    }

    binary_val(add)
    binary_val(sub)
    binary_val(mul)
    binary_val(div)
    binary_val(mod)
    binary_val(pow)
    binary_bool(eq)
    binary_bool(le)
    binary_bool(lt)
    unary(unm)
    unary(new)

    static int __tostring(lua_State* L) {
        const Wrapper* w = luaW_check<Wrapper>(L, 1);
        luaU_push<std::string>(L, w->tostring());
        return 1;
    }

    static luaL_Reg* static_methods() {
        static luaL_Reg table[] = {
            { "new",   &__new },
            { NULL, NULL }
        };
        return table;
    }

    static luaL_Reg* methods() {
        static luaL_Reg table[] = {
            { "__add", &__add },
            { "__sub", &__sub },
            { "__mul", &__mul },
            { "__div", &__div },
            { "__mod", &__div },
            { "__unm", &__div },
            { "__pow", &__pow },
            { "__eq", &__eq },
            { "__lt", &__lt },
            { "__le", &__le },
            { "__tostring", &__tostring },
            { NULL, NULL }
        };
        return table;
    }
};


typedef IntWrapperRegistry<int32_t> int32;
typedef IntWrapperRegistry<uint32_t> uint32;
typedef IntWrapperRegistry<int64_t> int64;
typedef IntWrapperRegistry<uint64_t> uint64;

extern "C" {

int luaopen_xtypes(lua_State* L) {
    luaW_register<int32::Wrapper>(L, "int32", int32::static_methods(), int32::methods());
    luaW_register<uint32::Wrapper>(L, "uint32", uint32::static_methods(), uint32::methods());
    luaW_register<int64::Wrapper>(L, "int64", int64::static_methods(), int64::methods());
    luaW_register<uint64::Wrapper>(L, "uint64", uint64::static_methods(), uint64::methods());
    return 1;
}

}

uint64_t power(uint64_t base, uint64_t exponent) {
    if (exponent == 1) {
        return base;
    }
    int64_t base2 = base * base;
    if (exponent % 2 == 1) {
        return power(base2, exponent/2) * base;
    } else {
        return power(base2, exponent/2);
    }
}
