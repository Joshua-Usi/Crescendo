#pragma once

#include <map>
#include <string>
#include <vector>
#include <memory>

namespace cs_std::xml
{
	namespace internal
	{
		template <typename T>
		class raw_pointer_iterator : public std::iterator<std::input_iterator_tag, T>
		{
		private:
			typename std::vector<std::unique_ptr<T>>::iterator it_;
		public:
			explicit raw_pointer_iterator(typename std::vector<std::unique_ptr<T>>::iterator it) : it_(it) {}
		public:
			T* operator*() const { return it_->get(); }
			bool operator==(const raw_pointer_iterator& other) const { return it_ == other.it_; }
			bool operator!=(const raw_pointer_iterator& other) const { return it_ != other.it_; }
			raw_pointer_iterator& operator++() { ++it_; return *this; }

		};
	}
	class attribute_container
	{
	public:
		std::map<std::string, std::string> attributes;
	public:
		attribute_container() = default;
		attribute_container(const std::map<std::string, std::string>& attributes) : attributes(attributes) {}
	public:
		std::string& get_attribute(const std::string& attributeName) { return attributes[attributeName]; }
		void set_attribute(const std::string& attributeName, const std::string& attributeValue) { attributes[attributeName] = attributeValue; }
		void remove_attribute(const std::string& attributeName) { attributes.erase(attributeName); }
		size_t attribute_count() const { return attributes.size(); }
	public:
		std::string& operator[](const std::string& attributeName) { return get_attribute(attributeName); }
	};
	class node : public attribute_container
	{
	public:
		std::vector<std::unique_ptr<node>> children;
		node* parent;
	public:
		std::string tag, innerText;
	public:
		node() = default;
		node(node* parent, const std::string& tag) : parent(parent), tag(tag) { if (parent != nullptr) parent->children.emplace_back(this); }
		node(node* parent, const std::string& tag, const std::string& innerText) : parent(parent), tag(tag), innerText(innerText) { if (parent != nullptr) parent->children.emplace_back(this); }
		node(node* parent, const std::string& tag, const std::string& innerText, const std::map<std::string, std::string>& attributes) : parent(parent), tag(tag), innerText(innerText), attribute_container(attributes) { if (parent != nullptr) parent->children.emplace_back(this); }
	public:
		// rapidxml interface
		// RapidXML like syntax
		node* _first_node() const
		{
			if (child_count() > 0) return children[0].get();
			return nullptr;
		}
		node* _next_sibling()
		{
			if (!parent) return nullptr;
			uint32_t i = 0;
			while (i < parent->child_count())
			{
				if ((parent->children[i].get() == this) && i + 1 < parent->child_count()) return &parent[i + 1];
				i++;
			}
			return nullptr;
		}
		node* _parent() const { return parent; }
	public:
		void add_child(node* child) { children.push_back(std::unique_ptr<node>(child)); }
		void remove_child(size_t index) { children.erase(children.begin() + index); }
		size_t child_count() const { return children.size(); }
	public:
		internal::raw_pointer_iterator<node> begin() { return internal::raw_pointer_iterator<node>(children.begin()); }
		internal::raw_pointer_iterator<node> end() { return internal::raw_pointer_iterator<node>(children.end()); }
	public:
		node* operator[](size_t index) { return children[index].get(); }
	};
	class document : public attribute_container
	{
	public:
		std::unique_ptr<node> root;
	public:
		document() = default;
		document(node* r) { root.reset(r); }
		document(const std::string& xml);
	public:
		node* get_root() const { return root.get(); }
		std::string stringify() const;
	public:
		internal::raw_pointer_iterator<node> begin() { return internal::raw_pointer_iterator<node>(root->children.begin()); }
		internal::raw_pointer_iterator<node> end() { return internal::raw_pointer_iterator<node>(root->children.end()); }
	};
}