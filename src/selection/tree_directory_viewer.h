#pragma once

#include "window.h"
#include "write_screen.h"
#include "style.h"

#include "tree_directory.h"

namespace wavy::selection {
    template<typename T>
    class tree_directory_viewer {
    public:
        size_t add_node(const std::string& name) {
            return m_tree.add_node(name);
        }

        void init() {
            m_tree.init_tree();
            m_cursor_line = 0;
        }

        T& node_data(const size_t index) {
            return (*m_tree.get_node(index))->data;
        }

        void set_frame(const size_t columns, const size_t rows) {
            m_display_nodes.resize(rows);
            set_frame_from(columns, rows, m_display_nodes.begin()->first);
        }

        void set_frame_from(const size_t columns, const size_t rows, const size_t node) {
            m_columns = columns;
            m_display_nodes.resize(rows);
            auto current = node;

            for (int i = 0; i < rows; i++) {
                if (auto node = m_tree.get_node(current)) {
                    m_display_nodes[i] = {current, *node};
                } else {
                    m_display_nodes[i] = {current, nullptr};
                }
                current = m_tree.move_down(current);
            }
        }

        std::vector<size_t> select() {
            auto& [id, node] = m_display_nodes[m_cursor_line];

            auto affected = m_tree.select(id, true);

            set_frame(m_columns, m_display_nodes.size());

            return affected;
        }

        std::vector<size_t> unselect() {
            auto& [id, node] = m_display_nodes[m_cursor_line];

            auto affected = m_tree.select(id, false);

            set_frame(m_columns, m_display_nodes.size());

            return affected;
        }

        void goto_root() {
            auto& [selected_id, selected_node] = m_display_nodes[m_cursor_line];
            auto root = m_tree.get_root(selected_id);

            int n = 0;
            for (auto& [id, node] : m_display_nodes) {
                if (id == root) {
                    m_cursor_line = n;
                    return;
                }
                
                n++;
            }

            set_frame_from(m_columns, m_display_nodes.size(), root);
            m_cursor_line = 0;
        }

        bool search_for_node(const std::string& filter) {
            auto& [selected_id, selected_node] = m_display_nodes[m_cursor_line];
            if (auto result_id = m_tree.search_for_node(filter, selected_id)) {
                int n = 0;
                for (auto& [id, node] : m_display_nodes) {
                    if (id == *result_id) {
                        m_cursor_line = n;
                        return true;
                    }
                    
                    n++;
                }

                set_frame_from(m_columns, m_display_nodes.size(), *result_id);
                m_cursor_line = 0;
                return true;
            }

            return false;
        }

        void collapse_toogle() {
            auto& [id, node] = m_display_nodes[m_cursor_line];

            m_tree.collapse_toogle(id);

            set_frame(m_columns, m_display_nodes.size());
        }

        void move_up() {
            if (m_cursor_line > 0) {
                m_cursor_line--;
            } else {
                auto first = m_display_nodes.begin()->first;
                if (first > 0) {
                    for (size_t i = m_display_nodes.size() - 1; i > 0; i--) {
                        m_display_nodes[i] = m_display_nodes[i - 1];
                    }
                    auto next = m_tree.move_up(first);
                    if (auto node = m_tree.get_node(next)) {
                        *m_display_nodes.begin() = {next, *node};
                    } else {
                        *m_display_nodes.begin() = {next, nullptr};
                    }
                }
            }
        }

        void move_down() {
            m_cursor_line++;

            if (m_cursor_line == m_display_nodes.size()) {
                m_cursor_line--;
                auto last = m_display_nodes.rbegin()->first;
                for (size_t i = 0; i < m_display_nodes.size() - 1; i++) {
                    m_display_nodes[i] = m_display_nodes[i + 1];
                }
                auto next = m_tree.move_down(last);
                if (auto node = m_tree.get_node(next)) {
                    *m_display_nodes.rbegin() = {next, *node};
                } else {
                    *m_display_nodes.rbegin() = {next, nullptr};
                }
            }
        }

        std::string node_name(const size_t index) const {
            auto name = (*m_tree.get_node(index))->name;
            auto [steps, path_name] = m_tree.path_name(index);

            return path_name + name;
        }

        std::string frame() const {
            std::string result;
            int n = 0;
            for (const auto& [id, node] : m_display_nodes) {
                std::string line;
                std::string style_prefix;
                std::string style_postfix;

                if (node == nullptr) {
                    line = std::string(m_columns, ' ') + "\n";
                    style_prefix = cmd::begin_style<style_empty_row>();
                    style_postfix = cmd::end_style<style_empty_row>();
                } else {
                    auto name = node->name;
                    auto [steps, path_name] = m_tree.path_name(id);

                    line = std::string(steps, ' ');
                    style_prefix = cmd::begin_style<style_normal_row>();
                    style_postfix = cmd::end_style<style_normal_row>();

                    if (node->collapsed && node->skip > 0) {
                        line += "+";
                        line += path_name + node->name.substr(0, node->depth);

                        if (node->depth != node->name.length()) {
                            line += "...";
                        }

                        style_prefix += cmd::begin_style<style_collapsed_row>();
                        style_postfix += cmd::end_style<style_collapsed_row>();
                    } else {
                        if (node->skip > 0) {
                            line += "-";
                        } else {
                            line += " ";
                        }
                        line += path_name;
                        line += name;
                    }

                    line += std::string(m_columns - line.length(), ' ') + "\n";

                    if (node->selected) {
                        style_prefix += cmd::begin_style<style_selected_row>();
                        style_postfix += cmd::end_style<style_selected_row>();
                    }
                }

                if (n == m_cursor_line) {
                    style_prefix += cmd::begin_style<style_cursor_row>();
                    style_postfix += cmd::end_style<style_cursor_row>();
                }

                result += style_prefix + line + style_postfix;
                n++;
            }
            return result;
        }
    private:
        typedef typename cmd::styles::group<
            cmd::styles::colors::background::extended<240>,
            cmd::styles::colors::foreground::extended<15>
        > style_empty_row;
        typedef typename cmd::styles::group<
            cmd::styles::colors::background::extended<240>,
            cmd::styles::colors::foreground::extended<15>
        > style_normal_row;
        typedef typename cmd::styles::group<
            cmd::styles::font::invert
        > style_cursor_row;
        typedef typename cmd::styles::group<
            cmd::styles::colors::background::extended<240>,
            cmd::styles::colors::foreground::extended<15>,
            cmd::styles::font::bold
        > style_collapsed_row;
        typedef typename cmd::styles::group<
            cmd::styles::colors::background::extended<6>,
            cmd::styles::colors::foreground::extended<15>,
            cmd::styles::font::italic
        > style_selected_row;

        tree_directory<T> m_tree;
        std::vector<std::pair<size_t, const typename tree_directory<T>::node*>> m_display_nodes;
        int m_cursor_line = 0;
        size_t m_columns;
    };
}
