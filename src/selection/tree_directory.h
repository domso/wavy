#pragma once

#include <unordered_map>
#include <memory>
#include <optional>

namespace wavy::selection {
    template<typename T>
    class tree_directory {
    public:
        struct node {
            std::string name;
            int level = 0;
            int skip = 1;
            int root = 0;
            int depth = 0;

            T data;

            bool hidden = false;
            bool collapsed = false;
            bool selected = false;
        };

        size_t add_node(const std::string& name) {
            node new_node;
            new_node.name = name;
            m_nodes.push_back(std::move(new_node));
            return m_nodes.size() - 1;
        }

        bool init_tree() {
            if (sort_and_check_unique_constraint()) {
                std::vector<size_t> node_stack;
                treeify(0, m_nodes.size(), 0);

                for (auto& node : m_nodes) {
                    node.name = node.name.substr(node.level);
                    node.collapsed = true;
                    node.hidden = (node.root != 0);
                }

                return true;
            }
            return false;
        }

        std::pair<size_t, std::string> path_name(const size_t node) const {
            std::string path;
            auto level = m_nodes[node].level;
            size_t steps = 0;

            auto current = node;
            while (m_nodes[current].root > 0) {
                current -= m_nodes[current].root;
                auto level_delta = level - m_nodes[current].level;
                path = m_nodes[current].name.substr(0, level_delta) + path;
                steps++;
                
                level = m_nodes[current].level;
            }

            return {steps, path};
        }

        size_t move_down(const size_t n) const {
            if (n >= m_nodes.size() - 1) {
                return n + 1;
            } else {
                for (auto i = n + 1; i < m_nodes.size(); i++) {
                    if (!m_nodes[i].hidden) {
                        return i;
                    }
                }
                return m_nodes.size();
            }
        }

        size_t move_up(const size_t n) const {
            if (n > m_nodes.size()) {
                return n - 1;
            } else if (n <= 1) {
                return 0;
            } else {
                for (auto i = n - 1; i > 0; i--) {
                    if (!m_nodes[i].hidden) {
                        return i;
                    }
                }
                return 0;
            }
        }

        std::optional<size_t> search_for_node(const std::string& filter, const size_t n) const {
            for (auto current = n + 1; current < m_nodes.size(); current++) {
                auto full_name = path_name(current).second + m_nodes[current].name;
                if (filter_matches(full_name, filter)) {
                    return current;
                }
            }
            for (auto current = 0; current < std::min(m_nodes.size(), n - 1); current++) {
                auto full_name = path_name(current).second + m_nodes[current].name;
                if (filter_matches(full_name, filter)) {
                    return current;
                }
            }
            return std::nullopt;
        }

        std::optional<const node*> get_node(const size_t n) const {
            if (n < m_nodes.size()) {
                return &(m_nodes[n]);
            }
            return std::nullopt;
        }

        std::optional<node*> get_node(const size_t n) {
            if (n < m_nodes.size()) {
                return &(m_nodes[n]);
            }
            return std::nullopt;
        }

        size_t get_root(const size_t n) const {
            if (n < m_nodes.size()) {
                return n - m_nodes[n].root;
            }
            return 0;
        }

        std::vector<size_t> select(const size_t node, const bool selected) {
            std::vector<size_t> affected;

            size_t end = std::min(node + m_nodes[node].skip, m_nodes.size());
            auto current = node + 1;

            if (m_nodes[node].selected != selected) {
                affected.push_back(node);
            }
            m_nodes[node].selected = selected;

            if (m_nodes[node].collapsed)  {
                while (current < end) {
                    if (m_nodes[current].selected != selected) {
                        affected.push_back(current);
                    }
                    m_nodes[current].selected = selected;

                    current++;
                }
            }

            return affected;
        }

        void collapse_toogle(const size_t node) {
            size_t end = std::min(node + m_nodes[node].skip, m_nodes.size());
            bool new_hidden;
            auto current = node + 1;

            if (m_nodes[node].collapsed) {
                new_hidden = false;
                m_nodes[node].collapsed = false;
            } else {
                new_hidden = true;
                m_nodes[node].collapsed = true;
            }
            
            while (current < end) {
                m_nodes[current].hidden = new_hidden;
               if (m_nodes[current].collapsed)  {
                   current += std::max(m_nodes[current].skip, 1);
               } else {
                   current++;
               }
            }
        }
    private:
        bool sort_and_check_unique_constraint() {
            bool redo;
            do {
                std::sort(m_nodes.begin(), m_nodes.end(), [](const node& l, const node& r) {
                    return l.name < r.name;
                });

                redo = false;
                std::string compare = "";
                for (auto& node : m_nodes) {
                    if (compare == node.name) {
                        node.name += "'";
                        redo = true;
                        break;
                    }
                    compare = node.name;            
                }
            } while (redo);

            return true;
        }

        void increase_node_level(const size_t root, const size_t end, const size_t n) {
            for (auto it = root + 1; it < end; it++) {
                m_nodes[it].level++;
                m_nodes[it].root = it - root;
            }

            if (n + 1 == m_nodes[root].name.length()) {
                treeify(root + 1, end, n + 1);
            } else {
                treeify(root, end, n + 1);
            }

            m_nodes[root].depth = m_nodes[root].name.length();
            for (auto it = root + 1; it < end; it++) {
                m_nodes[root].depth = std::min(m_nodes[root].depth, m_nodes[it].level);
            }
            m_nodes[root].depth = m_nodes[root].depth - m_nodes[root].level;

            m_nodes[root].skip = end - root;
            m_nodes[end - 1].skip = 0;
        }

        void treeify(const size_t start, const size_t end, const size_t n) {
            bool valid = false;
            char found;
            size_t found_pos;

            for (auto current = start; current < end; current++) { 
                if (!valid) {
                    found_pos = current;
                    found = m_nodes[current].name.at(n);
                    valid = true;
                } else {
                    if (found != m_nodes[current].name.at(n)) {
                        increase_node_level(found_pos, current, n);
                        
                        found_pos = current;
                        found = m_nodes[current].name.at(n);
                    }
                }
            }

            if (valid) {
                increase_node_level(found_pos, end, n);
            }
        }

        bool filter_matches(const std::string& name, const std::string& filter) const {
            return name.find(filter) != std::string::npos;
        }

        std::vector<node> m_nodes;
    };
}
