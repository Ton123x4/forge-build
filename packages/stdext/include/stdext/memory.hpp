#pragma once

#include <cassert>
#include <cstdlib>
#include <utility>
#include <memory>
#include <new>

namespace stdext {
    template<typename Type>
    struct memory_view {
        template<typename FromPtr>
        inline static auto from(FromPtr* ptr) noexcept -> Type* {
            static_assert(std::is_trivially_copyable_v<Type>, "memory_view::Type must be trivially copyable");
            return static_cast<Type*>(ptr);
        }

        template<typename FromPtr>
        inline static auto reinterpret(FromPtr* ptr) noexcept -> Type* {
            static_assert(std::is_trivially_copyable_v<Type>, "memory_view::Type must be trivially copyable");
            return reinterpret_cast<Type*>(ptr);
        }

        template<typename FromPtr>
        inline static auto reinterpret(FromPtr* ptr, size_t offset) noexcept -> Type* {
            static_assert(std::is_trivially_copyable_v<Type>, "memory_view::Type must be trivially copyable");
            return reinterpret_cast<Type*>(reinterpret_cast<char*>(ptr) + offset);
        }

        inline static auto size() noexcept {
            return sizeof(Type);
        }
    };

    struct safe_object_base {};

    template<typename Derived, typename Base = safe_object_base>
    struct safe_object : public Base {
        using Base::Base;
        using SafePtr = std::unique_ptr<Derived>;
        using RawPtr = Derived*;

        using safe_ptr = SafePtr;
        using raw_ptr = RawPtr;

        template<typename... Args, typename = std::enable_if_t<std::is_constructible_v<Derived, Args&&...>>>
        inline static auto make(Args&&... args) {
            return std::make_unique<Derived>(std::forward<Args>(args)...);
        }

        template<typename FromPtr>
        inline static auto from(FromPtr* ptr) noexcept -> Derived* {
            return static_cast<Derived*>(ptr);
        }

        template<typename FromPtr>
        inline static auto dynamic(FromPtr* ptr) noexcept -> Derived* {
            static_assert(std::is_polymorphic_v<Base>, "Base must be polymorphic for dynamic_cast to work");
            return dynamic_cast<Derived*>(ptr);
        }

        template<typename FromPtr>
        inline static auto from(std::unique_ptr<FromPtr>& ptr) noexcept -> Derived* {
            return static_cast<Derived*>(ptr.get());
        }

        template<typename FromPtr>
        inline static auto dynamic(std::unique_ptr<FromPtr>& ptr) noexcept -> Derived* {
            static_assert(std::is_polymorphic_v<Base>, "Base must be polymorphic for dynamic_cast to work");
            return dynamic_cast<Derived*>(ptr.get());
        }

        template<typename FromPtr>
        inline static auto take(std::unique_ptr<FromPtr>& ptr) noexcept -> SafePtr {
            return SafePtr(from(ptr.release()));
        }

        template<typename FromPtr>
        inline static auto take_dynamic(std::unique_ptr<FromPtr>& ptr) noexcept -> SafePtr {
            static_assert(std::is_polymorphic_v<Base>, "Base must be polymorphic for dynamic_cast to work");
            auto* casted = dynamic_cast<Derived*>(ptr.get());

            if (!casted) {
                return nullptr;
            }

            ptr.release();
            return SafePtr(casted);
        }
    };


    template<typename Target, typename Source>
    auto safe_cast(std::unique_ptr<Source> ptr) {
        static_assert(std::is_base_of_v<Source, Target> || std::is_base_of_v<Target, Source>, "Invalid type conversion: types are unrelated");

        if (ptr == nullptr) {
            return std::unique_ptr<Target>(nullptr);
        }

        return std::unique_ptr<Target>(static_cast<Target*>(ptr.release()));
    }

    template<typename Target, typename Source>
    auto safe_dynamic_cast(std::unique_ptr<Source> ptr) {
        if (ptr != nullptr) {
            auto raw = dynamic_cast<Target*>(ptr.get());

            if (raw != nullptr) {
                ptr.release();
                return std::unique_ptr<Target>(raw);
            }
        }

        return std::unique_ptr<Target>(nullptr);
    }


    template<typename Type>
    struct safe_alloc_deleter {
        void operator()(Type* ptr) const {
            std::free(ptr);
        }
    };

    template<typename Type, typename Deleter>
    auto make_safe(Type* ptr, Deleter deleter) {
        return std::unique_ptr<Type, Deleter>(ptr, deleter);
    }

    template<typename Type>
    auto make_alloc(size_t size = sizeof(Type)) {
        auto ptr = static_cast<Type*>(std::malloc(size));

        if (ptr == nullptr) {
            throw std::bad_alloc();
        }

        return std::unique_ptr<Type, safe_alloc_deleter<Type>>(ptr);
    }
}
