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
#ifndef AEONGAMES_DEPENDENCY_MAP_H
#define AEONGAMES_DEPENDENCY_MAP_H
#include <vector>
#include <unordered_map>
#include <tuple>
#include <algorithm>
#include <iterator>

namespace AeonGames
{
    /** @brief A dependency-aware associative container that stores elements in topologically sorted order.
     *  @tparam Key The key type used to identify nodes.
     *  @tparam T The payload type stored with each node.
     *  @tparam Hash Hash function object type for keys.
     *  @tparam KeyEqual Equality comparison function object type for keys.
     *  @tparam MapAllocator Allocator type for the internal unordered map.
     *  @tparam VectorAllocator Allocator type for the internal sorted vector.
     */
    template <
        class Key,
        class T = Key,
        class Hash = std::hash<Key>,
        class KeyEqual = std::equal_to<Key>,
        class MapAllocator = std::allocator< std::pair<const Key, typename std::tuple <
                Key,
                size_t,
                std::vector<Key, std::allocator<Key >>,
                T
                >>>,
        class VectorAllocator = std::allocator<Key>
        >
    class DependencyMap
    {
        using GraphNode = typename std::tuple <
                          Key,                               //-> Parent Iterator
                          size_t,                            //-> Child Iterator
                          std::vector<Key, VectorAllocator>, //-> Dependencies
                          T                                  //-> Payload
                          >;

    public:
        /** @brief Convenience type alias for an insertion tuple: (key, dependencies, payload). */
        using triple = typename std::tuple<Key, std::vector<Key>, T>;

        /** @brief Mutable bidirectional iterator over topologically sorted entries. */
        class iterator
        {
            typename std::vector<Key, VectorAllocator>::iterator mIterator{};
            typename std::unordered_map <
            Key,
            GraphNode,
            Hash,
            KeyEqual,
            MapAllocator
            > * mGraph{};
            friend class DependencyMap<Key, T, Hash, KeyEqual, MapAllocator, VectorAllocator>;
            iterator (
                const typename std::vector<Key, VectorAllocator>::iterator & aIterator,
                std::unordered_map <
                Key,
                GraphNode,
                Hash,
                KeyEqual,
                MapAllocator
                > * aGraph ) :
                mIterator ( aIterator ),
                mGraph ( aGraph ) {}
        public:
            using difference_type = typename std::vector<Key, VectorAllocator>::iterator::difference_type; ///< Iterator difference type.
            using value_type = typename std::vector<Key, VectorAllocator>::iterator::value_type; ///< Iterator value type.
            using reference = T &; ///< Reference to the mapped value type.
            using pointer = T *; ///< Pointer to the mapped value type.
            using iterator_category = std::bidirectional_iterator_tag; ///< Bidirectional iterator category.
            iterator() = default;
            /// @brief Copy constructor.
            iterator ( const iterator& ) = default;
            ~iterator() = default;
            /// @brief Copy assignment operator.
            iterator& operator= ( const iterator& ) = default;
            /// @brief Equality comparison operator.
            bool operator== ( const iterator & i ) const
            {
                return mIterator == i.mIterator;
            }
            /// @brief Inequality comparison operator.
            bool operator!= ( const iterator & i ) const
            {
                return mIterator != i.mIterator;
            }
            /// @brief Pre-increment operator.
            iterator & operator++()
            {
                mIterator++;
                return *this;
            }
            /// @brief Pre-decrement operator.
            iterator & operator--()
            {
                mIterator--;
                return *this;
            }
            /// @brief Dereference operator.
            reference operator*() const
            {
                return std::get<3> ( mGraph->at ( *mIterator ) );
            }
            /// @brief Member access operator.
            pointer operator->() const
            {
                return &std::get<3> ( mGraph->at ( *mIterator ) );
            };
            /** @brief Get the key of the element this iterator points to.
             *  @return A const reference to the key. */
            const Key & GetKey() const
            {
                return *mIterator;
            };
            /** @brief Get the dependency list of the element this iterator points to.
             *  @return A const reference to the vector of dependency keys. */
            const std::vector<Key, VectorAllocator>& GetDependencies() const
            {
                return std::get<2> ( mGraph->at ( *mIterator ) );
            };
        };

        /** @brief Const bidirectional iterator over topologically sorted entries. */
        class const_iterator
        {
            typename std::vector<Key, VectorAllocator>::const_iterator mIterator{};
            const typename std::unordered_map <
            Key,
            GraphNode,
            Hash,
            KeyEqual,
            MapAllocator
            > * mGraph{};
            friend class DependencyMap<Key, T, Hash, KeyEqual, MapAllocator, VectorAllocator>;
            const_iterator (
                const typename std::vector<Key, VectorAllocator>::const_iterator & aIterator,
                const std::unordered_map <
                Key,
                GraphNode,
                Hash,
                KeyEqual,
                MapAllocator
                > * aGraph ) :
                mIterator ( aIterator ),
                mGraph ( aGraph ) {}
        public:
            using difference_type = typename std::vector<Key, VectorAllocator>::const_iterator::difference_type; ///< Iterator difference type.
            using value_type = typename std::vector<Key, VectorAllocator>::const_iterator::value_type; ///< Iterator value type.
            using reference = const T &; ///< Const reference to the mapped value type.
            using pointer = const T *; ///< Const pointer to the mapped value type.
            using iterator_category = std::bidirectional_iterator_tag; ///< Bidirectional iterator category.
            const_iterator() = default;
            /// @brief Copy constructor.
            const_iterator ( const const_iterator& ) = default;
            ~const_iterator() = default;
            /// @brief Copy assignment operator.
            const_iterator& operator= ( const const_iterator& ) = default;
            /// @brief Equality comparison operator.
            bool operator== ( const const_iterator & i ) const
            {
                return mIterator == i.mIterator;
            }
            /// @brief Inequality comparison operator.
            bool operator!= ( const const_iterator & i ) const
            {
                return mIterator != i.mIterator;
            }
            /// @brief Pre-increment operator.
            const_iterator & operator++()
            {
                mIterator++;
                return *this;
            }
            /// @brief Pre-decrement operator.
            const_iterator & operator--()
            {
                mIterator--;
                return *this;
            }
            /// @brief Dereference operator.
            reference operator*() const
            {
                return std::get<3> ( mGraph->at ( *mIterator ) );
            }
            /// @brief Member access operator.
            pointer operator->() const
            {
                return &std::get<3> ( mGraph->at ( *mIterator ) );
            };
            /** @brief Get the key of the element this iterator points to.
             *  @return A const reference to the key. */
            const Key & GetKey() const
            {
                return *mIterator;
            };
            /** @brief Get the dependency list of the element this iterator points to.
             *  @return A const reference to the vector of dependency keys. */
            const std::vector<Key, VectorAllocator>& GetDependencies() const
            {
                return std::get<2> ( mGraph->at ( *mIterator ) );
            };
        };

        /** @brief Reserve storage for a given number of elements.
         *  @param count Number of elements to reserve space for. */
        void Reserve ( size_t count )
        {
            graph.reserve ( count );
            sorted.reserve ( count );
        }

        /** @brief Default constructor. */
        DependencyMap() = default;
        /** @brief Default destructor. */
        ~DependencyMap() = default;
        /** @brief Construct from an initializer list, performing topological sort.
         *  @param aList Initializer list of triples (key, dependencies, payload).
         *  @throws std::runtime_error If a circular dependency is detected. */
        DependencyMap ( std::initializer_list<triple> aList )
        {
            Reserve ( aList.size() );
            for ( auto& i : aList )
            {
                graph[std::get<0> ( i )] = GraphNode{{}, 0, std::get<1> ( i ), std::get<2> ( i ) };
            }
            for ( auto& i : graph )
            {
                // Only process the current child if it is not yet in the sorted vector.
                if ( std::find ( sorted.begin(), sorted.end(), i.first ) == sorted.end() )
                {
                    // Set initial node to the first valid child.
                    Key node = i.first;
                    // Set Parent node to itself
                    std::get<0> ( graph[i.first] ) = i.first;
                    // Non Recursive Depth First Search.
                    do
                    {
                        if ( std::get<1> ( graph[node] ) < std::get<2> ( graph[node] ).size() )
                        {
                            if ( std::get<2> ( graph[node] ) [std::get<1> ( graph[node] )] == i.first )
                            {
                                throw std::runtime_error ( "One of the provided nodes creates a circular dependency." );
                            }
                            // We get here if the current node still has further children to process
                            auto next = graph.find ( std::get<2> ( graph[node] ) [std::get<1> ( graph[node] )] );
                            ++std::get<1> ( graph[node] );
                            // Skip not (yet) existing nodes and nodes already inserted.
                            if ( next != graph.end() && std::find ( sorted.begin(), sorted.end(), ( *next ).first ) == sorted.end() )
                            {
                                std::get<0> ( ( *next ).second ) = node;
                                node = ( *next ).first;
                            }
                        }
                        else
                        {
                            // We get here if the current node has no further children to process
                            sorted.push_back ( node );
                            std::get<1> ( graph[node] ) = 0; // Reset counter for next traversal.
                            // Go back to the parent
                            node = std::get<0> ( graph[node] );
                        }
                    }
                    while ( node != std::get<0> ( graph[node] ) );
                    sorted.push_back ( node );
                }
            }
        }

        /** @brief Insert a new element, maintaining topological order.
         *  @param item A triple (key, dependencies, payload) to insert.
         *  @return The index at which the element was inserted in the sorted order.
         *  @throws std::runtime_error If the insertion would create a circular dependency. */
        size_t Insert ( const triple& item )
        {
            // If the map is empty just insert the new node.
            if ( sorted.empty() && graph.empty() )
            {
                graph[std::get<0> ( item )] = GraphNode{{}, 0, std::get<1> ( item ), std::get<2> ( item ) };
                sorted.emplace_back ( std::get<0> ( item ) );
                return 0;
            }
            // We'll move all dependencies and the new node to the start of the sorted vector.
            auto insertion_cursor = sorted.begin();
            bool circular_dependency = false;
            // Iterate the new node's children to bring all dependencies to the front of the sorted vector.
            for ( auto& i : std::get<1> ( item ) )
            {
                // Only process the current child if 1-it is already in the map and 2-is to the right of the insertion cursor
                if ( graph.find ( i ) != graph.end() && std::find ( sorted.begin(), insertion_cursor, i ) == insertion_cursor )
                {
                    // Set initial node to the current child.
                    Key node = i;
                    // Set Parent node
                    std::get<0> ( graph[i] ) = std::get<0> ( item );
                    // Non Recursive Depth First Search.
                    while ( node != std::get<0> ( item ) )
                    {
                        if ( std::get<1> ( graph[node] ) < std::get<2> ( graph[node] ).size() )
                        {
                            if ( std::get<2> ( graph[node] ) [std::get<1> ( graph[node] )] == std::get<0> ( item ) )
                            {
                                /*
                                If we get here, the new node would create a circular dependency,
                                so we must NOT insert it, if we break here however,
                                the instance would be left in an unusable state,
                                so record the circular dependency and let the proccess finish.
                                Later we'll just NOT insert the node but throw an exception.
                                */
                                circular_dependency = true;
                            }
                            // We get here if the current node still has further children to process
                            auto next = graph.find ( std::get<2> ( graph[node] ) [std::get<1> ( graph[node] )] );
                            ++std::get<1> ( graph[node] );
                            // Skip not (yet) existing nodes and nodes to the left of the insertion cursor.
                            if ( next != graph.end() && std::find ( sorted.begin(), insertion_cursor, ( *next ).first ) == insertion_cursor )
                            {
                                std::get<0> ( ( *next ).second ) = node;
                                node = ( *next ).first;
                            }
                        }
                        else
                        {
                            auto node_iterator = std::find ( sorted.begin(), sorted.end(), node );
                            std::rotate ( insertion_cursor++, node_iterator, node_iterator + 1 );
                            std::get<1> ( graph[node] ) = 0; // Reset counter for next traversal.
                            // Go back to the parent
                            node = std::get<0> ( graph[node] );
                        }
                    }
                }
            }
            if ( !circular_dependency )
            {
                // Insert NEW node
                graph[std::get<0> ( item )] = GraphNode{{}, 0, std::get<1> ( item ), std::get<2> ( item ) };
                return sorted.insert ( insertion_cursor, std::get<0> ( item ) ) - sorted.begin();
            }
            else
            {
                throw std::runtime_error ( "New node would create a circular dependency." );
            }
        }

        /** @brief Remove an element by key.
         *  @param key The key of the element to remove. */
        void Erase ( const Key& key )
        {
            graph.erase ( key );
            sorted.erase ( std::remove ( sorted.begin(), sorted.end(), key ), sorted.end() );
        }

        /** @brief Access element by sorted index (const).
         *  @param index Zero-based index into the sorted order.
         *  @return A const reference to the payload.
         *  @throws std::out_of_range If index is out of range. */
        const T& operator[] ( const std::size_t index ) const
        {
            if ( index >= sorted.size() )
            {
                throw std::out_of_range ( "Index out of range." );
            }
            return std::get<3> ( graph.at ( sorted[index] ) );
        }

        /** @brief Access element by sorted index (mutable).
         *  @param index Zero-based index into the sorted order.
         *  @return A mutable reference to the payload.
         *  @throws std::out_of_range If index is out of range. */
        T& operator[] ( const std::size_t index )
        {
            return const_cast<T&> ( static_cast<const DependencyMap<Key, T, Hash, KeyEqual, MapAllocator, VectorAllocator>*> ( this )->operator[] ( index ) );
        }

        /** @brief Get the number of elements.
         *  @return The number of stored elements. */
        const std::size_t Size() const
        {
            return sorted.size();
        }

        /** @brief Find an element by key (const).
         *  @param key The key to search for.
         *  @return A const_iterator to the element, or end() if not found. */
        const_iterator Find ( const Key& key ) const
        {
            return const_iterator
            {
                std::find ( sorted.begin(), sorted.end(), key ),
                &graph};
        }

        /** @brief Find an element by key (mutable).
         *  @param key The key to search for.
         *  @return An iterator to the element, or end() if not found. */
        iterator Find ( const Key& key )
        {
            return iterator
            {
                std::find ( sorted.begin(), sorted.end(), key ),
                &graph};
        }

        /** @brief Get a mutable iterator to the first element in sorted order.
         *  @return An iterator to the beginning. */
        iterator begin()
        {
            return iterator{sorted.begin(), &graph};
        }
        /** @brief Get a mutable iterator past the last element in sorted order.
         *  @return An iterator to the end. */
        iterator end()
        {
            return iterator{sorted.end(), &graph};
        }

        /** @brief Get a const iterator to the first element in sorted order.
         *  @return A const_iterator to the beginning. */
        const_iterator begin() const
        {
            return const_iterator{sorted.begin(), &graph};
        }

        /** @brief Get a const iterator past the last element in sorted order.
         *  @return A const_iterator to the end. */
        const_iterator end() const
        {
            return const_iterator{sorted.end(), &graph};
        }
    private:
        std::unordered_map
        <
        Key,
        GraphNode,
        Hash,
        KeyEqual,
        MapAllocator
        > graph;
        std::vector<Key, VectorAllocator> sorted;
    };
}
#endif
