/*
Copyright (C) 2018 Rodrigo Jose Hernandez Cordoba

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

namespace AeonGames
{
    template <
        class Key,
        class T,
        class Hash = std::hash<Key>,
        class KeyEqual = std::equal_to<Key>,
        class MapAllocator = std::allocator< std::pair<const Key, T>>,
        class VectorAllocator = std::allocator<Key>
        >
    class DependencyMap
    {
        typedef typename std::tuple <
        Key,                               //-> Parent Iterator
        size_t,                            //-> Child Iterator
        std::vector<Key, VectorAllocator>, //-> Dependencies
        T                                  //-> Payload
        > GraphNode;

    public:
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
                const typename std::vector<Key, VectorAllocator>::iterator& aIterator,
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
            typedef typename std::vector<Key, VectorAllocator>::iterator::difference_type difference_type;
            typedef typename std::vector<Key, VectorAllocator>::iterator::value_type value_type;
            typedef T& reference;
            typedef T* pointer;
            typedef typename std::bidirectional_iterator_tag iterator_category;
            iterator() = default;
            iterator ( const iterator& ) = default;
            ~iterator() = default;
            iterator& operator= ( const iterator& ) = default;
            bool operator== ( const iterator& i ) const
            {
                return mIterator == i.mIterator;
            }
            bool operator!= ( const iterator& i ) const
            {
                return mIterator != i.mIterator;
            }
            iterator& operator++()
            {
                mIterator++;
                return *this;
            }
            reference operator*() const
            {
                return std::get<3> ( mGraph->at ( *mIterator ) );
            }
            pointer operator->() const
            {
                return &std::get<3> ( mGraph->at ( *mIterator ) );
            };
        };

        void Reserve ( size_t count )
        {
            graph.reserve ( count );
            sorted.reserve ( count );
        }

        DependencyMap() = default;
        ~DependencyMap() = default;
        DependencyMap ( std::initializer_list<std::tuple<Key, std::vector<Key>, T>> aList )
        {
            Reserve ( aList.size() );
            for ( auto& i : aList )
            {
                graph[std::get<0> ( i )] = {{}, 0, std::get<1> ( i ), std::get<2> ( i ) };
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

        void Insert ( const std::tuple<Key, std::vector<Key>, T>& item )
        {
            // If the map is empty just insert the new node.
            if ( sorted.empty() && graph.empty() )
            {
                graph[std::get<0> ( item )] = {{}, 0, std::get<1> ( item ), std::get<2> ( item ) };
                sorted.emplace_back ( std::get<0> ( item ) );
                return;
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
                            // We get here if the current node has no further children to process
                            sorted.erase ( std::remove ( sorted.begin(), sorted.end(), node ), sorted.end() );
                            sorted.insert ( insertion_cursor++, node );
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
                graph[std::get<0> ( item )] = {{}, 0, std::get<1> ( item ), std::get<2> ( item ) };
                sorted.insert ( insertion_cursor, std::get<0> ( item ) );
            }
            else
            {
                throw std::runtime_error ( "New node would create a circular dependency." );
            }
        }

        void Erase ( const Key& key )
        {
            graph.erase ( key );
            sorted.erase ( std::remove ( sorted.begin(), sorted.end(), key ), sorted.end() );
        }

        const T& operator[] ( const std::size_t index ) const
        {
            if ( index >= sorted.size() )
            {
                throw std::out_of_range ( "Index out of range." );
            }
            return std::get<3> ( graph.at ( sorted[index] ) );
        }

        const std::size_t Size() const
        {
            return sorted.size();
        }

        iterator begin()
        {
            return iterator{sorted.begin(), &graph};
        }
        iterator end()
        {
            return iterator{sorted.end(), &graph};
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
