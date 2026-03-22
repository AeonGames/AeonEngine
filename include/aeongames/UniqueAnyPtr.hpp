/*
Copyright (C) 2018,2019,2025,2026 Rodrigo Jose Hernandez Cordoba

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#ifndef AEONGAMES_UNIQUEANYPTR_H
#define AEONGAMES_UNIQUEANYPTR_H
#include <sstream>
#include <memory>
#include <typeinfo>
#include <algorithm>

namespace AeonGames
{
    /** @brief A type-erased owning smart pointer with unique ownership semantics.
     *
     * Stores a pointer to any type along with a deleter, providing type-safe
     * access and automatic cleanup. Similar to std::unique_ptr but can hold any type.
     */
    class UniqueAnyPtr
    {
        void* mPointer{};
        const std::type_info& ( *mManager ) ( void* aPointer )
        {
            nullptr
        };
    public:
        /** @brief Swaps the contents of this pointer with another.
         * @param aUniqueAnyPtr The other UniqueAnyPtr to swap with.
         */
        void Swap ( UniqueAnyPtr& aUniqueAnyPtr ) noexcept
        {
            std::swap ( mPointer, aUniqueAnyPtr.mPointer );
            std::swap ( mManager, aUniqueAnyPtr.mManager );
        }
        /** @name Creation and Destruction. */
        /**@{*/
        /** @brief Default constructor. Creates an empty UniqueAnyPtr. */
        UniqueAnyPtr() noexcept = default;
        /** @brief Constructs an empty UniqueAnyPtr from nullptr. */
        UniqueAnyPtr ( std::nullptr_t ) noexcept : mManager
        {
            nullptr
        } {};
        UniqueAnyPtr ( const UniqueAnyPtr& aUniqueResource ) = delete;
        UniqueAnyPtr& operator= ( const UniqueAnyPtr& aUniqueResource ) = delete;
        /** @brief Move constructor. Transfers ownership from another UniqueAnyPtr.
         * @param aUniqueAnyPtr The source pointer to move from.
         */
        UniqueAnyPtr ( UniqueAnyPtr&& aUniqueAnyPtr ) noexcept : mManager
        {
            nullptr
        }
        {
            aUniqueAnyPtr.Swap ( *this );
        }

        /** @brief Constructs from a std::unique_ptr, taking ownership.
         * @tparam T The pointed-to type.
         * @param aUniquePointer The unique_ptr to take ownership from.
         */
        template<class T>
        UniqueAnyPtr ( std::unique_ptr<T>&& aUniquePointer ) noexcept :
            mPointer{ aUniquePointer.release() }, mManager{Manager<T>} {}

        /** @brief Constructs from a raw pointer, taking ownership.
         * @tparam T The pointed-to type.
         * @param aPointer Raw pointer to take ownership of.
         */
        template<class T>
        UniqueAnyPtr ( T* aPointer ) noexcept :
            mPointer{reinterpret_cast<void*> ( aPointer ) }, mManager{Manager<T>} {}

        /** @brief Constructs by moving a value into a new heap allocation.
         * @tparam T The value type.
         * @param aValue The value to move.
         */
        template<class T>
        UniqueAnyPtr ( T&& aValue ) noexcept :
            mPointer{new T ( std::move ( aValue ) ) }, mManager{Manager<T>} {}

        /** @brief Destructor. Deletes the owned object if present. */
        ~UniqueAnyPtr()
        {
            if ( mManager )
            {
                mManager ( mPointer );
            };
        }
        /**@}*/

        /** @name Assignment */
        /**@{*/
        /** @brief Move assignment operator. Transfers ownership from another UniqueAnyPtr.
         * @param aUniqueAnyPtr The source pointer to move from.
         * @return Reference to this.
         */
        UniqueAnyPtr& operator= ( UniqueAnyPtr&& aUniqueAnyPtr ) noexcept
        {
            aUniqueAnyPtr.Swap ( *this );
            return *this;
        }

        /** @brief Assigns from a std::unique_ptr, taking ownership.
         * @tparam T The pointed-to type.
         * @param aUniquePointer The unique_ptr to take ownership from.
         * @return Reference to this.
         */
        template<class T>
        UniqueAnyPtr& operator= ( std::unique_ptr<T>&& aUniquePointer ) noexcept
        {
            if ( mManager )
            {
                mManager ( mPointer );
            };
            mPointer = aUniquePointer.release();
            mManager = Manager<T>;
            return *this;
        }

        /** @brief Assigns from a raw pointer, taking ownership.
         * @tparam T The pointed-to type.
         * @param aPointer Raw pointer to take ownership of.
         * @return Reference to this.
         */
        template<class T>
        UniqueAnyPtr& operator= ( T* aPointer ) noexcept
        {
            if ( mManager )
            {
                mManager ( mPointer );
            };
            mPointer = aPointer;
            mManager = Manager<T>;
            return *this;
        }
        /**@{*/

        /** @brief Returns a const void pointer to the owned object.
         * @return Const void pointer to the stored object.
         */
        const void* GetRaw() const
        {
            return mPointer;
        }

        /** @brief Returns a typed pointer to the owned object (const overload).
         * @tparam T The expected type.
         * @return Pointer to the object, or nullptr if empty.
         * @throws std::runtime_error If the stored type does not match T.
         */
        template<class T> T* Get() const
        {
            if ( !mPointer )
            {
                return nullptr;
            }
            else if ( !HasType<T>() )
            {
                std::ostringstream stream;
                stream << "Unique Any Pointer of different type, Requested: " << typeid ( T ).name() << " Contained: " << GetTypeInfo().name();
                throw std::runtime_error ( stream.str().c_str() );
            }
            return reinterpret_cast<T*> ( mPointer );
        }

        /** @brief Returns a typed pointer to the owned object (non-const overload).
         * @tparam T The expected type.
         * @return Pointer to the object, or nullptr if empty.
         * @throws std::runtime_error If the stored type does not match T.
         */
        template<class T> T* Get()
        {
            // EC++ Item 3
            return const_cast<T*> ( static_cast<const UniqueAnyPtr*> ( this )->Get<T>() );
        }

        /** @brief Releases ownership and returns the raw void pointer.
         * @return The previously owned pointer.
         */
        void* ReleaseRaw()
        {
            void* pointer = mPointer;
            mPointer = nullptr;
            mManager = nullptr;
            return pointer;
        }

        /** @brief Releases ownership and returns a typed pointer.
         * @tparam T The expected type.
         * @return The previously owned pointer cast to T*.
         * @throws std::runtime_error If the stored type does not match T.
         */
        template<class T> T* Release()
        {
            if ( !HasType<T>() )
            {
                std::ostringstream stream;
                stream << "Unique Any Pointer of different type, Requested: " << typeid ( T ).name() << " Contained: " << GetTypeInfo().name();
                throw std::runtime_error ( stream.str().c_str() );
            }
            return reinterpret_cast<T*> ( ReleaseRaw() );
        }

        /** @brief Releases ownership and returns a std::unique_ptr.
         * @tparam T The expected type.
         * @return A std::unique_ptr owning the released object.
         * @throws std::runtime_error If the stored type does not match T.
         */
        template<class T> std::unique_ptr<T> UniquePointer()
        {
            return std::unique_ptr<T> ( Release<T>() );
        }

        /** @brief Destroys the owned object and resets to empty state. */
        void Reset()
        {
            UniqueAnyPtr unique_any_ptr{nullptr};
            unique_any_ptr.Swap ( *this );
        }

        /** @brief Returns the std::type_info of the stored type.
         * @return Reference to the type_info, or typeid(nullptr) if empty.
         */
        const std::type_info& GetTypeInfo() const
        {
            if ( mManager )
            {
                return mManager ( nullptr );
            }
            return typeid ( nullptr );
        }

        /** @brief Checks whether the stored object is of type T.
         * @tparam T The type to check against.
         * @return True if the stored type matches T.
         */
        template<class T>
        bool HasType() const
        {
            return GetTypeInfo().hash_code() == typeid ( T ).hash_code();
        }

    private:
        template<typename T> static const std::type_info& Manager ( void* aPointer )
        {
            if ( aPointer )
            {
                delete reinterpret_cast<T*> ( aPointer );
            }
            return typeid ( T );
        }
    };

    /** @brief Creates a UniqueAnyPtr owning a new instance of T.
     * @tparam T The type to construct.
     * @tparam Ts Constructor argument types.
     * @param params Arguments forwarded to the T constructor.
     * @return A UniqueAnyPtr owning the newly created object.
     */
    template<typename T, typename... Ts>
    UniqueAnyPtr MakeUniqueAny ( Ts&&... params )
    {
        return UniqueAnyPtr ( new T ( ::std::forward<Ts> ( params )... ) );
    }
}
#endif
