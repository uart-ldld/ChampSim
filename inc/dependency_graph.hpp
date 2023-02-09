/*
 * Copyright (C) 2023  Xiaoyue Chen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DEPENDENCY_GRAPH_H
#define DEPENDENCY_GRAPH_H

#include <cassert>
#include <utility>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/copy.hpp>
#include <boost/graph/graphviz.hpp>
#include <unordered_map>

namespace champsim
{

template <typename Key, typename T>
class dependency_graph
{
public:
  using value_type = std::pair<const Key, T>;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using Graph = boost::adjacency_list<boost::hash_setS, boost::listS, boost::bidirectionalS, value_type>;
  using Vertex = typename boost::graph_traits<Graph>::vertex_descriptor;
  using VertexTable = std::unordered_map<Key, Vertex>;

  bool contains(const Key& key) const { return vertex.find(key) != std::end(vertex); }

  const_pointer before(const Key& key) const
  {
    const auto& u = vertex.at(key);
    auto [e_it, e_end] = boost::out_edges(u, graph);
    if (e_it == e_end)
      return nullptr;

    auto v = boost::target(*e_it, graph);
    return &graph[v];
  }

  void add_edge(const Key& u_key, const Key& v_key)
  {
    auto u = add_vertex(u_key);
    auto v = add_vertex(v_key);
    boost::add_edge(u, v, graph);
    assert(boost::out_degree(u, graph) == 1);
  }

  void remove_edge(const Key& u_key, const Key& v_key)
  {
    auto u_it = vertex.find(u_key);
    if (u_it == std::end(vertex))
      return;
    auto v_it = vertex.find(v_key);
    if ((v_it) == std::end(vertex))
      return;

    Vertex u = u_it->second, v = v_it->second;
    boost::remove_edge(u, v, graph);
    if (!boost::in_degree(v, graph) && !boost::out_degree(v, graph)) {
      boost::remove_vertex(v, graph);
      vertex.erase(v_it);
    }
  }

  T& operator[](const Key& key)
  {
    auto v = add_vertex(key);
    return graph[v].second;
  }

  const T& operator[](const Key& key) const
  {
    auto v = vertex.at(key);
    return graph[v].second;
  }

  template <typename Writer>
  void write_graphviz(std::ostream& out, Writer writer) const
  {
    using WriteGraph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, value_type>;
    using WriteVertex = typename boost::graph_traits<WriteGraph>::vertex_descriptor;

    auto copy = WriteGraph{};
    auto map = std::unordered_map<Vertex, WriteVertex>{};

    for (auto [v_it, v_end] = vertices(graph); v_it != v_end; ++v_it) {
      map[*v_it] = boost::add_vertex(graph[*v_it], copy);
    }

    for (auto [e_it, e_end] = edges(graph); e_it != e_end; ++e_it) {
      boost::add_edge(map.at(source(*e_it, graph)), map.at(target(*e_it, graph)), copy);
    }

    auto writer_wrapper = [&](std::ostream& out, const auto& v) {
      writer(out, copy[v].first, copy[v].second);
    };

    boost::write_graphviz(out, copy, writer_wrapper);
  }

private:
  Vertex add_vertex(const Key& key)
  {
    if (vertex.find(key) == std::end(vertex)) {
      auto v = boost::add_vertex(std::make_pair(key, T{}), graph);
      vertex.try_emplace(key, v);
      return v;
    }

    return vertex.at(key);
  }

  Graph graph = {};
  VertexTable vertex = {};
};

} // namespace champsim

#endif
