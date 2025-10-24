#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <fstream>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

// -------------------- Data Structures --------------------
struct Book {
    int id;
    std::string title;
    std::string author;
    std::string published;           // optional field for demo
    std::vector<std::string> tags;
    int currentPage = 0;
    bool isRead = false;
};

// Sections
std::unordered_map<int, Book> library;
std::unordered_map<int, Book> reading;

// -------------------- Load JSON --------------------
void loadLibraryFromJSON(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filename << "\n";
        exit(1);
    }

    json j;
    file >> j;

    for (auto& item : j) {
        Book b;
        b.id = item["id"];
        b.title = item["title"];
        b.author = item["author"];
        b.published = item.value("published", "Unknown");
        b.tags = item["tags"].get<std::vector<std::string>>();
        library[b.id] = b;
    }
}

// -------------------- Find Book by Title --------------------
Book* findBookByTitle(const std::string& query) {
    for (auto& pair : library) {
        if (pair.second.title.find(query) != std::string::npos) {
            return &pair.second;
        }
    }
    return nullptr;
}

// -------------------- Main --------------------
int main() {
    loadLibraryFromJSON("books.json");

    std::cout << "Which book do you want to read? ";
    std::string input;
    std::getline(std::cin, input);

    Book* selectedBook = findBookByTitle(input);
    if (selectedBook) {
        // Mark as currently reading
        reading[selectedBook->id] = *selectedBook;

        // Display details
        std::cout << "\nYou are now reading:\n";
        std::cout << "Title: " << selectedBook->title << "\n";
        std::cout << "Author: " << selectedBook->author << "\n";
        std::cout << "Published: " << selectedBook->published << "\n";
        std::cout << "Tags: ";
        for (const auto& tag : selectedBook->tags) std::cout << tag << " ";
        std::cout << "\n";
    } else {
        std::cout << "Book not found in library.\n";
    }

    return 0;
}
