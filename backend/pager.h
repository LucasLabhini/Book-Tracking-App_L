#pragma once
#include <vector>
#include <string>
#include <optional>
#include "nlohmann/json.hpp"

enum class Status { Reading, Read, Wish };

struct Book {
    int id;
    std::string title;
    std::string author;
    std::string datePublished;
    std::string genre;
    int pageProgress = 0;
    
    bool inLibrary = true;
    Status section;

    static Book fromJSON(const nlohmann::json& j);
    nlohmann::json toJSON() const;
};

namespace Books {

    bool addBook(const Book& book, Status section);
    bool removeBook(int id);
    bool changeSection(int id, Status newSection);
    bool updateProgress(int id, int page);

    std::vector<Book> getBooks(std::optional<Status> section = std::nullopt);

    enum class SortBy { Title, Author, Date, Genre };
    std::vector<Book> getBooksSorted(std::optional<Status> section, SortBy sortBy);

    std::vector<Book> searchBooks(const std::string& term, std::optional<Status> section = std::nullopt);

}
